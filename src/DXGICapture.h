// DXGICapture.h
#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <string>
#include "global.h"

using Microsoft::WRL::ComPtr;

class DXGICapture {
public:
	bool Init(const std::wstring &outputDeviceName);
	bool CaptureFrame(ID3D11Texture2D **outTexture, DXGI_OUTDUPL_FRAME_INFO *outInfo);
	void ReleaseFrame();
	void GetDimensions(UINT &width, UINT &height, UINT &refreshRateNum, UINT &refreshRateDen);
	ComPtr<ID3D11Device> GetDevice() const { return m_Device; }
	ComPtr<ID3D11DeviceContext> GetContext() const { return m_Context; }

private:
	ComPtr<ID3D11Device> m_Device;
	ComPtr<ID3D11DeviceContext> m_Context;
	ComPtr<IDXGIOutputDuplication> m_Duplication;
	DXGI_OUTDUPL_DESC m_DuplDesc = {};

	// The name of the output we initialized for. Stored so we can reinit on failure.
	std::wstring m_outputDeviceName;

	// How many times to try reinitializing on failure before giving up.
	static constexpr int REINIT_RETRIES = 3;
};
