#include "pch.h"
#include "Chunk.h"

extern D3D12::Renderer gRenderer;

Chunk::Chunk(DirectX::XMFLOAT3 worldPos, DirectX::XMFLOAT3 dimension, UINT voxels)
{
	m_cbData.m_worldPos = worldPos;
	m_cbData.m_dimension = dimension;
	m_cbData.m_voxelDimension = voxels;
}


Chunk::~Chunk()
{
	m_cb.Release();
}

void Chunk::CreateConstantBuffer()
{
	size_t size = sizeof(m_cbData);
	auto cbvHeap = gRenderer.GetCBVHeap();
	ThrowIfFailed(m_cb.CreateConstantBuffer(cbvHeap, gRenderer.GetDevice(), size));
	m_cb.UpdateBuffer((void*)&m_cbData, size);
}
