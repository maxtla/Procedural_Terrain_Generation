// PTG_GPU_DX12.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include "Renderer.h"
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
D3D12::Renderer * gRenderer = nullptr;
D3D12::Fence * gFence = nullptr;
std::vector<Shader> gShaderCollection;
RenderState gRenderState;

//Application
bool Initialize();
void LoadShaders();
void Run();
void HandleEvent(SDL_Event &e);
void Shutdown();

//Scene
Scene gScene;

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
	gRenderer = new D3D12::Renderer();

	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		window = SDL_CreateWindow(gAppCtx.wnd_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, gAppCtx.width, gAppCtx.height, SDL_WINDOW_SHOWN);
		if (window)
		{
			gAppCtx.hwnd = GetActiveWindow();
			if (SUCCEEDED(gRenderer->Init(gAppCtx)))
			{
				gFence = gRenderer->MakeFence(0, 1, D3D12_FENCE_FLAG_NONE);
				gImGuiWrap.Initialize_IMGUI(gAppCtx.width, gAppCtx.height, gAppCtx.hwnd, gRenderer->GetDevice());

				LoadShaders();

				gScene.Initialize(gAppCtx);

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

void LoadShaders()
{
	gShaderCollection.push_back(gShaderReader.LoadCompiledShader("BasicVS.cso"));
	gShaderCollection.push_back(gShaderReader.LoadCompiledShader("BasicPS.cso"));

	gRenderState.AddShader(VS, gShaderCollection[0]);
	gRenderState.AddShader(PS, gShaderCollection[1]);

	gRenderState.CreatePipelineState(gRenderer->GetRootSignature(), gRenderState.GetDefaultStateDescription());
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
		
		//Do work on the CPU -> Update()...
		gScene.Update(deltaTime);

		ImGui::Begin("Test");
		ImGui::End();

		//Prepares the backbuffer surface
		gRenderer->StartFrame();

		//Populate command list
		gScene.Draw(gRenderer->GetCommandList());
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
		gAppCtx.running = false;
		break;
	case SDL_KEYUP:
		if (e.key.keysym.sym == SDLK_ESCAPE)
			gAppCtx.running = false;
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

	for (auto & s : gShaderCollection)
		free(s.compiledShaderCode);

	SDL_DestroyWindow(window);
	SDL_Quit();
}
