#include "ESP.h"
#include "..\GTA_Functions\GTA_Functions.h"
#include "..\IDA\defs.h"
#include "plugin.h"
DWORD ClientModule;
CClientEntity* AimPlayer = NULL;
const char* WeaponNames_[] = {

"Fist",

"Brassknuckle",

"Golfclub",

"Nightstick",

"Knife",

"Baseballbat",

"Shovel",

"Pool Cue",

"Katana",

"Chainsaw",

// gifts

"Dildo1",          // 10

"Dildo2",

"Vibe1",

"Vibe2",

"Flowers",

"Cane",

"Grenade",

"Teargas",

"Molotov",

"Rocket",

"Rocket Hs",        // 20

"Freefall Bomb",

// FIRST SKILL WEAPON

"Pistol",          // handguns

"Pistol Silenced",

"Desert Eagle",

"Shotgun",          // shotguns

"Sawnoff Shotgun",  // one handed

"Spas12 Shotgun",

"Micro Uzi",        // submachine guns

"Mp5",

"Ak47",            // 30      // machine guns

"M4",

"Tec9",            // this uses stat from the micro_uzi

// END SKILL WEAPONS

"Countryrifle",     // rifles

"Sniperrifle",

"Rocketlauncher",   // specials

"Rocketlauncher Hs",

"Flamethrower",

"Minigun",

"Remote Satchel Charge",

"Detonator",        // 40 // plastic explosive

"Spraycan",

"Extinguisher",

"Camera",

"Nightvision",

"Infrared",

"Parachute",

"Last Weapontype",

"Armour",

// these are possible ways to die

"Rammedbycar",

"Runoverbycar",     // 50

"Explosion",

"Uzi Driveby",

"Drowning",

"Fall",

"Unidentified",    // Used for damage being done

"Anymelee",

"Anyweapon",

"Flare",

// Added by us

"Tank Grenade",

"Invalid"

};

unsigned int __cdecl Reverse_Color(int a1, unsigned int a2)
{
	float v2; // xmm3_4
	v2 = 255.0 - (float)HIBYTE(a2);
	return (a1 & 0xFF000000)
		+ ((((unsigned __int8)(int)(float)((float)((float)(int)(HIBYTE(a2) * BYTE1(a2)) + (float)((float)BYTE1(a1) * v2))
			* 0.0039215689) | ((unsigned __int8)(int)(float)((float)((float)(int)(HIBYTE(a2) * BYTE2(a2))
				+ (float)((float)BYTE2(a1) * v2))
				* 0.0039215689) << 8)) << 8) | (unsigned __int8)(int)(float)((float)((float)(int)(HIBYTE(a2) * (unsigned __int8)a2) + (float)((float)(unsigned __int8)a1 * v2)) * 0.0039215689));
}
int GetTotalClientEntities()
{
	if (m_PlayerManager != 0x0)
	{
		return *(int*)(m_PlayerManager + 0xC);
	}
}

CClientEntity* GetClientEntityFromID(int index)
{
	if (m_PlayerManager != 0x0)
	{
		DWORD CClientPlayerManager = m_PlayerManager;
		DWORD ClientsArray = *(DWORD*)(CClientPlayerManager + 0x10);
		if ((CClientEntity*)(ClientsArray + (index * 0x4)))
		{
			CClientEntity* _Ent = *(CClientEntity**)(ClientsArray + (index * 0x4));
			if (_Ent && _Ent->GetID() > 100)
				return _Ent;
		}
	}
	return nullptr;
}

