#pragma once
#include "RenderState.h"
#include "Chunk.h"

class MCStage
{
public:
	MCStage();
	~MCStage();

	void Init();
	void FillVertexBuffers(std::vector<TextureBuffer3D> &volumes, bool async);

	void PrepareForRendering(ID3D12GraphicsCommandList * pCmdList, bool async);
	void Update(float& dt);
private:
	RenderState m_rs;
	D3D12::Fence * m_fence = NULL;
	UINT m_index = 0U;

	std::vector<Chunk> m_chunks;
};

