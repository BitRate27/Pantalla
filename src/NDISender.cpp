// NDISender
#include <windows.h>
#include <iostream>
#include <setupapi.h>
#include <devguid.h>
#include <cfgmgr32.h>
#include "global.h"
#include "FrameReader.h"
#include "DXGICapture.h"
#include "ndi-loader.h"
#include "parsec-vdd.h"
#include "NDISender.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <conio.h>
#include <string>
#include <vector>
#include <cwctype>
#include <QGuiApplication>
#include <QScreen>
#include <QList>
#include <QString>
#include <QTimer>
#include <QMetaObject>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "cfgmgr32.lib")
QString FormatScreenName(QScreen *screen);

NDISender::NDISender(std::wstring deviceName, std::string ndiName)
	: m_captureRunning(false),
	  m_deviceName(deviceName),
	  m_ndiName(ndiName)
{
	m_captureThread = std::thread([this]() {
		m_captureRunning = true;
		log_file("Waiting for screen wait thread to detect new display...\n");
		while (!m_captureRunning) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		log_file("Starting capture loop for device %ls with NDI name %s\n", this->m_deviceName.c_str(),
			 this->m_ndiName.c_str());

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		CaptureLoop(this->m_deviceName, this->m_ndiName);
	});
	m_captureThread.detach();
};

NDISender::~NDISender()
{
	// Stop capture thread if running
	if (m_captureRunning) {
		m_captureRunning = false;
		log_file("Stopping capture loop for device %ls with NDI name %s\n", this->m_deviceName.c_str(),
			 this->m_ndiName.c_str());
		std::this_thread::sleep_for(std::chrono::milliseconds(101));
		if (m_captureThread.joinable()) {
			m_captureThread.join();
		}
	}
};

ID3D11Texture2D *staging_tex = nullptr;
void CreateStagingTexture(ID3D11Device *device, UINT width, UINT height)
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // NDI prefers BGRA
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.BindFlags = 0;

	device->CreateTexture2D(&desc, nullptr, &staging_tex);
}

void NDISender::CaptureLoop(std::wstring deviceName, std::string ndiName)
{
	if (!m_capture.Init(deviceName)) {
		log_file("Failed to init DXGI capture (adapter)\n");
		return;
	}

	UINT width, height, refreshRateNum, refreshRateDen;
	m_capture.GetDimensions(width, height, refreshRateNum, refreshRateDen);

	// Get D3D device/context from capture (expose via getter)
	ID3D11Device *device = m_capture.GetDevice().Get();
	ID3D11DeviceContext *context = m_capture.GetContext().Get();

	FrameReader reader;
	reader.Init(device, width, height);

	// Create an NDI source that is called deviceName
	NDIlib_send_create_t NDI_send_create_desc = {0};
	NDI_send_create_desc.p_ndi_name = ndiName.c_str();
	NDI_send_create_desc.clock_video = true;
	NDI_send_create_desc.clock_audio = false;
    log_file("Creating NDI Sender with name: %s\n", NDI_send_create_desc.p_ndi_name);

	NDIlib_video_frame_v2_t video_frame = {};
	video_frame.frame_rate_N = refreshRateNum;
	video_frame.frame_rate_D = refreshRateDen;
	log_file("Capturing %ux%u, at %u/%u\n", width, height, refreshRateNum, refreshRateDen);

	NDIlib_send_instance_t pNDI_send = ndiLib->send_create(&NDI_send_create_desc);
	if (!pNDI_send) {
		m_captureRunning = false;
		log_file("Failed to create NDI sender\n");
	}

	int frameCounter = 0;

	while (m_captureRunning) {
		ID3D11Texture2D *tex = nullptr;
		DXGI_OUTDUPL_FRAME_INFO frameInfo = {};

		++frameCounter;
		if ((frameCounter % 30) == 0) {
			// Periodically check display settings in case resolution changed
			DEVMODEW cur = {};
			cur.dmSize = sizeof(DEVMODEW);
			if (EnumDisplaySettingsW(deviceName.c_str(), ENUM_CURRENT_SETTINGS, &cur)) {
				if ((UINT)cur.dmPelsWidth != width || (UINT)cur.dmPelsHeight != height) {
					// resolution changed � reinitialize capture, reader and NDI sender
					log_file("Resolution change detected on %ls: %ux%u -> %ux%u\n",
						 deviceName.c_str(), width, height, cur.dmPelsWidth, cur.dmPelsHeight);
					width = cur.dmPelsWidth;
					height = cur.dmPelsHeight;

					// destroy and recreate NDI sender
					if (pNDI_send) {
						ndiLib->send_destroy(pNDI_send);
						pNDI_send = nullptr;
					}

					// reinit capture
					m_capture = DXGICapture();
					if (!m_capture.Init(deviceName)) {
						log_file("Failed to re-init DXGI capture after resolution change\n");
						m_captureRunning = false;
						break;
					}
					device = m_capture.GetDevice().Get();
					context = m_capture.GetContext().Get();

					// reinit reader with new size
					reader.Init(device, width, height);

					// recreate NDI sender
					pNDI_send = ndiLib->send_create(&NDI_send_create_desc);
					if (!pNDI_send) {
						log_file("Failed to recreate NDI sender after resolution change\n");
						m_captureRunning = false;
						break;
					}
					log_file(
						"Successfully handled resolution change and recreated capture and NDI sender\n");
				}
			}
		}
		if (!m_capture.CaptureFrame(&tex, &frameInfo)) {
			// If duplication was lost or adapter gone, sleep a bit and allow watcher to stop us.
			//log_file("CaptureFrame failed, likely due to display change. Waiting for watcher to stop capture loop...\n");
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			continue;
		}
		// Only process if there are actual updates
		if (frameInfo.LastPresentTime.QuadPart != 0) {
			if (reader.ReadPixels(context, tex, width, height, video_frame)) {
				ndiLib->send_send_video_async_v2(pNDI_send, &video_frame);
				//ndiLib->send_send_video_v2(pNDI_send, &video_frame);
				reader.Unmap(context);
			}
		}

		m_capture.ReleaseFrame();
		tex->Release();
		++frameCounter;
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	if (pNDI_send) {
		ndiLib->send_destroy(pNDI_send);
	}

	log_file("CaptureLoop exiting\n");
}