void SetAimTarget()
{
	for (int i = 0; i < GetTotalClientEntities(); ++i)
	{
		CClientEntity* Ped = GetClientEntityFromID(i);
		if (Ped)
		{
			CVector PedPosition = CVector(0.0f, 0.0f, 0.0f);
			Ped->GetPosition(PedPosition);

			CVector* FeetSP = new CVector();
			CVector* HeadSP = new CVector();
			float		ESP_tag_player_D3DBox_pixelOffsetX = -0.5f;
			float		ESP_tag_player_D3DBox_pixelOffsetY = -0.5f;

			CVector* FeetPos = new CVector(PedPosition.x, PedPosition.y, PedPosition.z - 1.0f);
			CVector* HeadPos = new CVector(PedPosition.x, PedPosition.y, PedPosition.z + 1.0f);

			W2S(FeetPos, FeetSP);
			W2S(HeadPos, HeadSP);

			if (FeetSP->z < 1.0) continue;
			if (HeadSP->z < 1.0) continue;

			float Mesafe = 0.0;
			float centerX, centerY;
			GetAimingCenter(centerX, centerY);

			double GGNeah = sqrt(((GetSystemMetrics(SM_CXSCREEN) * 0.5299999714f - HeadSP->x) * (GetSystemMetrics(SM_CXSCREEN) * 0.5299999714f - HeadSP->x)) + ((GetSystemMetrics(SM_CYSCREEN) * 0.4f - HeadSP->y) * (GetSystemMetrics(SM_CYSCREEN) * 0.4f - HeadSP->y)));
			if (GGNeah > Mesafe && Mesafe != 0.0) continue;
			AimPlayer = Ped;
			Mesafe = (float)GGNeah;
		}
	}

	
}

