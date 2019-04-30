#pragma once
#include "TextureBuffer3D.h"
#include "RenderState.h"

class DensityStage
{
public:
	DensityStage();
	~DensityStage();

	void Init();
	void Dispatch();

	std::vector<TextureBuffer3D>& GetVolumes() { return m_volumes; }
private:
	std::vector<TextureBuffer3D> m_volumes;
	RenderState m_rs;
	D3D12::Fence * m_fence = NULL;
};

