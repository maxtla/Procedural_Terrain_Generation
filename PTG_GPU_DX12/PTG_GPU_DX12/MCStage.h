#pragma once
#include "RenderState.h"
#include "Chunk.h"

class MCStage
{
public:
	MCStage();
	~MCStage();

	void Init();
	void FillVertexBuffers(std::vector<TextureBuffer3D> &volumes);
	void PrepareForRendering(ID3D12GraphicsCommandList * pCmdList);
private:
	RenderState m_rs;
	D3D12::Fence * m_fence = NULL;

	std::vector<Chunk> m_chunks;
};

