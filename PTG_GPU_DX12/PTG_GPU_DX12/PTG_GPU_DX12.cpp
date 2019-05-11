// PTG_GPU_DX12.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"

#include "Scene.h"
#include "RenderState.h"

extern AppCtx gAppCtx;
extern ShaderReader gShaderReader;
//SDL
SDL_Window * window = NULL;

//ImGui
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
ImGuiWrap gImGuiWrap;

//Renderer
extern D3D12::Renderer gRenderer;
D3D12::Fence * gFence = nullptr;
std::vector<Shader> gShaderCollection;

//Application
bool Initialize();
void LoadShaders();
void Run();
void HandleEvent(SDL_Event &e);
void Shutdown();

//Scene
Scene * gScene = nullptr;

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	if (Initialize())
	{
		Run();
		Shutdown();
	}
	return 0;
}

bool Initialize()
{

	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		window = SDL_CreateWindow(gAppCtx.wnd_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, gAppCtx.width, gAppCtx.height, SDL_WINDOW_SHOWN);
		if (window)
		{
			gAppCtx.pSDLWindow = window;
			gAppCtx.hwnd = GetActiveWindow();
			if (SUCCEEDED(gRenderer.Init(gAppCtx)))
			{
				gFence = gRenderer.MakeFence(0, 1, D3D12_FENCE_FLAG_NONE);
				gImGuiWrap.Initialize_IMGUI(gAppCtx.width, gAppCtx.height, gAppCtx.hwnd, gRenderer.GetDevice());

				LoadShaders();

				gScene = new Scene();

				gScene->Initialize(gAppCtx);

				return true;
			}
			else
			{
				SDL_DestroyWindow(window);
				window = nullptr;
				return false;
			}
		}
		else
			return false;
	}
	else
	{
		SDL_Log("Unable to intialize SDL: %s", SDL_GetError());
		return false;
	}
}

void LoadShaders()
{
	gShaderCollection.push_back(gShaderReader.LoadCompiledShader("BasicVS.cso")); //0
	gShaderCollection.push_back(gShaderReader.LoadCompiledShader("BasicPS.cso")); //1

	gShaderCollection.push_back(gShaderReader.LoadCompiledShader("CubesVS.cso")); //2
	gShaderCollection.push_back(gShaderReader.LoadCompiledShader("CubesGS.cso")); //3

	gShaderCollection.push_back(gShaderReader.LoadCompiledShader("DensityVolume.cso")); //4
	gShaderCollection.push_back(gShaderReader.LoadCompiledShader("MarchingCubes.cso")); //5
}

void Run()
{
	SDL_Event e;
	Uint64 ts_current = SDL_GetPerformanceCounter();
	Uint64 ts_last = 0;
	float deltaTime = 0.f;

	while (gAppCtx.running)
	{
		ts_last = ts_current;
		ts_current = SDL_GetPerformanceCounter();
		deltaTime = (float)( (ts_current - ts_last) * 1000 / (float)SDL_GetPerformanceFrequency() );
		//Peek for messages and pass them to ImGui, don't remove them, SDL does that internally
		MSG msg;
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE) != 0)
			ImGui_ImplWin32_WndProcHandler(msg.hwnd, msg.message, msg.wParam, msg.lParam);
		//Handle SDL Events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			HandleEvent(e);
		}
		//New frame
		gImGuiWrap.Frame_IMGUI();
		{
			ImGui::Begin("Performance Metrics");
			std::stringstream ss;
			ss << "\nFrametime: " << deltaTime << " ms" << "\n\nFramerate: " << (UINT)((1/deltaTime)*1000U);
			ImGui::Text(ss.str().c_str());
			ImGui::End();
		}
		//Do work on the CPU -> Update()...
		gScene->Update(deltaTime);
		//Populate command list(s)
		gRenderer.StartFrame();
		gScene->Draw(gRenderer.GetCommandList());
		gImGuiWrap.Render_IMGUI(gRenderer.GetCommandList());
		gRenderer.EndFrame();
		//Execute comman list
		gRenderer.RenderFrame(gFence);
		gRenderer.Present();
	}

}

void HandleEvent(SDL_Event & e)
{
	
	switch (e.type)
	{
	case SDL_QUIT:
		gAppCtx.running = false;
		break;
	case SDL_KEYUP:
		if (e.key.keysym.sym == SDLK_ESCAPE)
			gAppCtx.running = false;
		break;
	case SDL_MOUSEMOTION:
		if (SDL_GetRelativeMouseMode())
		{
			gAppCtx.mx = e.motion.xrel;
			gAppCtx.my = e.motion.yrel;
		}
		else
		{
			gAppCtx.mx = e.motion.x;
			gAppCtx.my = e.motion.y;
		}
		break;
	case SDL_MOUSEWHEEL:
		if (e.wheel.y > 0)
			gAppCtx.camSettings.moveSpeed += 0.0005f;
		else if (e.wheel.y < 0)
			gAppCtx.camSettings.moveSpeed -= 0.0005f;
		break;
	default:
		break;
	}
}

void Shutdown()
{
	gRenderer.WaitForGPUCompletion(gRenderer.GetCommandQueue(), gFence);

	delete gScene;

	gImGuiWrap.Destroy_IMGUI();

	gRenderer.DestroyFence(gFence);

	gRenderer.CleanUp();

	for (auto & s : gShaderCollection)
		free(s.compiledShaderCode);

	SDL_DestroyWindow(window);
	SDL_Quit();
}
