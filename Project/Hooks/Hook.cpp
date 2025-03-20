#include "..\Main\main.h"
#include "Hook.h"
#include "CVector.h"
#include <iptypes.h>
#include "plugin.h"
#include "..\Visuals\CClientEntity.h"
#include "..\Mta_SDK/net/bitstream.h"
#include "..\Mta_SDK/network\CNet.h"
static bool Waiting; bool HookCheck = false;
bool Send_Script_Packet;
void* LuaVM;
void* Call_ECX[50000];
std::string Script_Name[50000];
void* Call_This[50000];
DWORD m_PlayerManager;
bool ScreenShotted; DWORD ScreenShotTickCount; int ScreenBypassType;
static bool qwe = false;
char* Script_Buff; size_t Script_sz; const char* Script_name;
bool SilentAim; float SilentAimPos[3];
bool DamageDividier; int DamageDividierValue;
bool LuaExecutor;
float sPedPos[3];
void* sLocalPed;
void* sTargetPed;
static bool scriptvar;
int Script_ID = 0;
void* Shot_ECX;
void* Shot_EDX;
static bool Shotted;
typedef bool(__cdecl* ptr_GetPedBonePosition)(void* Ped, int bone, CVector& vecPosition); ptr_GetPedBonePosition callGetBonePosition = nullptr; DWORD Offset_GetBonePosition = 0x0;
typedef void(__thiscall* ptrDoPulses)(void* This); DWORD Offset_DoPulses = 0x0; ptrDoPulses callDoPulses = nullptr;
typedef void(__thiscall* pShot)(void* ECX, int weaponType, const CVector& vecStart, const CVector& vecEnd, float fDamage, unsigned char ucHitZone, void* pRemoteDamagedPlayer); pShot ShotS = nullptr;  static DWORD SendBullet = 0x0; int sTargetBoneID;
typedef int(__cdecl* t_LuaLoadBuffer)(void* L, const char* buff, size_t sz, const char* name); t_LuaLoadBuffer callLuaLoadBuffer = nullptr; DWORD Offset_LuaLoadBuffer = 0x0;
typedef int(__cdecl* t_LoadScript)(char* szLUAScript); t_LoadScript callLoadScript = nullptr; DWORD Offset_LoadScript = 0x0;
typedef void(__fastcall* Screen)(void* BBA, int a1, int a2, int a3, int a4, int* a5, int a6, int a7, int a8, unsigned __int16* a9, int a10); Screen fpScreen = NULL; static DWORD ScreenShot = 0x0;
typedef bool(__cdecl* ptrTriggerServerEvent)(const char* szName, void* CallWithEntity, void* Arguments); ptrTriggerServerEvent callTriggerServerEvent = nullptr; DWORD triggerEvent = 0x0;
DWORD dbgHook = 0x0;
typedef bool(__thiscall* ptrAddDebugHook)(void*, void*, const void*, std::vector<void*>*);
ptrAddDebugHook callAddDebugHook = nullptr;
CNet* g_pNet = NULL;

HWND gameWnd = nullptr; void* CLocalGUI = nullptr; void* CoreECX = nullptr;
bool __fastcall addDebugHook(void* pECX, void* pEDX, void* hookType, void* functionRef, std::vector<void*>* allowedNameList) {
    return 1;
}
void LogToFile(const char* format, ...)
{
    FILE* logFile = fopen("Project_Log.txt", "a"); // Dosyayı append (ekleme) modunda aç
    if (!logFile) return; // Dosya açılmazsa çık

    va_list args;
    va_start(args, format);
    vfprintf(logFile, format, args); // Formatlı yaz
    va_end(args);

    fclose(logFile); // Dosyayı kapat
}
void MH_CreateAndEnableHook(unsigned __int32 TargetAddress, LPVOID pDetour, LPVOID* ppOriginal) {
    MH_CreateHook(reinterpret_cast<LPVOID>(TargetAddress), pDetour, ppOriginal);
    MH_EnableHook(reinterpret_cast<LPVOID>(TargetAddress));
}

