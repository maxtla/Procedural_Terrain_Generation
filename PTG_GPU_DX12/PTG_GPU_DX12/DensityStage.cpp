#include "pch.h"
#include "DensityStage.h"

extern std::vector<Shader> gShaderCollection;
extern D3D12::Renderer gRenderer;

DensityStage::DensityStage()
{
}


DensityStage::~DensityStage()
{
	gRenderer.WaitForGPUCompletion(gRenderer.GetComputeCmdQueue(), m_fence);

	for (auto &b : m_volumes)
		b.Release();

	gRenderer.DestroyFence(m_fence);
}

void DensityStage::Init()
{
	m_rs.AddShader(CS, gShaderCollection[4]);
	m_rs.CreatePipelineState(gRenderer.GetRootSignature(), m_rs.GetDefaultStateDescription(), true);

	TextureBuffer3D tex3DVolume;
	tex3DVolume.Create3DTextureBuffer(TextureBuffer3D::Texture3DDesc());

	m_volumes.push_back(tex3DVolume);

	m_fence = gRenderer.MakeFence(0, 1, D3D12_FENCE_FLAG_NONE, L"DensityFence");
}

void DensityStage::Dispatch()
{
	auto cmdQ = gRenderer.GetComputeCmdQueue();
	auto cmdAllo = gRenderer.GetComputeAllocator();
	auto cmdList = gRenderer.GetComputeCmdList();
	auto rs = gRenderer.GetRootSignature();
	auto uavHeap = gRenderer.GetUAVHeap();

	cmdAllo->Reset();
	cmdList->Reset(cmdAllo, NULL);
	
	//Prepare pipeline
	cmdList->SetComputeRootSignature(rs);
	m_rs.Apply(cmdList);
	cmdList->SetDescriptorHeaps(1, (ID3D12DescriptorHeap*const*)&uavHeap);
	m_volumes[0].BindUAV(2, cmdList);

	cmdList->Dispatch(9, 9, 9);

	cmdList->Close();
	
	cmdQ->ExecuteCommandLists(1, (ID3D12CommandList* const*)&cmdList);

	gRenderer.WaitForGPUCompletion(cmdQ, m_fence);
}
