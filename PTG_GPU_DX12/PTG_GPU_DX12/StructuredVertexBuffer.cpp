#include "pch.h"
#include "StructuredVertexBuffer.h"

extern D3D12::Renderer gRenderer;

StructuredVertexBuffer::StructuredVertexBuffer()
{
}


StructuredVertexBuffer::~StructuredVertexBuffer()
{
}

void StructuredVertexBuffer::CreateBuffer()
{
	if (D3D12::Renderer::UAV_COUNT + 1 == HEAP_MAX_UAV)
		return;

	HRESULT hr;

	D3D12_HEAP_PROPERTIES defaultHeap;
	defaultHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeap.CreationNodeMask = 0;
	defaultHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeap.VisibleNodeMask = 0;

	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(MAX_VERTEX_BUFFER_SIZE);
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	hr = gRenderer.GetDevice()->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		NULL,
		IID_PPV_ARGS(&m_vertexBuffer)
	);

	defaultHeap.Type = D3D12_HEAP_TYPE_CUSTOM;
	defaultHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
	defaultHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	resDesc.Width = sizeof(UINT);
	hr = gRenderer.GetDevice()->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		NULL,
		IID_PPV_ARGS(&m_counter)
	);


	if (SUCCEEDED(hr))
	{
		m_vertexBuffer->SetName(L"SVB");
		m_counter->SetName(L"SVB_Counter");

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = (MAX_VERTEX_BUFFER_SIZE/72);//(MAX_VERTEX_BUFFER_SIZE / StructureByteStride);
		uavDesc.Buffer.StructureByteStride = 72; //sizeof(float3) * 2 * 3
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		auto uavHeapHandle = gRenderer.GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
		m_incrementSize = gRenderer.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		uavHeapHandle.ptr += m_incrementSize *(HEAP_UAV_OFFSET + D3D12::Renderer::UAV_COUNT);

		gRenderer.GetDevice()->CreateUnorderedAccessView(m_vertexBuffer, m_counter, &uavDesc, uavHeapHandle);

		m_bufferIndex = D3D12::Renderer::UAV_COUNT++;
	}

}

void StructuredVertexBuffer::Release()
{
	SafeRelease(&m_vertexBuffer);
	SafeRelease(&m_counter);
}

void StructuredVertexBuffer::BindBuffer(UINT rootParameterIndex, ID3D12GraphicsCommandList * pCmdList, bool compute)
{
	if (!m_vertexBuffer)
		return;

	//reset UAV counter
	UINT* counterValue = nullptr;
	D3D12_RANGE rr = { 0,0 };
	m_counter->Map(0, &rr, reinterpret_cast<void**>(&counterValue));
	ZeroMemory(counterValue, sizeof(UINT));
	m_counter->Unmap(0, nullptr);

	auto uavHeapHandle = gRenderer.GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	uavHeapHandle.ptr += m_incrementSize * (HEAP_UAV_OFFSET + m_bufferIndex);
	
	if (compute)
		pCmdList->SetComputeRootDescriptorTable(rootParameterIndex, uavHeapHandle);
	else
		pCmdList->SetGraphicsRootDescriptorTable(rootParameterIndex, uavHeapHandle);
}

void StructuredVertexBuffer::BindAndDraw(ID3D12GraphicsCommandList * pCmdList, bool doTimestamp)
{
	UINT* counterValue = nullptr;
	D3D12_RANGE rr = { 0,sizeof(UINT) };
	m_counter->Map(0, &rr, reinterpret_cast<void**>(&counterValue));
	rr.End = 0;
	m_counter->Unmap(0, &rr);

	m_vbv.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vbv.SizeInBytes = (*counterValue) * 72;
	m_vbv.StrideInBytes = VERTEX_BYTE_STRIDE;


	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, 0);

	pCmdList->ResourceBarrier(1, &barrier);

	pCmdList->IASetVertexBuffers(0, 1, &m_vbv);

	m_triangleCount = (*counterValue) * 3;
	if (!doTimestamp)
		pCmdList->DrawInstanced(m_triangleCount, 1, 0, 0);
	else
	{
		D3D12Profiler::Begin();
		D3D12Profiler::Timestamp(0);
		pCmdList->DrawInstanced(m_triangleCount, 1, 0, 0);
		D3D12Profiler::Timestamp(1);
		D3D12Profiler::End();
	}

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0);
}
