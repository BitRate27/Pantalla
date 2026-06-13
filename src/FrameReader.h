// FrameReader.h

#include <setupapi.h>
#include <devguid.h>
#include <cfgmgr32.h>
#include <d3d11.h>
#include <wrl/client.h>
#include "ndi-loader.h"
#include "global.h"

using Microsoft::WRL::ComPtr;

class FrameReader {
public:
	bool Init(ID3D11Device *device, UINT width, UINT height);

	// Returns BGRA pixel data pointer
	bool ReadPixels(ID3D11DeviceContext *ctx, ID3D11Texture2D *gpuTex, UINT width, UINT height,
			NDIlib_video_frame_v2_t &ndi_frame);
	void Unmap(ID3D11DeviceContext *ctx);

private:
	ComPtr<ID3D11Texture2D> m_StagingTex;
	//PerfTimer copyPerfTimer;
	//PerfTimer mapPerfTimer;
};
