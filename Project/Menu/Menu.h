#pragma once
#include "..\ImGuý\imgui.h"
#include "..\ImGuý\imgui_impl_dx9.h"
#include "..\ImGuý\imgui_impl_win32.h"
#include <d3d9.h>
#include <d3dx9tex.h>
#include <string>
#include <vector>
#include <regex>
#include "..\ImGuý\TextEditor.h"
#include <iostream>
#include <direct.h>
#include <random>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>

namespace LuaTab {
    extern bool customFileName;
    extern char fileName[256];
    extern char LuaName[256];
    extern bool autoInject;
    extern int autoInjectMode;
    extern bool debugHookSpoofer;
    extern bool blockOnClientPasteEvent;
    extern char filterResourceName[256];
    extern TextEditor editor;
}
namespace VisualsTab {
    extern bool enableESP;
    extern bool boxESP;
    extern bool Crasher;
    extern bool Airbreak;
    extern int distance;
    extern int fillBoxAlpha;
    extern int boxSize;
    extern int cornerSize;
}


namespace AimbotsTab {
    extern bool Aimbot;
    extern bool SilentAim;
    extern bool RotationSync;
    extern bool CrossHairAimbot;
    extern int Fov;
}

static bool Loaded_Menu;
extern void* Call_ECX[50000];
extern void* Call_This[50000];
extern std::string Script_Name[50000];
void on_present(IDirect3DDevice9* device_ptr, const RECT*, const RECT*, HWND wnd, const RGNDATA*);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern bool Menu_Aktif;
extern bool Script_Send;
extern bool Lua_Dump;
void Reset_Script_And_Resources();
void Update_Menu();
void AddResources(const char* ScriptName);
void AddLoadScriptToScriptList(const std::string& loadScriptName);
void AddDeobfuscateScriptToScriptList(const std::string& deobfuscateScriptName);
bool ContainsWord(const char* szName, const std::string& word);
void DumpToFile(const char* cpInBuffer, UINT uiInSize, const char* szScriptName);
void addScript(const std::string& title, const std::string& content);