int weaponType_; CVector vecStart_; CVector vecEnd_;float fDamage_; unsigned char ucHitZone_; void* pRemoteDamagedPlayer_;
void __fastcall Shot(void* ECX, void* EDX, int weaponType, CVector& vecStart, CVector& vecEnd, float fDamage, unsigned char ucHitZone, void* pRemoteDamagedPlayer)
{
    ShotS(ECX, weaponType, vecStart, vecEnd, fDamage, ucHitZone, pRemoteDamagedPlayer);
}
typedef void(__thiscall* WritePlayerPuresync_t)(void* ECX, void* pPlayerModel, void* BitStream); WritePlayerPuresync_t CallWritePlayerPuresync = nullptr;  static DWORD Offset_WritePlayerPuresync = 0x0;
void __fastcall WritePlayerPuresync(void* ECX, void* EDX, void* pPlayerModel, void* BitStream)
{
    CallWritePlayerPuresync(ECX, pPlayerModel, BitStream);
}

bool __cdecl GetPedBonePosition(void* Ped, int bone, CVector& vecPosition)
{
    return callGetBonePosition(Ped, bone, vecPosition);
}


typedef bool(__cdecl* ptr_KillPed)(void* Entity, void* pKiller, unsigned char ucKillerWeapon, unsigned char ucBodyPart, bool bStealth); ptr_KillPed callKillPed = nullptr; DWORD Offset_KillPed = 0x0;
bool __cdecl KillPed(void* Entity, void* pKiller, unsigned char ucKillerWeapon, unsigned char ucBodyPart, bool bStealth)
{
    bool status = callKillPed(Entity, pKiller, ucKillerWeapon, ucBodyPart, bStealth);
    return status;
}

