#include "pch.h"
#include "StructuredVertexBuffer.h"

ID3D12DescriptorHeap* StructuredVertexBuffer::m_heap = nullptr;
UINT StructuredVertexBuffer::BUFFER_COUNT = 0;
extern D3D12::Renderer gRenderer;

void StructuredVertexBuffer::InitHeap()
{
	if (m_heap == nullptr)
	{
		D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
		dhd.NumDescriptors = MAX_BUFFERS;
		dhd.NodeMask = 0;
		dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		gRenderer.GetDevice()->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_heap));
	}
}

void StructuredVertexBuffer::FreeHeap()
{
	if (m_heap)
	{
		SafeRelease(&m_heap);
		m_heap = nullptr;
	}
}

StructuredVertexBuffer::StructuredVertexBuffer()
{
}


StructuredVertexBuffer::~StructuredVertexBuffer()
{
}

void StructuredVertexBuffer::CreateBuffer()
{
	if (BUFFER_COUNT + 1 == MAX_BUFFERS)
		return;

	HRESULT hr;

	D3D12_HEAP_PROPERTIES defaultHeap;
	defaultHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeap.CreationNodeMask = 0;
	defaultHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeap.VisibleNodeMask = 0;

	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(D3D12::MAX_VERTEX_BUFFER_SIZE * 8);
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
		uavDesc.Buffer.NumElements = (D3D12::MAX_VERTEX_BUFFER_SIZE/72)*8;//(D3D12::MAX_VERTEX_BUFFER_SIZE / StructureByteStride);
		uavDesc.Buffer.StructureByteStride = 72; //sizeof(float3) * 2 * 3
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		auto uavHeapHandle = m_heap->GetCPUDescriptorHandleForHeapStart();
		m_incrementSize = gRenderer.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		uavHeapHandle.ptr += (m_incrementSize * BUFFER_COUNT);

		gRenderer.GetDevice()->CreateUnorderedAccessView(m_vertexBuffer, m_counter, &uavDesc, uavHeapHandle);

		m_bufferIndex = BUFFER_COUNT;
		BUFFER_COUNT++;
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
	std::cout << *counterValue << std::endl;
	ZeroMemory(counterValue, sizeof(UINT));
	m_counter->Unmap(0, nullptr);

	auto uavHeapHandle = m_heap->GetGPUDescriptorHandleForHeapStart();
	uavHeapHandle.ptr += (m_incrementSize * m_bufferIndex);
	
	if (compute)
		pCmdList->SetComputeRootDescriptorTable(rootParameterIndex, uavHeapHandle);
	else
		pCmdList->SetGraphicsRootDescriptorTable(rootParameterIndex, uavHeapHandle);
}
