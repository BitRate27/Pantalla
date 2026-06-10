// DXGICapture.cpp
#include "DXGICapture.h"
#include <iostream>
#include <comdef.h>
#include <thread>
#include <chrono>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

bool DXGICapture::Init(const std::wstring &outputDeviceName)
{
	m_outputDeviceName = outputDeviceName;

	ComPtr<IDXGIFactory1> factory;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)&factory);
	if (FAILED(hr) || !factory) {
		log_file("Failed to create DXGI factory: %ld\n", hr);
		return false;
	}
	log_file("Capture::Init %ls\n", outputDeviceName.c_str());
	// enumerate adapters and their outputs, look for output whose DXGI_OUTPUT_DESC::DeviceName matches outputDeviceName
	ComPtr<IDXGIAdapter> tmpAdapter;
	ComPtr<IDXGIOutput> tmpOutput;
	bool found = false;
	for (UINT ai = 0; SUCCEEDED(factory->EnumAdapters(ai, &tmpAdapter)) && !found; ++ai) {
		for (UINT oi = 0; SUCCEEDED(tmpAdapter->EnumOutputs(oi, &tmpOutput)) && !found; ++oi) {
			DXGI_OUTPUT_DESC outDesc = {};
			if (SUCCEEDED(tmpOutput->GetDesc(&outDesc))) {
				std::wstring descName(outDesc.DeviceName);
				if (descName == outputDeviceName) {
					// create duplication on this output
					D3D_FEATURE_LEVEL featureLevel;
					log_file("Creating D3D11 device for adapter %d output %d\n", ai, oi);
					hr = D3D11CreateDevice(tmpAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
							       D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
							       D3D11_SDK_VERSION, &m_Device, &featureLevel, &m_Context);
					if (FAILED(hr)) {
						log_file("Failed to create D3D11 device: %ld\n", hr);
						return false;
					}

					ComPtr<IDXGIDevice> dxgiDevice;
					m_Device.As(&dxgiDevice);

					ComPtr<IDXGIOutput1> output1;
					tmpOutput.As(&output1);

					hr = output1->DuplicateOutput(m_Device.Get(), &m_Duplication);
					log_file("DuplicateOutput hr: %ld\n", hr);
					if (SUCCEEDED(hr)) {
						m_Duplication->GetDesc(&m_DuplDesc);
						log_file("Successfully created output duplication, system_memory=%d\n",
							 m_DuplDesc.DesktopImageInSystemMemory);
						found = true;
					}
				}
			}
			tmpOutput.Reset();
		}
		tmpAdapter.Reset();
	}
	if (!found) {
		log_file("Failed to find output matching device name %ls\n", outputDeviceName.c_str());
	}
	return found;
}

bool DXGICapture::CaptureFrame(ID3D11Texture2D **outTexture, DXGI_OUTDUPL_FRAME_INFO *outInfo)
{
	ComPtr<IDXGIResource> resource;
	HRESULT hr = S_OK;

	if (!m_Duplication)
		return false;

	// Try AcquireNextFrame
	hr = m_Duplication->AcquireNextFrame(35, outInfo, &resource);

	if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
		// No new frame, not an error
		return false;
	}

	if (FAILED(hr)) {
		// Log details
		_com_error err(hr);
		log_file("AcquireNextFrame failed hr=0x%08X (%ld): %s\n", hr, hr, err.ErrorMessage());
		if (m_Device) {
			HRESULT r2 = m_Device->GetDeviceRemovedReason();
			_com_error e2(r2);
			log_file("GetDeviceRemovedReason =0x%08X (%ld): %s\n", r2, r2, e2.ErrorMessage());
		}

		// Attempt reinit retries
		for (int attempt = 1; attempt <= REINIT_RETRIES; ++attempt) {
			log_file("Attempting reinit %d/%d\n", attempt, REINIT_RETRIES);
			// Release old duplication/device/context
			m_Duplication.Reset();
			m_Context.Reset();
			m_Device.Reset();

			// small backoff before retrying
			std::this_thread::sleep_for(std::chrono::milliseconds(200 * attempt));

			if (m_outputDeviceName.empty()) {
				log_file("No stored output name; cannot reinit\n");
				continue;
			}

			if (Init(m_outputDeviceName)) {
				log_file("Reinit successful on attempt %d\n", attempt);
				// Try acquire once more
				hr = m_Duplication->AcquireNextFrame(16, outInfo, &resource);
				if (SUCCEEDED(hr)) {
					// Got a frame after reinit
					hr = resource.As((ComPtr<ID3D11Texture2D> *)outTexture);
					return SUCCEEDED(hr);
				}
				// otherwise continue retry loop
			} else {
				log_file("Reinit attempt %d failed\n", attempt);
			}
		}

		log_file("All reinit attempts failed\n");
		return false;
	}

	// If we get here, AcquireNextFrame succeeded
	hr = resource.As((ComPtr<ID3D11Texture2D> *)outTexture);
	if (!SUCCEEDED(hr)) {
		_com_error err(hr);
		log_file("Converting outTexture to resource failed hr=0x%08X (%ld): %s\n", hr, hr, err.ErrorMessage());
	}
	return SUCCEEDED(hr);
}

void DXGICapture::ReleaseFrame()
{
	if (m_Duplication)
		m_Duplication->ReleaseFrame();
}

void DXGICapture::GetDimensions(UINT &width, UINT &height, UINT &refreshRateNum, UINT &refreshRateDen)
{
	width = m_DuplDesc.ModeDesc.Width;
	height = m_DuplDesc.ModeDesc.Height;
	refreshRateNum = m_DuplDesc.ModeDesc.RefreshRate.Numerator;
	refreshRateDen = m_DuplDesc.ModeDesc.RefreshRate.Denominator;
}
