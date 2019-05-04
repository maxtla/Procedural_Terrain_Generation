#ifndef PCH_H
#define PCH_H

//SDL and Windows and commons
#define SDL_MAIN_HANDLED
#include <Windows.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <comdef.h>
#include <memory>
#include <tchar.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

//Memory leak detection
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//DirectX 12 Toolkit
#include "DirectXHelpers.h"
#include "d3dx12.h"
#include "GeometricPrimitive.h"
#include "GraphicsMemory.h"

//ImGui
#include "ImGuiWrap.h"

//Own classes that rarely change
#include "ShaderReader.h"
#include "Renderer.h"
#include "D3D12Profiler.h"

//Application
struct CameraSettings
{
	float x = 0.f;
	float y = 0.f;
	float z = -2.f;

	float fov = DirectX::XM_PIDIV4; //standard fov
	float aspectRatio = 1.0f; // W / H = 800U / 800U = 1U 
	float nearZ = 0.01f;
	float farZ = 100.f;

	float moveSpeed = 0.001f;
	float rotSpeed = 0.001f;
};

struct AppCtx
{
	std::string wnd_title = "Procedural Terrain Generation with DirectX 12";
	UINT width		= 1920U;
	UINT height		= 1080U;
	HWND hwnd;

	bool vsync		= false;
	bool running	= true;
	UINT backbufferCount = 3;

	CameraSettings camSettings;
	SDL_Window * pSDLWindow;

	int mx = 0;
	int my = 0;
};

//Some commonly used functions
template <class T> inline ULONG SafeRelease(T **ppT)
{
	ULONG refCount;
	if (*ppT)
	{
		refCount = (*ppT)->Release();
		if (refCount == 0)
			*ppT = NULL;
	}
	return refCount;
}

static inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

static void PostMessageBoxOnError(HRESULT hr, TCHAR* info, TCHAR* caption, UINT type, HWND hWnd)
{
	_com_error err(hr);
	TCHAR message[64]; //buffer to write to when concatenating the TCHAR strings
	_stprintf(message, _T("%s%s"), info, err.ErrorMessage());
	MessageBoxW(hWnd, message, caption, MB_OK | type);
}
#endif //PCH_H
