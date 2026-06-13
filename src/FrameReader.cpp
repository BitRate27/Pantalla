// FrameReader.cpp
#pragma once
#include <windows.h>
#include "global.h"
#include "FrameReader.h"

bool FrameReader::Init(ID3D11Device *device, UINT width, UINT height)
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // BGRA — matches NDI BGRX
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	//copyPerfTimer.init("FrameReader::CopyResource", 100);
	//mapPerfTimer.init("FrameReader::Map", 100);

	return SUCCEEDED(device->CreateTexture2D(&desc, nullptr, &m_StagingTex));
}

bool FrameReader::ReadPixels(ID3D11DeviceContext *ctx, ID3D11Texture2D *gpuTex, UINT width, UINT height,
			     NDIlib_video_frame_v2_t &ndi_frame)
{
	// Copy GPU texture -> staging (CPU-readable)
	//copyPerfTimer.start();
	ctx->CopyResource(m_StagingTex.Get(), gpuTex);
	//copyPerfTimer.end();

	//mapPerfTimer.start();
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	HRESULT hr = ctx->Map(m_StagingTex.Get(), 0, D3D11_MAP_READ, 0, &mapped);
	//mapPerfTimer.end();
	if (FAILED(hr))
		return false;

	ndi_frame.xres = width;
	ndi_frame.yres = height;
	ndi_frame.FourCC = NDIlib_FourCC_video_type_BGRA;
	ndi_frame.line_stride_in_bytes = mapped.RowPitch; // critical: use GPU row pitch
	ndi_frame.p_data = static_cast<uint8_t *>(mapped.pData);
	ndi_frame.picture_aspect_ratio = (float)width / (float)height;
	ndi_frame.frame_format_type = NDIlib_frame_format_type_progressive;
	ndi_frame.timecode = NDIlib_send_timecode_synthesize;

	return true;
}

void FrameReader::Unmap(ID3D11DeviceContext *ctx)
{
	ctx->Unmap(m_StagingTex.Get(), 0);
}
