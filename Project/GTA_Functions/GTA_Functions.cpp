#include "GTA_Functions.h"

void CalcScreenCoors(D3DXVECTOR3* vecWorld, D3DXVECTOR3* vecScreen)
{
	D3DXMATRIX    m((float*)(0xB6FA2C));
	DWORD* dwLenX = (DWORD*)(0xC17044);
	DWORD* dwLenY = (DWORD*)(0xC17048);
	
	vecScreen->x = (vecWorld->z * m._31) + (vecWorld->y * m._21) + (vecWorld->x * m._11) + m._41;
	vecScreen->y = (vecWorld->z * m._32) + (vecWorld->y * m._22) + (vecWorld->x * m._12) + m._42;
	vecScreen->z = (vecWorld->z * m._33) + (vecWorld->y * m._23) + (vecWorld->x * m._13) + m._43;

	double    fRecip = (double)1.0 / vecScreen->z;
	vecScreen->x *= (float)(fRecip * (*dwLenX));
	vecScreen->y *= (float)(fRecip * (*dwLenY));
}
void GetAimingCenter(float& x, float& y)
{
	x = (float)pD3DHook->pPresentParam.BackBufferWidth * 0.5299999714f;
	y = (float)pD3DHook->pPresentParam.BackBufferHeight * 0.4f;
	return;
}
void W2S(CVector* vecWorld, CVector* vecScreen)
{
    D3DXMATRIX m((float*)(0xB6FA2C));
    DWORD* dwLenX = (DWORD*)(0xC17044);
    DWORD* dwLenY = (DWORD*)(0xC17048);

    vecScreen->x = (vecWorld->z * m._31) + (vecWorld->y * m._21) + (vecWorld->x * m._11) + m._41;
    vecScreen->y = (vecWorld->z * m._32) + (vecWorld->y * m._22) + (vecWorld->x * m._12) + m._42;
    vecScreen->z = (vecWorld->z * m._33) + (vecWorld->y * m._23) + (vecWorld->x * m._13) + m._43;

    // Z koordinatýnýn tersini alarak ekran çözünürlüðüne dönüþtür
    double fRecip = (double)1.0 / vecScreen->z;
    vecScreen->x *= (float)(fRecip * (*dwLenX)); // Ekran geniþliði ile çarp
    vecScreen->y *= (float)(fRecip * (*dwLenY)); // Ekran yüksekliði ile çarp
}
float vecLength(CVector vecPos)
{
	return sqrt((vecPos.x * vecPos.x) + (vecPos.y * vecPos.y) + (vecPos.z * vecPos.z));
}
void AirBreak()
{
	if (VisualsTab::Airbreak)
	{
		float fCameraRotation = *(float*)0xB6F178;
		float fSpeed = 10.f;
		CPed* pPedSelf = FindPlayerPed();
		CVector* nVec = &pPedSelf->m_matrix->pos;
		pPedSelf->m_fCurrentRotation = pPedSelf->m_fAimingRotation = -fCameraRotation;
		pPedSelf->m_vecMoveSpeed.z = 0.f;

		CVehicle* pVehicle = FindPlayerVehicle(-1, false);
		if (pVehicle)
		{
			nVec = &pVehicle->m_matrix->pos;
			float fDiff = vecLength(pVehicle->m_matrix->pos - *nVec);
			pVehicle->m_matrix->SetRotateZOnly(-fCameraRotation);
			pVehicle->m_matrix->pos.x = nVec->x - sinf(fCameraRotation) * fDiff;
			pVehicle->m_matrix->pos.y = nVec->y - cosf(fCameraRotation) * fDiff;
			pVehicle->m_vecMoveSpeed.x = pVehicle->m_vecMoveSpeed.y = pVehicle->m_vecMoveSpeed.z = 0.f;
		}
		else if (VisualsTab::Airbreak)
		{
			pPedSelf->m_nPedFlags.bIsStanding = pPedSelf->m_nPedFlags.bWasStanding = pPedSelf->m_nPedFlags.bStayInSamePlace = true;
		}
		else
		{
			pPedSelf->m_nPedFlags.bIsStanding = true;
		}

		if (GetAsyncKeyState('W') & 0x8000)
		{
			nVec->x += sinf(fCameraRotation) * fSpeed;
			nVec->y += cosf(fCameraRotation) * fSpeed;
		}
		if (GetAsyncKeyState('S') & 0x8000)
		{
			nVec->x -= sinf(fCameraRotation) * fSpeed;
			nVec->y -= cosf(fCameraRotation) * fSpeed;
		}
		if (GetAsyncKeyState('D') & 0x8000)
		{
			nVec->x += cosf(fCameraRotation) * fSpeed;
			nVec->y -= sinf(fCameraRotation) * fSpeed;
		}
		if (GetAsyncKeyState('A') & 0x8000)
		{
			nVec->x -= cosf(fCameraRotation) * fSpeed;
			nVec->y += sinf(fCameraRotation) * fSpeed;
		}

		if (VisualsTab::Airbreak)
		{
			if (GetAsyncKeyState(VK_UP) & 0x8000)
			{
				nVec->z += fSpeed;
			}
			if (GetAsyncKeyState(VK_DOWN) & 0x8000)
			{
				nVec->z -= fSpeed;
			}
		}
	}
}
