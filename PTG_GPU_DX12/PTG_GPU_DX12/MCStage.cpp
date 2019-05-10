#include "pch.h"
#include "MCStage.h"

extern std::vector<Shader> gShaderCollection;
extern D3D12::Renderer gRenderer;

extern UINT CHUNK_THREAD_GROUPS_X;
extern UINT CHUNK_THREAD_GROUPS_Y;
extern UINT CHUNK_THREAD_GROUPS_Z;

MCStage::MCStage()
{
}


MCStage::~MCStage()
{
	gRenderer.WaitForGPUCompletion(gRenderer.GetComputeCmdQueue(), m_fence);

	for (auto &c : m_chunks)
		c.Release();

	gRenderer.DestroyFence(m_fence);

}

void MCStage::Init()
{
	m_rs.AddShader(CS, gShaderCollection[5]);
	m_rs.CreatePipelineState(gRenderer.GetRootSignature(), m_rs.GetDefaultStateDescription(), true);

	m_chunks.push_back(Chunk({ 0.f, 0.f, 0.f }, {1.f, 1.f, 1.f}, 24)); //Pos, Dimension (is scaling of the chunk), Cells
	m_chunks.push_back(Chunk({ 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f }, 24));

	m_fence = gRenderer.MakeFence(0, 1, D3D12_FENCE_FLAG_NONE, L"MC_Fence");

	for (auto &c : m_chunks)
	{
		c.CreateConstantBuffer();
		c.CreateSVB();
	}
}

void MCStage::FillVertexBuffers(std::vector<TextureBuffer3D> &volumes, bool async, bool doTimestamp)
{
	static bool wasTurnedOff = false;
	static bool previousState = false;


	auto cmdQ = gRenderer.GetComputeCmdQueue();
	auto cmdAllo = gRenderer.GetComputeAllocator();
	auto cmdList = gRenderer.GetComputeCmdList();
	auto rs = gRenderer.GetRootSignature();
	auto heap = gRenderer.GetDescriptorHeap();

	if (async || (previousState && !async))
	{
		gRenderer.WaitForGPUCompletion(cmdQ, m_fence); //Previous resource were use for rendering last frame, this frame the current resources need to finish
	}

	previousState = async;

	cmdAllo->Reset();
	cmdList->Reset(cmdAllo, NULL);

	UINT values[4] = { CHUNK_THREAD_GROUPS_X, CHUNK_THREAD_GROUPS_Y, CHUNK_THREAD_GROUPS_Z, NUM_THREADS_PER_GROUP };
	//Prepare pipeline
	cmdList->SetComputeRootSignature(rs);
	cmdList->SetDescriptorHeaps(1, (ID3D12DescriptorHeap*const*)&heap);
	cmdList->SetComputeRoot32BitConstants(4, 4, (const void*)values, 0);

	m_rs.Apply(cmdList);

	if (async)
	{
		m_index = (m_index + 1) % 2;
		m_chunks[m_index].GenerateVertices(&volumes[0], doTimestamp);
	}
	else
	{
		m_chunks[m_index].GenerateVertices(&volumes[0], doTimestamp);
		m_index = (m_index + 1) % 2;
	}

	cmdList->Close();

	cmdQ->ExecuteCommandLists(1, (ID3D12CommandList*const*)&cmdList);
	if (!async)
		gRenderer.WaitForGPUCompletion(cmdQ, m_fence); //Resources need to finish before rendering
}

void MCStage::PrepareForRendering(ID3D12GraphicsCommandList * pCmdList, bool async, bool doTimestamp)
{
	m_chunks[(m_index + 1) % 2].Render(pCmdList, doTimestamp);
}

void MCStage::Update(float & dt)
{
	ImGui::Begin("Chunk Data");
	ImGui::Text(m_chunks[m_index].GetChunkInfoStr().c_str());
	ImGui::Text("\nChunk ThreadGroups");
	ImGui::SliderInt(" X", (int*)&CHUNK_THREAD_GROUPS_X, 1, DENSITY_THREAD_GROUPS_X);
	ImGui::SliderInt(" Y", (int*)&CHUNK_THREAD_GROUPS_Y, 1, DENSITY_THREAD_GROUPS_Y);
	ImGui::SliderInt(" Z", (int*)&CHUNK_THREAD_GROUPS_Z, 1, DENSITY_THREAD_GROUPS_Z);
	ImGui::End();
}
