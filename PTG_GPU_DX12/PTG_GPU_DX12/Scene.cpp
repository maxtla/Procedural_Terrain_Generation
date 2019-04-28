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
	m_gs.Release();
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

	m_gs.Init();

	m_texBuff3D.Create3DTextureBuffer(TextureBuffer3D::Texture3DDesc());
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
			SDL_SetRelativeMouseMode((SDL_bool)false);
		else
		{
			SDL_SetRelativeMouseMode((SDL_bool)true);
			SDL_WarpMouseInWindow(gAppCtx.pSDLWindow, (int)gAppCtx.width*0.5, (int)gAppCtx.height*0.5);
			gAppCtx.mx = gAppCtx.my = 0;
		}
	}

	m_camera.Update(dt);
}

void Scene::Draw(ID3D12GraphicsCommandList * pCommandList)
{
	m_gs.DrawAndExecute();

	gRenderer.StartFrame();

	m_camera.Draw(pCommandList);
	m_rs.Apply(pCommandList);
	m_gs.PrepareForRendering(pCommandList);
}
