#include "pch.h"
#include "Scene.h"


Scene::Scene()
{
}


Scene::~Scene()
{
}

void Scene::Initialize(AppCtx appCtx)
{
	m_camera.SetWorldPosition(appCtx.camSettings.x, appCtx.camSettings.y, appCtx.camSettings.z);
	m_camera.SetMoveSpeed(appCtx.camSettings.moveSpeed);
	m_camera.SetRotationalSpeed(appCtx.camSettings.rotSpeed);
	m_camera.SetProjectionMatrix(appCtx.camSettings.fov, appCtx.camSettings.aspectRatio, appCtx.camSettings.nearZ, appCtx.camSettings.farZ);
}

void Scene::Update(float& dt)
{
	int numKeys;
	const Uint8 * keyStates = SDL_GetKeyboardState(&numKeys);

	if (keyStates[SDL_SCANCODE_TAB])
	{
		if (SDL_GetRelativeMouseMode())
			SDL_SetRelativeMouseMode((SDL_bool)false);
		else
			SDL_SetRelativeMouseMode((SDL_bool)true);
	}

	m_camera.Update(dt);
}

void Scene::Draw(ID3D12GraphicsCommandList * pCommandList)
{
	m_camera.Draw(pCommandList);
}
