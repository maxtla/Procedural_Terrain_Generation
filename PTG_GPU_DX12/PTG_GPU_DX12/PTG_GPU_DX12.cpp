// PTG_GPU_DX12.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "Renderer.h"

const char* WND_TITLE = "Procedural Terrain Generation with DirectX 12";
const unsigned int WND_WIDTH	= 800;
const unsigned int WND_HEIGHT	= 800;

//SDL
SDL_Window * window = NULL;

//ImGui
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
ImGuiWrap gImGuiWrap;

//Renderer
D3D12::Renderer * gRenderer = nullptr;
HWND gHwnd;
D3D12::Fence * gFence = nullptr;

//Application
bool run = true;
bool Initialize();
void Run();
void HandleEvent(SDL_Event &e);
void Shutdown();

int main()
{
	if (Initialize())
	{
		Run();
		Shutdown();
	}
	return 0;
}

bool Initialize()
{
	gRenderer = new D3D12::Renderer();

	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		window = SDL_CreateWindow(WND_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WND_WIDTH, WND_HEIGHT, SDL_WINDOW_SHOWN);
		if (window)
		{
			gHwnd = GetActiveWindow();
			if (SUCCEEDED(gRenderer->Init(gHwnd, WND_WIDTH, WND_HEIGHT)))
			{
				gFence = gRenderer->MakeFence(0, 1, D3D12_FENCE_FLAG_NONE);
				gImGuiWrap.Initialize_IMGUI(WND_WIDTH, WND_HEIGHT, gHwnd, gRenderer->GetDevice());
				return true;
			}
			else
			{
				delete gRenderer;
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

void Run()
{
	SDL_Event e;

	while (run)
	{
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

		//Do work on the CPU -> Update()...
		ImGui::Begin("Test");
		ImGui::End();

		//Prepares the backbuffer surface
		gRenderer->StartFrame();
		//Populate command list
		gImGuiWrap.Render_IMGUI(gRenderer->GetCommandList());

		//End recording, execute lists and present the final frame
		gRenderer->EndFrame();
		gRenderer->Present(gFence);
	}

}

void HandleEvent(SDL_Event & e)
{
	
	switch (e.type)
	{
	case SDL_QUIT:
		run = false;
		break;
	case SDL_KEYUP:
		if (e.key.keysym.sym == SDLK_ESCAPE)
			run = false;
		break;
	default:
		break;
	}
}

void Shutdown()
{
	gRenderer->WaitForGPUCompletion(gRenderer->GetCommandQueue(), gFence);

	gImGuiWrap.Destroy_IMGUI();

	gRenderer->DestroyFence(gFence);

	gRenderer->CleanUp();
	delete gRenderer;

	SDL_DestroyWindow(window);
	SDL_Quit();
}
