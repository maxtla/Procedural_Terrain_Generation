#include "pch.h"
#include "ConstantBuffer.h"

extern D3D12::Renderer gRenderer;

int ConstantBuffer::BUFFER_COUNT = 0;

ConstantBuffer::ConstantBuffer()
{
}


ConstantBuffer::~ConstantBuffer()
{
}

HRESULT ConstantBuffer::CreateConstantBuffer(ID3D12DescriptorHeap * pCBVHeap, ID3D12Device * pDev, size_t bufferWidth)
{
	if (BUFFER_COUNT + 1 == D3D12::MAX_CONSTANT_BUFFERS)
		return E_FAIL;

	HRESULT hr;

	D3D12_HEAP_PROPERTIES uploadHeap;
	uploadHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeap.CreationNodeMask = 0;
	uploadHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeap.VisibleNodeMask = 0;

	hr = pDev->CreateCommittedResource(
		&uploadHeap,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL,
		IID_PPV_ARGS(&m_buffer)
	);

	if (SUCCEEDED(hr))
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = m_buffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = (bufferWidth + 255) & ~255;

		auto cbvHeapHandle = pCBVHeap->GetCPUDescriptorHandleForHeapStart();
		m_incrementSize = pDev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		cbvHeapHandle.ptr += (m_incrementSize * BUFFER_COUNT);

		pDev->CreateConstantBufferView(&cbvDesc, cbvHeapHandle);

		m_bufferSlot = BUFFER_COUNT;
		BUFFER_COUNT++;
	}
	return hr;
}

void ConstantBuffer::Release()
{
	SafeRelease(&m_buffer);
}

void ConstantBuffer::UpdateBuffer(void * src, size_t size)
{
	if (!m_buffer)
		return; 

	D3D12_RANGE rr;
	rr.Begin = 0;
	rr.End = size;

	void * dest = nullptr;

	m_buffer->Map(0, &rr, &dest);
	memcpy(dest, src, size);
	m_buffer->Unmap(0, &rr);
}

void ConstantBuffer::BindBuffer(UINT rootParameterIndex, ID3D12GraphicsCommandList * pCommandList, bool compute = false)
{
	if (!m_buffer)
		return;

	auto cbvHeapHandle = gRenderer.GetCBVHeap()->GetGPUDescriptorHandleForHeapStart();
	cbvHeapHandle.ptr += (m_incrementSize * m_bufferSlot);

	if (compute)
		pCommandList->SetComputeRootDescriptorTable(rootParameterIndex, cbvHeapHandle);
	else
		pCommandList->SetGraphicsRootDescriptorTable(rootParameterIndex, cbvHeapHandle);
}
