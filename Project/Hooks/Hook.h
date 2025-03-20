#pragma once
#include <Windows.h>
#include <string>
#include "FindSignature.h"
#include "..\Librarys\MinHook.h"
#include "CVector.h"
void Hooks_Install();
void Hooks_Check();
class Hooks
{
public:
    static void InstalClientHook();
    static void InstallHook();
    static void RemoveHooks();
};
bool __fastcall LoadScriptFromBuffer(void* ECX, void* EDX, const char* cpInBuffer, unsigned int uiInSize, const char* szFileName);
extern DWORD m_PlayerManager;
extern void* GetNametagColor_ECX;
extern DWORD ScreenShotTickCount;
extern bool Send_Script_Packet;
bool __cdecl GetPedBonePosition(void* Ped, int bone, CVector& vecPosition);
bool __cdecl KillPed(void* Entity, void* pKiller, unsigned char ucKillerWeapon, unsigned char ucBodyPart, bool bStealth);