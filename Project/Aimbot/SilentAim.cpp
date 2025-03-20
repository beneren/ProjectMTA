#include "SilentAim.h"
#include "plugin.h"
#include "..\Visuals\ESP.h"
CPed* pPedSelf = FindPlayerPed();

SilentAim::SilentAim(){
	targetAngle = 0.0f;
	originalAngle = 0.0f;

	hook1 = new CCallHook((void*)0x007406C7, 5, 0, cp_after, sc_all, e_jmp);
	hook2 = new CCallHook((void*)0x00740B24, 8, 0, cp_after, sc_all, e_jmp);
	hook3 = new CCallHook((void*)0x0060B819, 5, 0, cp_after, sc_all, e_jmp);
	hook1->enable(this, &SilentAim::HookTarget1);
	hook2->enable(this, &SilentAim::HookTarget2);
	hook3->enable(this, &SilentAim::HookTarget3);

	hookRot1 = new CCallHook((void*)0x00522C26, 8, 0, cp_after, sc_all, e_jmp);
	// hookRot2 = new CCallHook( (void *)0x0052268B, 6 );
	hookRot1->enable(this, &SilentAim::HookRotation);
	// hookRot2->enable( this, &SilentAim::HookRotation2 );
}

SilentAim::~SilentAim() {
	delete hook1;
	delete hook2;
	delete hook3;
	delete hookRot1;
}

float GetAngle(float fX, float fY, const float& X, const float& Y) {
	float kx = X - fX;
	float ky = Y - fY;

	if (kx == 0) kx = 0.00001f;
	float t = kx / ky;
	if (t < 0) t = -t;

	float a = (float)(180 * atan((float)t) / M_PI);

	if ((kx <= 0) && (ky <= 0))
		a = 180 - a;
	else if ((kx >= 0) && (ky >= 0))
		a = 359.99999f - a;
	else if ((kx >= 0) && (ky <= 0))
		a = -(180 - a);

	return (a * M_PI) / 180.0f;
}


void SilentAim::HookTarget(CVector& gun, CVector& vec) {
	pPedSelf = FindPlayerPed();
	if (!pPedSelf) return;
	if (!pPedSelf->IsPointerValid()) return;
	if (!pPedSelf->GetMatrix()) return;
	RwV3d LocalBonePos;
	pPedSelf->GetBonePosition(LocalBonePos, 24, 0);
	if ((LocalBonePos - gun).Magnitude() > 1.0f) return;
	if (AimbotsTab::SilentAim)
	{
		bool rewrite = false;
		if (AimPlayer != NULL)
		{
			CVector OutAimPlayerPos;
			AimPlayer->GetPosition(OutAimPlayerPos);
			CVector Bone_Pos;
			eBone closestBone = BONE_UPPERTORSO;
			std::vector<int> validBones = { 8, 4, 22, 32, 301, 302, 51, 41, 52, 42, 53, 43 };

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(validBones.begin(), validBones.end(), g);

			int bone_id = validBones.front();

			GetPedBonePosition(AimPlayer, (eBone)bone_id, Bone_Pos);

			closestBone = (eBone)bone_id;
			rewrite = true;
			vec = Bone_Pos;
		}
		if (!wsState && rewrite) {
			memsafe::write((void*)0x00740701, false);
			memsafe::write((void*)0x00740703, false);
			memsafe::write((void*)0x00740709, false);
			memsafe::write((void*)0x0074B749, false);
			memsafe::write((void*)0x0074B74B, false);
			memsafe::write((void*)0x0074B751, false);
			/*/if (AimPlayer->) {
				memsafe::write((void*)0x00740707, false);
				memsafe::write((void*)0x0074B74F, false);
			}*/
			wsState = true;
		}
		else if (wsState && !rewrite) {
			memsafe::write((void*)0x00740701, true);
			memsafe::write((void*)0x00740703, true);
			memsafe::write((void*)0x00740707, true);
			memsafe::write((void*)0x00740709, true);
			memsafe::write((void*)0x0074B749, true);
			memsafe::write((void*)0x0074B74B, true);
			memsafe::write((void*)0x0074B74F, true);
			memsafe::write((void*)0x0074B751, true);
			wsState = false;
		}
	}
}

void SilentAim::HookTarget1() {
	auto  esp = hook1->reg86(r86::ESP);
	auto& vec = *(CVector*)(esp + 0x10);
	auto& gun = *(CVector*)(esp + 0x2C);
	HookTarget(gun, vec);
}

void SilentAim::HookTarget2() {
	auto  esp = hook2->reg86(r86::ESP);
	auto& vec = *(CVector*)(esp + 0x10);
	auto& gun = *(CVector*)(esp + 0x2C);
	HookTarget(gun, vec);
}

void SilentAim::HookTarget3() {
	pPedSelf = FindPlayerPed();
	if (!pPedSelf) return;
	if (!pPedSelf->IsPointerValid()) return;
	if (!pPedSelf->GetMatrix()) return;
	if (pPedSelf->GetWeapon() == 0) return;

	auto  esp = hook3->reg86(r86::ESP);
	auto& vec = *(CVector*)(esp + 0x38);
	auto& cam = *(CVector*)(esp + 0x44);

	RwV3d LocalBonePos;
	pPedSelf->GetBonePosition(LocalBonePos, 4, 0);
	if ((cam - LocalBonePos).Magnitude() > 1.0f) return;
	if (AimbotsTab::RotationSync && AimbotsTab::SilentAim)
	{
		if (AimPlayer != NULL)
		{
			CVector OutAimPlayerPos;
			CVector LocalPos;
			LocalPos = pPedSelf->GetPosition();
			AimPlayer->GetPosition(OutAimPlayerPos);
			targetAngle = GetAngle(LocalPos.x, LocalPos.y, OutAimPlayerPos.x, OutAimPlayerPos.y);
		}
		else
		{
			targetAngle = originalAngle;
		}
	}
	else
	{
		targetAngle = originalAngle;
	}
}

void SilentAim::HookRotation() {
	auto pAngle = (float*)(hookRot1->reg86(r86::ESP) + 0x68);
	originalAngle = *pAngle;

	if (!pPedSelf) return;
	if (!pPedSelf->IsPointerValid()) return;
	if (!pPedSelf->GetMatrix()) return;
	if (pPedSelf->GetWeapon() == 0) return;

	*pAngle = targetAngle;
}

void SilentAim::HookRotation2() {
	pPedSelf = FindPlayerPed();
	if (!pPedSelf) return;
	if (!pPedSelf->IsPointerValid()) return;
	if (!pPedSelf->GetMatrix()) return;
	if (pPedSelf->GetWeapon() == 0) return;

	*(float*)0x008CC530 = targetAngle - 1.5707964;
}

