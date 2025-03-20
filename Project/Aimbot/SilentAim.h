#pragma once
#include "..\Main\main.h"
#include "../CallHook/ccallhook.h"
class SilentAim{
public:
	explicit SilentAim();
	virtual ~SilentAim();

private:
	CCallHook* hook1, * hook2, * hook3;
	CCallHook* hookRot1 /*, *hookRot2*/;

	bool wsState;
	int curWid;
	class CWeapon* weapInfo = nullptr;

	float originalAngle = 0.0f;
	float targetAngle = 0.0f;
	class CPed* targetPed = nullptr;
	CVector camTarget;
	CVector posTarget;
	DWORD lastShot;

	void HookTarget(CVector& gun, CVector& vec);
	void HookTarget1();
	void HookTarget2();
	void HookTarget3();
	void HookRotation();
	void HookRotation2();
};