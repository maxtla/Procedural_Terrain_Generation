// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#ifndef PCH_H
#define PCH_H

//SDL and Windows
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
