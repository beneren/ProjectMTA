#pragma once
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "detours.lib")
#pragma comment(lib, "Urlmon.lib")

#pragma warning(disable: 4409)

#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdio>
#include <stdio.h>
#include <cassert>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <queue>
#include <algorithm>
#include <process.h>
#include <d3dx9.h> // We use #define D3DCOLOR now so you don't really need SDK just for this.
#include <d3d9.h>
#include <cstring>
#include <thread>
#include <random>
#include <Tlhelp32.h>
#include "psapi.h"

#include <intrin.h>
#include <map>
#include <direct.h>

#include <unordered_map>
#include <thread>
#include "CVector.h"
#include "CVector2D.h"
#include "..\Memory\xorstr.hpp"
#include "..\Menu\Menu.h"
#include "..\Librarys\detours.h"
#include "..\D3DHook\d3drender.h"
#include "..\D3DHook\d3dhook.h"
#include "..\Hooks\Hook.h"
#include "..\RakNet\BitStream.h"
#include "..\RakNet\RakClient.h"
#include "..\RakNet\HookedRakClient.h"

extern CD3DHook* pD3DHook;
void MainLoop();