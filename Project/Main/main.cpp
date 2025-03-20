#include "main.h"
#include "plugin.h"
#include "..\Visuals\ESP.h"
#include "..\Aimbot\SilentAim.h"
static bool Inıt_Hooks = false;
using namespace plugin;
CD3DHook* pD3DHook;
static SilentAim* pSilentAim = static_cast<SilentAim*>(nullptr);

void silinecekCrashes() {
    LPCWSTR registryPath = xorstr_(L"SOFTWARE\\WOW6432Node\\Multi Theft Auto: San Andreas All\\1.6\\Settings\\diagnostics");
    LPCWSTR valueNames[] = { (L"crash-data"), (L"crash-data1"), (L"crash-data2") }; // Düzeltme: Dizi olarak tanımlandı
    HKEY hKey;

    LONG openResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, registryPath, 0, KEY_SET_VALUE, &hKey);

    if (openResult != ERROR_SUCCESS) {
        // Handle the error, e.g., log the error code or display a message box
        //std::wstring errorMessage = L"Failed to open registry key: " + std::to_wstring(openResult);
        //MessageBoxW(nullptr, errorMessage.c_str(), L"Error", MB_ICONERROR);
        return;
    }

    for (const auto& valueName : valueNames) {
        LONG deleteResult = RegDeleteValueW(hKey, valueName);

        if (deleteResult != ERROR_SUCCESS) {
            // Handle the error, e.g., log the error code and value name
           // std::wstring errorMessage = L"Failed to delete value '" + std::wstring(valueName) + L"': " + std::to_wstring(deleteResult);
            //MessageBoxW(nullptr, errorMessage.c_str(), L"Error", MB_ICONERROR);
        }
        else {
            // Optionally log successful deletion
        }
    }

    RegCloseKey(hKey);
}


void Hook_Client()
{
    unsigned long VAR_SystemState;
    while (true)
    {
        VAR_SystemState = *(unsigned long*)0xC8D4C0;
        switch (VAR_SystemState)
        {
        case 5:

        case 7:
            //return;
        case 8:
            Hooks::InstalClientHook();
        case 9:
            if(!pSilentAim) pSilentAim = new SilentAim();
            Hooks::InstalClientHook();
        }
    }
}

void Hook_Start()
{
    unsigned long VAR_SystemState;
    while (true)
    {
        VAR_SystemState = *(unsigned long*)0xC8D4C0;
        switch (VAR_SystemState)
        {
        case 5:
            if (!Inıt_Hooks)
            {
                Inıt_Hooks = true;
                Hooks::InstallHook();
                break;
            }
        case 7:

            if (!Inıt_Hooks)
            {
                Inıt_Hooks = true;
                Hooks::InstallHook();
                break;
            }
            return;
        case 8:
            if (!Inıt_Hooks)
            {
                Inıt_Hooks = true;
                Hooks::InstallHook();
                break;
            }
        case 9:
            if (!Inıt_Hooks)
            {
                Inıt_Hooks = true;
                Hooks::InstallHook();
                break;
            }
        }
    }
}

int Setup_Cheat() {
	silinecekCrashes();
	std::thread Thread_Hook(Hook_Start);
	Thread_Hook.detach();
    std::thread Thread_Hook_Client(Hook_Client);
    Thread_Hook_Client.detach();
	pD3DHook = new CD3DHook();
	return 0;
}

void MainLoop()
{
	if (pD3DHook->pRender)
	{
        silinecekCrashes();
        if (ScreenShotTickCount == 0) {
            ScreenShotTickCount = GetTickCount64();
        }
        if (ScreenShotTickCount != 0 && (GetTickCount64() - ScreenShotTickCount) >= 5000) {
            if (GetAsyncKeyState(VK_INSERT))
            {
                static DWORD timer = 0;
                if (GetTickCount64() - 500 > timer)
                {
                    static bool valid;
                    valid = 0;
                    if (Menu_Aktif)
                    {
                        Menu_Aktif = 0; valid = 1;
                    }
                    if (!Menu_Aktif && valid == 0)
                    {
                        Menu_Aktif = 1;
                        printf("%d Menu aktif\n", Menu_Aktif);
                    }
                    timer = GetTickCount64();
                }
            }
            Player_ESP();
            if (Menu_Aktif) Update_Menu();
        }
	}
}



BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,
	DWORD fdwReason,
	LPVOID lpReserved)
{


	DisableThreadLibraryCalls((HMODULE)hinstDLL);
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH: {
	
        AllocConsole();
        FILE* file;
        freopen_s(&file, xorstr_("CONOUT$"), xorstr_("w"), stdout);  // std::cout
        freopen_s(&file, xorstr_("CONOUT$"), xorstr_("w"), stderr); // std::cerr
        freopen_s(&file, xorstr_("CONIN$"), xorstr_("r"), stdin);   // std::cin
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&Setup_Cheat, NULL, NULL, NULL);
		break;
	}
	}
	return TRUE;
}