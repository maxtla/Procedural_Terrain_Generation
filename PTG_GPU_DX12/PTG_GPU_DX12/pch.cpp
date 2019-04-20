// pch.cpp: source file corresponding to pre-compiled header; necessary for compilation to succeed

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include "pch.h"

AppCtx gAppCtx;
ShaderReader gShaderReader;
D3D12::Renderer gRenderer;
// In general, ignore this file, but keep it around if you are using pre-compiled headers.