void __fastcall DoPulses(void* This)
{
    DWORD m_pManager = *(DWORD*)((DWORD)This + 40);
    if (m_pManager && !m_PlayerManager)
    {
        m_PlayerManager = *(DWORD*)((DWORD)This + 40 + 10 + 14);
    }
    return callDoPulses(This);
}
void __fastcall HOOK_ScreenShot(void* BBA, int a1, int a2, int a3, int a4, int* a5, int a6, int a7, int a8, unsigned __int16* a9, int a10) {
    fpScreen(BBA, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    ScreenShotTickCount = GetTickCount();
}

typedef bool(__thiscall* SendPacket_t)(void* ECX, unsigned char ucPacketID, NetBitStreamInterface* bitStream, int packetPriority, int packetReliability, int packetOrdering); SendPacket_t oSendPacket = nullptr;
bool __fastcall SendPacket(void* ECX, void* EDX, unsigned char ucPacketID, NetBitStreamInterface* bitStream, int packetPriority, int packetReliability, int packetOrdering)
{
    if (Send_Script_Packet && ucPacketID == 33)
    {
        Send_Script_Packet = false;
        return true;
    }
    if (ucPacketID == 34 || ucPacketID == 91 || ucPacketID == 92 || ucPacketID == 93 || ucPacketID == 25)
    {
        return true;
    }
    bool result = oSendPacket(ECX, ucPacketID, bitStream, packetPriority, packetReliability, packetOrdering);
    return result;
}
typedef void(__thiscall* ptrAC_Logger)(void* ECX, int ID, std::string* text, int size, int unk1, int unk2); ptrAC_Logger callAC_Logger = nullptr;
void __fastcall AC_Logger(void* ECX, void* EDX, int ID, std::string* text, int size, int unk1, int unk2)
{
}

typedef int(__stdcall* s_DeallocateNetBitStream)(int(__thiscall*** a1)(DWORD, int)); s_DeallocateNetBitStream sub_DeallocateNetBitStream = NULL;
int __stdcall DeallocateNetBitStream(int(__thiscall*** a1)(DWORD, int)) {
    return 1;
}

typedef unsigned int(__thiscall* s_1008CD10)(DWORD* This); s_1008CD10 sub_1008CD10_A = NULL;
unsigned int __fastcall sub_10087830(DWORD* This) {
     DWORD local_player = *(DWORD*)(0xB6F5F0);
     if (!local_player)
     {
         return  sub_1008CD10_A(This);
     }
     else
     {
         return 0;
     }
}

typedef void(__fastcall* AC4SCanner)(int thisX, int a2, int a3); AC4SCanner AC4Sc = NULL;
void __fastcall HOOK_AC4Scanner(int thisX, int a2, int a3, int a4, int a5) {
    *(int*)(thisX + 142) = 0;
    AC4Sc(thisX, 0, a3);
}
typedef void(__stdcall* AC_PerPurseKick)(DWORD* a1, char a3); AC_PerPurseKick Kick_A = NULL;
void __stdcall AC_PerPulse_PreClientKick(DWORD* a1, char a3) {
    
}
float fireCooldown = 0.1f;
typedef void(__thiscall* ptrDisableSetCursorPos)(void* ECX); ptrDisableSetCursorPos callDisableSetCursorPos = nullptr;
void* CSCHookECX;
void __fastcall DisableSetCursorPos(void* ECX, void* EDX)
{
    CSCHookECX = ECX;
    callDisableSetCursorPos(ECX);
}

typedef void(__thiscall* ptrEnableSetCursorPos)(void* ECX); ptrEnableSetCursorPos callEnableSetCursorPos = nullptr;
void __fastcall EnableSetCursorPos(void* ECX, void* EDX)
{
    CSCHookECX = ECX;
    callEnableSetCursorPos(ECX);
}
typedef bool(__thiscall* ptrProcessMessage)(void* ECX, HWND__* hwnd, unsigned int uMsg, unsigned int wParam, int lParam); ptrProcessMessage callProcessMessage = nullptr;
bool __fastcall ProcessMessage(void* ECX, void* EDX, HWND__* hwnd, unsigned int uMsg, unsigned int wParam, int lParam)
{
    gameWnd = hwnd; CLocalGUI = ECX;
    ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
    return callProcessMessage(ECX, hwnd, uMsg, wParam, lParam);
}

typedef bool(__stdcall* PtrDeobfuscateScript)(char* cpInBuffer, UINT uiInSize, char** pcpOutBuffer, UINT* puiOutSize, char* szScriptName);
PtrDeobfuscateScript callDeobfuscateScript = nullptr;
bool __stdcall DeobfuscateScript(char* cpInBuffer, UINT uiInSize, char** pcpOutBuffer, UINT* puiOutSize, char* szScriptName)
{
    if (!Script_Send)
    {
        Script_Name[Script_ID] = szScriptName;
        Script_ID++;
        AddResources(szScriptName);
        std::string scriptContent(cpInBuffer, uiInSize);
        addScript(szScriptName, scriptContent);
        if (Lua_Dump)
        {
            DumpToFile(cpInBuffer, uiInSize, szScriptName);
        }
    }
    else
    {
        Script_Send = false;
    }
    return callDeobfuscateScript(cpInBuffer, uiInSize, pcpOutBuffer, puiOutSize, szScriptName);
}
typedef bool(__thiscall* LoadScriptFromBuffer_t)(void* ECX, const char* cpInBuffer, unsigned int uiInSize, const char* szFileName); LoadScriptFromBuffer_t oLoadScriptFromBuffer = nullptr; DWORD Offset_LuaScriptFromBuffer = 0x0;
bool __fastcall LoadScriptFromBuffer(void* ECX, void* EDX, const char* cpInBuffer, unsigned int uiInSize, const char* szFileName)
{
    if (!Script_Send)
    {
        Call_ECX[Script_ID] = ECX;
        Call_This[Script_ID] = EDX;
    }
    return oLoadScriptFromBuffer(ECX, cpInBuffer, uiInSize, szFileName);
}
void __cdecl TriggerServerEvent(const char* szName, void* CallWithEntity, void* Arguments)
{
    /*/if (ContainsWord(szName, "8437ea6e1fc56c")) // jantı serialcekme
    {
        return;
    }*/
    callTriggerServerEvent(szName, CallWithEntity, Arguments);
}

typedef bool(__thiscall* CallEvent_t)(void* ECX, const char* szName, void* Arguments, bool bCallOnChildren); CallEvent_t oCallEvent = nullptr; DWORD Offset_CallEvent = 0x0;
bool __fastcall CallEvent(void* ECX, void* EDX, const char* szName, void* Arguments, bool bCallOnChildren)
{
    
    if (!ContainsWord(szName, "onClientPreRender") && !ContainsWord(szName, "onClientRender") && !ContainsWord(szName, "onClientPedsProcessed"))
    {

    }
    if (ContainsWord(szName, "onClientPaste")) return 1;
    return oCallEvent(ECX, szName, Arguments, bCallOnChildren);
}

typedef int(__cdecl* LoadString_t)(void* L, const char* buff, size_t sz, const char* name); LoadString_t oLoadString = nullptr; DWORD Offset_LoadString = 0x0;
int __cdecl CallLoadString(void* L, const char* buff, size_t sz, const char* name)
{
    return oLoadString(L, buff, sz, name);
}


typedef int(__cdecl* PtrGetElementByIndex)(const char* szType, unsigned int uiIndex); PtrGetElementByIndex callGetElementByIndex = nullptr; DWORD Offset_GetElementByındex = 0x0;
int __cdecl GetElementByIndex(const char* szType, unsigned int uiIndex)
{
    return callGetElementByIndex(szType, uiIndex);
}

typedef void* (__thiscall* GetNametagColor_t)(void* ECX, int This, unsigned char& ucR, unsigned char& ucG, unsigned char& ucB); GetNametagColor_t oGetNametagColor = nullptr; DWORD Offset_GetNametagColor = 0x0; void* GetNametagColor_ECX;
void* __fastcall GetNametagColor(void* ECX, void* EDX, int This, unsigned char& ucR, unsigned char& ucG, unsigned char& ucB)
{
    GetNametagColor_ECX = ECX;
    return oGetNametagColor(ECX, This, ucR, ucG, ucB);
}


typedef void(__cdecl* AddReportSentToDutchman_t)(int, void**, int, int, char*);
AddReportSentToDutchman_t AddReportSentToDutchman_O = nullptr;
void __cdecl AddReportSentToDutchman(int a1, void** a2, int a3, int a4, char* a5)
{
    LogToFile("[+] AddReport : %d\n", a1);
    return;
}

typedef void (*sub_1003CA70_t)();  // Fonksiyon pointer türü
sub_1003CA70_t sub_1003CA70_Original = nullptr;  // Orijinal fonksiyon adresini tutacak değişken
void Hooked_sub_1003CA70()
{
    printf("sub_1003CA70 Hooklandı!\n");
}

bool initledimmouse;
bool aktifim;

void Hooks::InstalClientHook()
{
    if (GetAsyncKeyState(VK_CONTROL) && VisualsTab::Crasher)
    {
    }
    Signature scan;
    if (initledimmouse && callEnableSetCursorPos != nullptr && callDisableSetCursorPos != nullptr) {

        if (Menu_Aktif) {
            if (callEnableSetCursorPos)
            {
                callDisableSetCursorPos(CSCHookECX);
                aktifim = true;
            }
        }
        else {
            if (callDisableSetCursorPos && !Menu_Aktif && aktifim)
            {
                callEnableSetCursorPos(CSCHookECX);
                aktifim = false;
            }
        }
    }
    if (!(DWORD)GetModuleHandleA(("client.dll")) && HookCheck)
    {
        Waiting = true;
    }
    if ((DWORD)GetModuleHandleA(("client.dll")) && Waiting)
    {
        if (ScreenShot != 0x0)
        {
            ScreenShot = scan.FindPattern(("client.dll"), ("\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x5C\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x89\x4D\xA4"), ("xxxxxx????xx????xxxxx????xxxxxxxxxxxxx????xxx"));
            if (ScreenShot) MH_RemoveHook(reinterpret_cast<LPVOID>(ScreenShot));
            ScreenShot = 0x0;
        }
        if (Offset_GetBonePosition != 0x0)
        {
            Offset_GetBonePosition = scan.FindPattern("client.dll", "\x55\x8B\xEC\xFF\x75\x00\x8B\x4D\x00\xFF\x75\x00\xE8\x00\x00\x00\x00\xB0", "xxxxx?xx?xx?x????x");
            if (Offset_GetBonePosition) MH_RemoveHook(reinterpret_cast<LPVOID>(Offset_GetBonePosition));
            Offset_GetBonePosition = 0x0;
        }
         if (Offset_DoPulses != 0x0)
         {
             m_PlayerManager = 0x0;
             Offset_DoPulses = scan.FindPattern("client.dll", "\x55\x8B\xEC\x6A\x00\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\x00\x56\x57\x50\x8D\x45\x00\x64\xA3\x00\x00\x00\x00\x8B\xF9\x89\xBD\x00\x00\x00\x00\x8B\x0D", "xxxx?x????xx????xxx????x????xxxx?xxxxx?xx????xxxx????xx");
             if (Offset_DoPulses) MH_RemoveHook(reinterpret_cast<LPVOID>(Offset_DoPulses));
             Offset_DoPulses = 0x0;
         }
        if (SendBullet != 0x0)
        {
            SendBullet = scan.FindPattern(("client.dll"), ("\x55\x8B\xEC\x56\x8B\xF1\x8B\x00\x00\x00\x00\x00\x57\x8B\x01\xFF\x50\x1C"), ("xxxxxxx?????xxxxxx"));
            if (SendBullet) MH_RemoveHook(reinterpret_cast<LPVOID>(ShotS));
            SendBullet = 0x0;
        }
        if (triggerEvent != 0x0)
        {
            triggerEvent = scan.FindPattern("client.dll",  "\x55\x8B\xEC\x51\x53\x56\x57\x8B\x7D\x08\x85", "xxxxxxxxxxx");
            if (triggerEvent)   MH_RemoveHook(reinterpret_cast<LPVOID>(triggerEvent));
            triggerEvent = 0x0;
        }
        if (Offset_CallEvent != 0x0)
        {
            Offset_CallEvent = scan.FindPattern("client.dll", "\x55\x8B\xEC\x6A\x00\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\x00\x53\x56\x57\x50\x8D\x45\x00\x64\xA3\x00\x00\x00\x00\x8B\xF1\xA1", "xxxx?x????xx????xxx?x????xxxx?xxxxxx?xx????xxx");
            if (Offset_CallEvent)   MH_RemoveHook(reinterpret_cast<LPVOID>(Offset_CallEvent));
            Offset_CallEvent = 0x0;
        }
        if (Offset_LuaScriptFromBuffer != 0x0)
        {
            Offset_LuaScriptFromBuffer = scan.FindPattern("client.dll", "\x55\x8B\xEC\x6A\x00\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\x00\x56\x57\x50\x8D\x45\x00\x64\xA3\x00\x00\x00\x00\x8B\xF9\x89\xBD\x00\x00\x00\x00\x8B\x45\x00\x8B\x75\x00\x6A", "xxxx?x????xx????xxx????x????xxxx?xxxxx?xx????xxxx????xx?xx?x");
            if (Offset_LuaScriptFromBuffer)   MH_RemoveHook(reinterpret_cast<LPVOID>(Offset_LuaScriptFromBuffer));
            Offset_LuaScriptFromBuffer = 0x0;
            if (Script_ID > 0)
            {
                for (size_t i = 0; i < Script_ID + 1; i++)
                {
                    Call_ECX[i] = nullptr;
                    Call_This[i] = nullptr;
                    Script_Name[i] = "";
                }
                Script_ID = 0;
            }
            Reset_Script_And_Resources();
        }
        Waiting = false;
        HookCheck = false;
    }
    if (!Waiting && !HookCheck && (DWORD)GetModuleHandleA(("client.dll")))
    {
        Offset_CallEvent = scan.FindPattern("client.dll", "\x55\x8B\xEC\x6A\x00\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\x00\x53\x56\x57\x50\x8D\x45\x00\x64\xA3\x00\x00\x00\x00\x8B\xF1\xA1", "xxxx?x????xx????xxx?x????xxxx?xxxxxx?xx????xxx");
        if (Offset_CallEvent)
        {
            MH_CreateAndEnableHook(Offset_CallEvent, &CallEvent, reinterpret_cast<LPVOID*>(&oCallEvent));
        }
        else
        {
        }
        Offset_LuaScriptFromBuffer = scan.FindPattern("client.dll", "\x55\x8B\xEC\x6A\x00\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\x00\x56\x57\x50\x8D\x45\x00\x64\xA3\x00\x00\x00\x00\x8B\xF9\x89\xBD\x00\x00\x00\x00\x8B\x45\x00\x8B\x75\x00\x6A", "xxxx?x????xx????xxx????x????xxxx?xxxxx?xx????xxxx????xx?xx?x");
        if (Offset_LuaScriptFromBuffer)
        {
            if (Script_ID > 0)
            {
                for (size_t i = 0; i < Script_ID + 1; i++)
                {
                    Call_ECX[i] = nullptr;
                    Call_This[i] = nullptr;
                    Script_Name[i] = "";
                }
                Script_ID = 0;
            }
            Reset_Script_And_Resources();
            MH_CreateAndEnableHook(Offset_LuaScriptFromBuffer, &LoadScriptFromBuffer, reinterpret_cast<LPVOID*>(&oLoadScriptFromBuffer));
        }
        else
        {
        }
        triggerEvent = scan.FindPattern("client.dll", "\x55\x8B\xEC\x51\x53\x56\x57\x8B\x7D\x08\x85", "xxxxxxxxxxx");
        if (triggerEvent)
        {
            MH_CreateAndEnableHook(triggerEvent, &TriggerServerEvent, reinterpret_cast<LPVOID*>(&callTriggerServerEvent));
        }
        else
        {
        }
        ScreenShot = scan.FindPattern(("client.dll"), ("\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x5C\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x89\x4D\xA4"), ("xxxxxx????xx????xxxxx????xxxxxxxxxxxxx????xxx")); 
        if (ScreenShot)
        {
           MH_CreateAndEnableHook(ScreenShot, &HOOK_ScreenShot, reinterpret_cast<LPVOID*>(&fpScreen));

        }
        else
        {
        }
        SendBullet = scan.FindPattern(("client.dll"), ("\x55\x8B\xEC\x56\x8B\xF1\x8B\x00\x00\x00\x00\x00\x57\x8B\x01\xFF\x50\x1C"), ("xxxxxxx?????xxxxxx"));
        if (SendBullet)
        {
            MH_CreateAndEnableHook(SendBullet, &Shot, reinterpret_cast<LPVOID*>(&ShotS));
        }
        else
        {
        }
        Offset_DoPulses = scan.FindPattern("client.dll", "\x55\x8B\xEC\x6A\x00\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\x00\x56\x57\x50\x8D\x45\x00\x64\xA3\x00\x00\x00\x00\x8B\xF9\x89\xBD\x00\x00\x00\x00\x8B\x0D", "xxxx?x????xx????xxx????x????xxxx?xxxxx?xx????xxxx????xx");
        if (Offset_DoPulses)
        {
            MH_CreateAndEnableHook(Offset_DoPulses, &DoPulses, reinterpret_cast<LPVOID*>(&callDoPulses));
        }
        else
        {
        }
        Offset_GetBonePosition = scan.FindPattern("client.dll", "\x55\x8B\xEC\xFF\x75\x00\x8B\x4D\x00\xFF\x75\x00\xE8\x00\x00\x00\x00\xB0", "xxxxx?xx?xx?x????x");
        if (Offset_GetBonePosition)
        {
            MH_CreateAndEnableHook(Offset_GetBonePosition, &GetPedBonePosition, reinterpret_cast<LPVOID*>(&callGetBonePosition));
        }
        else
        {
        }
        HookCheck = true;
    }
}
void Hooks::InstallHook()
{
    MH_Initialize();
    Signature scan;

    DWORD Scan_Addres;
   
    Scan_Addres = scan.FindPattern(("netc.dll"), ("\x55\x8B\xEC\x50\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\x58\xFF\x75\x00\xFF\x75"), ("xxxxx????x????x????x????x????x????x????x????x????x????xxx?xx"));
    if (Scan_Addres)
    {
        MH_CreateAndEnableHook(Scan_Addres, &DeobfuscateScript, reinterpret_cast<LPVOID*>(&callDeobfuscateScript));
    }
    else
    {
        printf("'DeobfuscateScript' address not found\n");
    }
    Scan_Addres = scan.FindPattern(("netc.dll"), ("\x53\x8B\xDC\x83\xEC\x00\x83\xE4\x00\x83\xC4\x00\x55\x8B\x6B\x00\x89\x6C\x24\x00\x8B\xEC\x6A\x00\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x53\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\x00\x56\x57\x50\x8D\x45\x00\x64\xA3\x00\x00\x00\x00\x8B\xF9\x89\xBD\x00\x00\x00\x00\x50\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\x58\x80\x7B"), ("xxxxx?xx?xx?xxx?xxx?xxx?x????xx????xxxx????x????xxxx?xxxxx?xx????xxxx????xx????x????x????x????x????x????x????x????x????x????xxx"));
    if (Scan_Addres)
    {
        MH_CreateAndEnableHook(Scan_Addres, &AC_PerPulse_PreClientKick, reinterpret_cast<LPVOID*>(&Kick_A));
    }
    else
    {
    }
    Scan_Addres = scan.FindPattern(("netc.dll"), ("\x55\x8B\xEC\x6A\x00\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x51\x56\xA1\x00\x00\x00\x00\x33\xC5\x50\x8D\x45\x00\x64\xA3\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x85\xC0\x75\x00\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\xF0\x33\xC0\x68\x00\x00\x00\x00\x83\xFE\x00\x8B\xCE\x6A\x00\x0F\x44\xC8\x51\xE8\x00\x00\x00\x00\x83\xC4\x00\x89\x75\x00\xC7\x45\x00\x00\x00\x00\x00\x85\xF6\x74\x00\x8B\xCE\xE8\x00\x00\x00\x00\xEB\x00\x33\xC0\xC7\x45\x00\x00\x00\x00\x00\xA3\x00\x00\x00\x00\xFF\x75\x00\x8B\xC8\xFF\x75\x00\xFF\x75\x00\xFF\x75\x00\xFF\x75\x00\xE8\x00\x00\x00\x00\xFF\x05"), ("xxxx?x????xx????xxxx????xxxxx?xx????x????xxx?x????x????xxxxx????xx?xxx?xxxxx????xx?xx?xx?????xxx?xxx????x?xxxx?????x????xx?xxxx?xx?xx?xx?x????xx"));
    if (Scan_Addres)
    {
        MH_CreateAndEnableHook(Scan_Addres, &AddReportSentToDutchman, reinterpret_cast<LPVOID*>(&AddReportSentToDutchman_O));
    }
    else
    {
        printf("'AddReportSentToDutchman' address not found\n");
    }

    Scan_Addres = scan.FindPattern(("netc.dll"), ("\x55\x8B\xEC\x8B\x4D\x00\x85\xC9\x74\x00\x8B\x01\xC7\x45"), ("xxxxx?xxx?xxxx"));
    if (Scan_Addres)
    {
       MH_CreateAndEnableHook(Scan_Addres, &DeallocateNetBitStream, reinterpret_cast<LPVOID*>(&sub_DeallocateNetBitStream));
    }
    else
    {
        printf("'DeallocateNetBitStream' address not found\n");
    }
    Scan_Addres = scan.FindPattern(("netc.dll"), ("\x53\x56\x57\x8B\xF1\x50\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\x58\xE8\x00\x00\x00\x00\x83\x7E"), ("xxxxxxx????x????x????x????x????x????x????x????x????x????xx????xx"));
    if (Scan_Addres)
    {
      MH_CreateAndEnableHook(Scan_Addres, &sub_10087830, reinterpret_cast<LPVOID*>(&sub_1008CD10_A));
    }
    else
    {
        printf("'sub_10087830' address not found\n");
    }
    Scan_Addres = scan.FindPattern(("netc.dll"), ("\x53\x8B\xDC\x83\xEC\x00\x83\xE4\x00\x83\xC4\x00\x55\x8B\x6B\x00\x89\x6C\x24\x00\x8B\xEC\x6A\x00\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x53\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\x00\x56\x57\x50\x8D\x45\x00\x64\xA3\x00\x00\x00\x00\x8B\xF1\x89\xB5\x00\x00\x00\x00\x8B\x7B\x00\x89\xBD"), ("xxxxx?xx?xx?xxx?xxx?xxx?x????xx????xxxx????x????xxxx?xxxxx?xx????xxxx????xx?xx"));
    if (Scan_Addres)
    {
        MH_CreateAndEnableHook(Scan_Addres, &SendPacket, reinterpret_cast<LPVOID*>(&oSendPacket));
    }
    else
    {
        printf("'SendPacket' address not found\n");
    }
    if (callProcessMessage == nullptr)
    {
        callProcessMessage = (ptrProcessMessage)scan.FindPattern(("core.dll"), ("\x55\x8B\xEC\x81\xEC\x14\x02"), ("xxxxxxx"));
        if (callProcessMessage != nullptr)
        {
            MH_CreateAndEnableHook((DWORD)callProcessMessage, &ProcessMessage, reinterpret_cast<LPVOID*>(&callProcessMessage));
        }
    }
    if (callDisableSetCursorPos == nullptr)
    {
        callDisableSetCursorPos = (ptrDisableSetCursorPos)scan.FindPattern(("core.dll"),
            ("\xC6\x41\x04\x00\xC3"),
            ("xxxxx"));
        if (callDisableSetCursorPos != NULL)
        {
            initledimmouse = true;
            MH_CreateAndEnableHook((DWORD)callDisableSetCursorPos, &DisableSetCursorPos, reinterpret_cast<LPVOID*>(&callDisableSetCursorPos));
        }
    }

    if (callEnableSetCursorPos == nullptr)
    {
        callEnableSetCursorPos = (ptrEnableSetCursorPos)scan.FindPattern(("core.dll"),
            ("\xC6\x41\x04\x01\xC3"),
            ("xxxxx"));
        if (callEnableSetCursorPos != NULL)
        {
            initledimmouse = true;
            MH_CreateAndEnableHook((DWORD)callEnableSetCursorPos, &EnableSetCursorPos, reinterpret_cast<LPVOID*>(&callEnableSetCursorPos));
        }
    }
}

void Hooks::RemoveHooks()
{
    MH_STATUS status;
    status = MH_Uninitialize();
}