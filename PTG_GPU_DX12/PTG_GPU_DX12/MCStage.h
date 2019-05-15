#pragma once
#include "RenderState.h"
#include "Chunk.h"

class MCStage
{
public:
	MCStage();
	~MCStage();

	void Init();
	void FillVertexBuffers(std::vector<TextureBuffer3D> &volumes, bool async, bool doTimestamp);

	void PrepareForRendering(ID3D12GraphicsCommandList * pCmdList, bool async, bool doTimestamp);
	void Update(float& dt);
private:
	RenderState m_rs;
	D3D12::Fence * m_fence = NULL;
	UINT m_index = 0U;

	std::vector<Chunk> m_chunks;

	struct CBData
	{
		unsigned int tg_x = 1U;
		unsigned int tg_y = 1U;
		unsigned int tg_z = 1U;
		unsigned int num_threads = 8U;
		float isoValue = 0.f;
	} m_cbData;
};

