#include "pch.h"
#include "MCStage.h"

extern std::vector<Shader> gShaderCollection;
extern D3D12::Renderer gRenderer;

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

	m_chunks.push_back(Chunk({ 0.f, -1.f, 0.f }));
	m_chunks.push_back(Chunk({ 1.0f, -1.0f, 0.0f }));

	m_fence = gRenderer.MakeFence(0, 1, D3D12_FENCE_FLAG_NONE, L"MC_Fence");

	for (auto &c : m_chunks)
	{
		c.CreateConstantBuffer();
		c.CreateSVB();
	}
}

void MCStage::FillVertexBuffers(std::vector<TextureBuffer3D> &volumes)
{
	auto cmdQ = gRenderer.GetComputeCmdQueue();
	auto cmdAllo = gRenderer.GetComputeAllocator();
	auto cmdList = gRenderer.GetComputeCmdList();
	auto rs = gRenderer.GetRootSignature();
	auto heap = gRenderer.GetDescriptorHeap();

	cmdAllo->Reset();
	cmdList->Reset(cmdAllo, NULL);

	//Prepare pipeline
	cmdList->SetComputeRootSignature(rs);
	cmdList->SetDescriptorHeaps(1, (ID3D12DescriptorHeap*const*)&heap);

	m_rs.Apply(cmdList);

	for (int i = 0; i < m_chunks.size(); i++)
		m_chunks[i].GenerateVertices(&volumes[i]);

	cmdList->Close();


	cmdQ->ExecuteCommandLists(1, (ID3D12CommandList*const*)&cmdList);

	gRenderer.WaitForGPUCompletion(cmdQ, m_fence);
}

void MCStage::PrepareForRendering(ID3D12GraphicsCommandList * pCmdList)
{
	for (auto &c : m_chunks)
		c.Render(pCmdList);
}
