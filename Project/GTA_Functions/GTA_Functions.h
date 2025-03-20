#pragma once
#include "..\Main\main.h"
#include "plugin.h"
#include "..\Visuals\ESP.h"
void CalcScreenCoors(D3DXVECTOR3* vecWorld, D3DXVECTOR3* vecScreen);
void W2S(CVector* vecWorld, CVector* vecScreen);
void AirBreak();
void GetAimingCenter(float& x, float& y);