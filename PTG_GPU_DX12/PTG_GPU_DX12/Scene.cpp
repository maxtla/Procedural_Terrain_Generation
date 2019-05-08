#include "pch.h"
#include "Scene.h"

extern AppCtx gAppCtx;
extern D3D12::Renderer gRenderer;
extern std::vector<Shader> gShaderCollection;

Scene::Scene()
{
}


Scene::~Scene()
{
	D3D12Profiler::Release();
}

void Scene::Initialize(AppCtx appCtx)
{
	appCtx.camSettings.aspectRatio = (float)appCtx.width / appCtx.height;
	m_camera.SetWorldPosition(appCtx.camSettings.x, appCtx.camSettings.y, appCtx.camSettings.z);
	m_camera.SetMoveSpeed(appCtx.camSettings.moveSpeed);
	m_camera.SetRotationalSpeed(appCtx.camSettings.rotSpeed);
	m_camera.SetProjectionMatrix(appCtx.camSettings.fov, appCtx.camSettings.aspectRatio, appCtx.camSettings.nearZ, appCtx.camSettings.farZ);
	m_camera.TransposeProjectioneMatrix();

	m_rs.AddShader(VS, gShaderCollection[0]);
	m_rs.AddShader(PS, gShaderCollection[1]);

	m_rs.CreatePipelineState(gRenderer.GetRootSignature(), m_rs.GetDefaultStateDescription());

	m_ds.Init();

	m_mcs.Init();

	D3D12Profiler::Init(2, 2);
}

void Scene::Update(float& dt)
{
	int numKeys;
	const Uint8 * keyStates = SDL_GetKeyboardState(&numKeys);

	static bool keyReleased = false;
	static bool keyPressed = false;
	if (!keyReleased && keyStates[SDL_SCANCODE_TAB])
	{
		keyPressed = true;
	}
	else if (keyPressed && !keyStates[SDL_SCANCODE_TAB])
	{
		keyPressed = false;
		keyReleased = true;
	}

	if (keyReleased)
	{
		keyReleased = false;
		if (SDL_GetRelativeMouseMode())
		{
			SDL_SetRelativeMouseMode((SDL_bool)false);
		}
		else
		{
			SDL_SetRelativeMouseMode((SDL_bool)true);
			SDL_WarpMouseInWindow(gAppCtx.pSDLWindow, (int)(gAppCtx.width*0.5), (int)(gAppCtx.height*0.5));
			gAppCtx.mx = gAppCtx.my = 0;
		}
	}

	m_camera.Update(dt);
	m_mcs.Update(dt);

	ImGui::Begin("Async Compute");
	if (!m_async)
	{
		m_async = ImGui::Button("Turn on async", { 100, 25 });
	}
	else
	{
		if (ImGui::Button("Turn off async", { 105, 25 }))
			m_async = false;
	}
	ImGui::End();

	ImGui::Begin("D3D12 Profiler");
	if (!m_profile)
	{
		m_profile = ImGui::Button("Start profiling", { 112, 25 });
	}
	else
	{
		if (ImGui::Button("Stop profiling", { 105, 25 }))
			m_profile = false;

		D3D12Profiler::MapData();
		D3D12Profiler::MapDataCompute();
		std::stringstream ss;
		ss << "Mesh generation time (ms): " << D3D12Profiler::GetDurationCompute(0, 1);
		ss << "\nMesh render time (ms): " << D3D12Profiler::GetDuration(0, 1);
		ImGui::Text(ss.str().c_str());
		D3D12Profiler::UnmapData();
		D3D12Profiler::UnmapDataCompute();
	}
	ImGui::End();
}

void Scene::Draw(ID3D12GraphicsCommandList * pCommandList)
{
	static bool runOnce = true;
	if (runOnce)
	{
		m_ds.Dispatch();
		runOnce = false;
	}

	m_mcs.FillVertexBuffers(m_ds.GetVolumes(), m_async, m_profile);

	m_camera.Draw(pCommandList);
	m_rs.Apply(pCommandList);
	m_mcs.PrepareForRendering(pCommandList, m_async, m_profile);
}
