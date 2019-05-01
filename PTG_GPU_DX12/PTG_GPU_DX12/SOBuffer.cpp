#include "pch.h"
#include "SOBuffer.h"

extern D3D12::Renderer gRenderer;

SOBuffer::SOBuffer()
{
}


SOBuffer::~SOBuffer()
{
	if (m_SOBuffer)
		SafeRelease(&m_SOBuffer);
	if (m_SizeLocationBuffer)
		SafeRelease(&m_SizeLocationBuffer);
}

void SOBuffer::Init()
{
	m_bufferSize = SO_BUFFER_SIZE;

	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	gRenderer.GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize),
		D3D12_RESOURCE_STATE_STREAM_OUT,
		NULL,
		IID_PPV_ARGS(&m_SOBuffer)
	);

	if (m_SOBuffer)
	{
		m_SOBuffer->SetName(L"SO_Buffer");
		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		gRenderer.GetDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT64)),
			D3D12_RESOURCE_STATE_STREAM_OUT,
			NULL,
			IID_PPV_ARGS(&m_SizeLocationBuffer)
		);
		m_SizeLocationBuffer->SetName(L"SizeLocationBuffer");

		m_SOBV.BufferLocation = m_SOBuffer->GetGPUVirtualAddress();
		m_SOBV.SizeInBytes = m_bufferSize;
		m_SOBV.BufferFilledSizeLocation = m_SizeLocationBuffer->GetGPUVirtualAddress();
	}
}

void SOBuffer::Apply(ID3D12GraphicsCommandList * pCommandList)
{
	if (m_SOBuffer)
		pCommandList->SOSetTargets(0, 1, &m_SOBV);
}

UINT64 SOBuffer::GetNrOfWrittenVertices()
{
	UINT64 nrOfVertices;
	D3D12_RANGE rr = {0, sizeof(UINT64)};
	void * pData = nullptr;
	m_SizeLocationBuffer->Map(0, &rr, &pData);
	nrOfVertices = *(UINT64*)pData;
	rr.End = 0;
	m_SizeLocationBuffer->Unmap(0, &rr);
	return nrOfVertices;
}
