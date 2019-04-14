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

//DirectX 12 Toolkit
#include "DirectXHelpers.h"
#include "d3dx12.h"

//ImGui
#include "ImGuiWrap.h"

//Application
struct CameraSettings
{
	float x = 0.f;
	float y = 1.f;
	float z = -1.f;

	float fov = DirectX::XM_PIDIV4; //standard fov
	float aspectRatio = 1.0f; // W / H = 800U / 800U = 1U 
	float nearZ = 1.0f;
	float farZ = 100.f;

	float moveSpeed = 15.f;
	float rotSpeed = 0.001f;
};

struct AppCtx
{
	std::string wnd_title = "Procedural Terrain Generation with DirectX 12";
	UINT width		= 800U;
	UINT height		= 800U;
	HWND hwnd;

	bool vsync		= false;
	bool running	= true;

	CameraSettings camSettings;
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
