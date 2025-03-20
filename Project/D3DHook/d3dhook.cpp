#include "..\Main\main.h"

HRESULT __stdcall Hooked_Present(IDirect3DDevice9 *pDevice, CONST RECT *pSrcRect, CONST RECT *pDestRect, HWND hDestWindow, CONST RGNDATA *pDirtyRegion)
{

	if (!pDevice)
		return pD3DHook->Orginal_Present(pDevice, pSrcRect, pDestRect, hDestWindow, pDirtyRegion);

	if (pD3DHook->bD3DRenderInit == false)
	{
			pD3DHook->pRender->Initialize(pDevice);
			pD3DHook->pD3DFont->Initialize(pDevice);
			pD3DHook->pD3DFontFixed->Initialize(pDevice);
			pD3DHook->pD3DFontFixedSmall->Initialize(pDevice);
			pD3DHook->pD3DFontChat->Initialize(pDevice);
			pD3DHook->pD3DFont_sampStuff->Initialize(pDevice);
			pD3DHook->pD3DFontDebugWnd->Initialize(pDevice);
			pD3DHook->bD3DRenderInit = true;
	}
	if (pD3DHook->pRender) {
		on_present(pDevice, pSrcRect, pDestRect, hDestWindow, pDirtyRegion);
	}
	MainLoop();
	return pD3DHook->Orginal_Present(pDevice, pSrcRect, pDestRect, hDestWindow, pDirtyRegion);
}

HRESULT __stdcall Hooked_Reset(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentParams)
{
	if (!pDevice)
		return pD3DHook->Orginal_Reset(pDevice, pPresentParams);
	ImGui_ImplDX9_InvalidateDeviceObjects();
	if (pD3DHook->bD3DRenderInit == true)
	{
		pD3DHook->pD3DFont->Invalidate();
		pD3DHook->pD3DFontFixed->Invalidate();
		pD3DHook->pD3DFontFixedSmall->Invalidate();
		pD3DHook->pD3DFontChat->Invalidate();
		pD3DHook->pD3DFont_sampStuff->Invalidate();
		pD3DHook->pD3DFontDebugWnd->Invalidate();

		pD3DHook->pRender->Invalidate();
		pD3DHook->bD3DRenderInit = false;
	}

	HRESULT ret = pD3DHook->Orginal_Reset(pDevice, pPresentParams);

	if (ret == D3D_OK)
	{
		pD3DHook->pRender->Initialize(pDevice);
		pD3DHook->pD3DFont->Initialize(pDevice);
		pD3DHook->pD3DFontFixed->Initialize(pDevice);
		pD3DHook->pD3DFontFixedSmall->Initialize(pDevice);
		pD3DHook->pD3DFontChat->Initialize(pDevice);
		pD3DHook->pD3DFont_sampStuff->Initialize(pDevice);
		pD3DHook->pD3DFontDebugWnd->Initialize(pDevice);

		pD3DHook->pPresentParam = *pPresentParams;

		pD3DHook->bD3DRenderInit = true;
	}

	return ret;
}


