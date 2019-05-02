#include "pch.h"
#include "Chunk.h"

extern D3D12::Renderer gRenderer;

Chunk::Chunk(DirectX::XMFLOAT3 worldPos, DirectX::XMFLOAT3 dimension, UINT voxels)
{
	m_cbData.m_worldPos = worldPos;
	m_cbData.m_dimension = dimension;
	m_cbData.m_voxelDimension = voxels;
	m_cbData.m_invVoxelDim = 1.0f / voxels;
}


Chunk::~Chunk()
{
	
}

void Chunk::CreateConstantBuffer()
{
	size_t size = sizeof(CBData);
	auto cbvHeap = gRenderer.GetDescriptorHeap();
	ThrowIfFailed(m_cb.CreateConstantBuffer(cbvHeap, gRenderer.GetDevice(), size));
	m_cb.UpdateBuffer((void*)&m_cbData, size);
}

void Chunk::CreateSVB()
{
	m_svb.CreateBuffer();
}

void Chunk::Release()
{
	m_cb.Release();
	m_svb.Release();
}

void Chunk::GenerateVertices(TextureBuffer3D * pDensityTexture)
{
	auto cmdList = gRenderer.GetComputeCmdList();


	m_cb.BindBuffer(1, cmdList, true);
	m_svb.BindBuffer(2, cmdList, true);
	pDensityTexture->BindSRV(3, cmdList);


	cmdList->Dispatch(CHUNK_THREAD_GROUPS, CHUNK_THREAD_GROUPS, CHUNK_THREAD_GROUPS);
}

void Chunk::Render(ID3D12GraphicsCommandList * pCmdList)
{
	m_svb.BindAndDraw(pCmdList);
}