void Player_ESP() {
	if (m_PlayerManager != 0x0)
	{
		float dist = 1000;
		DWORD local_player = *(DWORD*)(0xB6F5F0);
		CPed* pPedSelf = FindPlayerPed();
		bool Valid_AimPlayer = false;
		if ((DWORD)GetModuleHandle(("client.dll")) != ClientModule && ClientModule != 0x0)
		{
			ClientModule = (DWORD)GetModuleHandle(xorstr_("client.dll"));
		}
		if (ClientModule == 0x0 && local_player)
		{
			ClientModule = (DWORD)GetModuleHandle(xorstr_("client.dll"));
		}
		if (ClientModule == 0x0 || !local_player) return;
		if (VisualsTab::Airbreak && pPedSelf->IsVisible())
		{
			AirBreak();
		}
		float Mesafe = 0.0;
		if (VisualsTab::enableESP && pPedSelf->IsVisible())
		{
			SetAimTarget();
			for (int i = 0; i < GetTotalClientEntities(); ++i)
			{
				CClientEntity* Ped = GetClientEntityFromID(i);
				if (Ped)
				{
					CVector PedPosition = CVector(0.0f, 0.0f, 0.0f);
					Ped->GetPosition(PedPosition);
					
					CVector* FeetSP = new CVector();
					CVector* HeadSP = new CVector();
					float		ESP_tag_player_D3DBox_pixelOffsetX = -0.5f;
					float		ESP_tag_player_D3DBox_pixelOffsetY = -0.5f;

					CVector* FeetPos = new CVector(PedPosition.x, PedPosition.y, PedPosition.z - 1.0f);
					CVector* HeadPos = new CVector(PedPosition.x, PedPosition.y, PedPosition.z + 1.0f);
					CVector* Self_Pos = &pPedSelf->GetPosition();

					float dx = Self_Pos->x - HeadPos->x;
					float dy = Self_Pos->y - HeadPos->y;
					float dz = Self_Pos->z - HeadPos->z;

					float distance = sqrt(dx * dx + dy * dy + dz * dz);

					if (distance >= VisualsTab::distance) continue;

					W2S(FeetPos, FeetSP);
					W2S(HeadPos, HeadSP);

					if (FeetSP->z < 1.0) continue;
					if (HeadSP->z < 1.0) continue;


					if (pPedSelf->GetPosition().x == PedPosition.x && pPedSelf->GetPosition().y == PedPosition.y && pPedSelf->GetPosition().z == PedPosition.z)
					{
						if (GetAsyncKeyState(VK_CONTROL) && VisualsTab::Crasher && AimPlayer)
						{
							static DWORD timer = 0;
							if (GetTickCount64() - 500 > timer)
							{
								
							}
						}
					}
					else
					{

					}



					char* Ped_Name = ReadName((DWORD)Ped + 0x618);
					float Health = *(float*)((DWORD)Ped + 0x318);
					float Armor = *(float*)((DWORD)Ped + 0x31C);
					int Ping = *(int*)((DWORD)Ped + 0x630);
					bool m_bNametagColorOverridden = *(bool*)((DWORD)Ped + 1755);
					
				
					unsigned char m_ucNametagColorR, m_ucNametagColorG, m_ucNametagColorB;
					m_ucNametagColorR = *(unsigned char*)((DWORD)Ped + 1752);
					m_ucNametagColorG = *(unsigned char*)((DWORD)Ped + 1753);
					m_ucNametagColorB = *(unsigned char*)((DWORD)Ped + 1754);
					char buf[256];

					if (Health > 100.0) Health = 100.0;

					float Color = -0.17539214;
					if (Health < 100.0 && Health > 60.0)
					{
						Color = COERCE_FLOAT(
							Reverse_Color(
								-1103914497,
								((int)(float)((float)((float)(40.0 - (float)(Health - 60.0)) * 0.025) * 255.0) << 24) | 0xC800C8));
					}
					if (Health < 60.0 && Health > 20.0)
					{
						Color = COERCE_FLOAT(
							Reverse_Color(
								-1094188856,
								((int)(float)((float)((float)(40.0 - (float)(Health - 20.0)) * 0.025) * 255.0) << 24) | 0xC83333));
					}
					if (Health < 20.0 && Health > 0.0)
						Color = COERCE_FLOAT(
							Reverse_Color(
								-1094175949,
								((int)(float)((float)((float)(20.0 - Health) * 0.050000001) * 255.0) << 24) | 0x151515));

					float scale = 1.1f;
					int Fake_Ping;
					Fake_Ping = Ping;
					if (Ping > 98) Fake_Ping = 98;


					pD3DHook->pRender->D3DBox(HeadSP->x + ESP_tag_player_D3DBox_pixelOffsetX,
						HeadSP->y + ESP_tag_player_D3DBox_pixelOffsetY,
						100.0f * scale, 30.0f * scale, 1862270976);

					pD3DHook->pRender->D3DBox(HeadSP->x + 1.0f + ESP_tag_player_D3DBox_pixelOffsetX,
						HeadSP->y + 1.0f + ESP_tag_player_D3DBox_pixelOffsetY,
						(Health - 2.0f) * scale, 8.0f * scale, LODWORD(Color));

					pD3DHook->pRender->D3DBox(HeadSP->x + 1.0f + ESP_tag_player_D3DBox_pixelOffsetX,
						HeadSP->y + 1.0f + ESP_tag_player_D3DBox_pixelOffsetY + 20.f * scale,
						Fake_Ping * scale, 8.0f * scale, 1862336511);

					float offY = ESP_tag_player_D3DBox_pixelOffsetY;
					float armbar = Armor;
					if (armbar > 100.0f) armbar = 100.0f;

					pD3DHook->pRender->D3DBox(HeadSP->x + 1.0f + ESP_tag_player_D3DBox_pixelOffsetX,
						HeadSP->y + 1.0f + ESP_tag_player_D3DBox_pixelOffsetY + 10.f * scale,
						(Armor - 2.0f) * scale, 8.0f * scale, 1876745436);
					offY += 10.0f * scale;


					float h = pD3DHook->pD3DFontFixed->DrawHeight() + 1;
					_snprintf_s(buf, sizeof(buf) - 1, xorstr_("Health: %d"), (int)Health);
					pD3DHook->pD3DFontFixedSmall->Print(HeadSP->x + 8.0f * scale,
						HeadSP->y - h + 10.0f * scale,
						-2097190400, buf);

					_snprintf_s(buf, sizeof(buf) - 1, xorstr_("Armor: %d"), (int)Armor);
					pD3DHook->pD3DFontFixedSmall->Print(HeadSP->x + 8.0f * scale,
						HeadSP->y - h + 20.0f * scale,
						-2097190400, buf);
					_snprintf_s(buf, sizeof(buf) - 1, xorstr_("Ping: %d"), Ping);
					pD3DHook->pD3DFontFixedSmall->Print(HeadSP->x + 8.0f * scale,
						HeadSP->y - h + 30.0f * scale,
						-2097190400, buf);

					h = pD3DHook->pD3DFont_sampStuff->DrawHeight() - 1;
					_snprintf_s(buf, sizeof(buf) - 1, xorstr_("%s"), Ped_Name);
					if (m_bNametagColorOverridden)
					{
						pD3DHook->pD3DFont_sampStuff->Print(HeadSP->x, HeadSP->y - h - 3.0f, D3DCOLOR_XRGB(m_ucNametagColorR, m_ucNametagColorG, m_ucNametagColorB), buf);
					}
					else
					{
						pD3DHook->pD3DFont_sampStuff->Print(HeadSP->x, HeadSP->y - h - 3.0f, D3DCOLOR_ARGB(0xFF, 0x99, 0x99, 0x99), buf);
					}

					if (Health != 0.0)
					{
						std::string weaponText = "";
						SFixedArray<eWeaponType, WEAPONSLOT_MAX> m_WeaponTypes;
						eWeaponSlot                              m_CurrentWeaponSlot = *(eWeaponSlot*)((DWORD)Ped + 888);
						m_WeaponTypes = *(SFixedArray<eWeaponType, WEAPONSLOT_MAX>*)((DWORD)Ped + 892);
						/*/for (size_t i = 0; i < 2000; i++)
						{
							m_CurrentWeaponSlot = *(eWeaponSlot*)((DWORD)Ped + i);
							if (m_CurrentWeaponSlot == WEAPONSLOT_TYPE_HANDGUN)
							{
								printf("CurrentWeaponSlot: %d offset:%d\n", m_CurrentWeaponSlot, i);
							}
						}*/
						weaponText = WeaponNames_[m_WeaponTypes[m_CurrentWeaponSlot]];
						pD3DHook->pD3DFontFixedSmall->PrintShadow(HeadSP->x, HeadSP->y + 40, -26368, (weaponText.c_str()));
					}

					for (int bone_id = BONE_PELVIS1; bone_id <= BONE_RIGHTFOOT; ++bone_id)
					{
						if (bone_id >= BONE_PELVIS1 && bone_id <= BONE_HEAD ||
							bone_id >= BONE_RIGHTUPPERTORSO && bone_id <= BONE_RIGHTTHUMB ||
							bone_id >= BONE_LEFTUPPERTORSO && bone_id <= BONE_LEFTTHUMB ||
							bone_id >= BONE_LEFTHIP && bone_id <= BONE_LEFTFOOT ||
							bone_id >= BONE_RIGHTHIP && bone_id <= BONE_RIGHTFOOT)
						{
							CVector vHead, vFoot;
							GetPedBonePosition(Ped, (eBone)BONE_HEAD, vHead);
							GetPedBonePosition(Ped,(eBone)BONE_LEFTFOOT, vFoot);

							D3DXVECTOR3 headPos{ vHead.x, vHead.y, vHead.z }, footPos{ vFoot.x, vFoot.y, vFoot.z };
							D3DXVECTOR3 headScreenPos, footScreenPos;

							// Get head and foot position on screen
							CalcScreenCoors(&headPos, &headScreenPos);
							CalcScreenCoors(&footPos, &footScreenPos);


							D3DCOLOR boxColor = D3DCOLOR_XRGB(0, 0, 255);


							// Calculate x, y, height, width, etc.
							float tmp = pow((headScreenPos.x - footScreenPos.x), 2);
							float tmp2 = pow((headScreenPos.y - footScreenPos.y), 2);

							float height = (sqrt(tmp + tmp2)) * 1.2f;
							float width = height / 2.0f;

							static int state = 0;
							static DWORD Time = 0;
							if (headScreenPos.z > 1.f || footScreenPos.z > 0.f)
							{// 293 -65536 wallner -14805761
								if (AimPlayer == Ped)
								{
									pD3DHook->pRender->D3DBoxBorder(headScreenPos.x - (width / 2.0f), headScreenPos.y, width, height, -65536, 0);
								}
								else
								{
									pD3DHook->pRender->D3DBoxBorder(headScreenPos.x - (width / 2.0f), headScreenPos.y, width, height, -14805761, 0);
								}
								if (AimPlayer == NULL)
								{
									pD3DHook->pRender->D3DBoxBorder(headScreenPos.x - (width / 2.0f), headScreenPos.y, width, height, -14805761, 0);
								}
								
							}
						}
					}
					delete FeetSP;
					delete HeadSP;
					delete FeetPos;
					delete HeadPos;
				}
			}
		}
	}
}