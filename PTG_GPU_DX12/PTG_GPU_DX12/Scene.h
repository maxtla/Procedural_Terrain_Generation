#pragma once
#include "Camera.h"
#include "DensityStage.h"
#include "MCStage.h"
//A scene contains the world, configurations for rendering, 
// and manages the Camera, other input and per frame updates

class Scene : public Drawable
{
public:
	Scene();
	~Scene();

	void Initialize(AppCtx appCtx);

	void Update(float& dt);
	void Draw(ID3D12GraphicsCommandList * pCommandList);
private:
	Camera m_camera;
	RenderState m_rs;
	DensityStage m_ds;
	MCStage m_mcs;
	bool m_async = false;
	bool m_profile = false;
};

