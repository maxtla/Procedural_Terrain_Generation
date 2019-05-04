#include "pch.h"
#include "D3D12Profiler.h"

extern D3D12::Renderer gRenderer;

bool D3D12Profiler::m_initialized = false;

bool D3D12Profiler::m_hasBegun = false;
bool D3D12Profiler::m_hasEnded = true;
bool D3D12Profiler::m_hasAquiredData = false;

bool D3D12Profiler::m_hasBegunCompute = false;
bool D3D12Profiler::m_hasEndedCompute = true;
bool D3D12Profiler::m_hasAquiredDataCompute= false;

ID3D12QueryHeap * D3D12Profiler::m_queryHeap = nullptr;
ID3D12Resource * D3D12Profiler::m_queryBuffer = nullptr;

UINT64 D3D12Profiler::m_gpuFrequency = 0U;
UINT64 * D3D12Profiler::m_queryData = nullptr;
UINT D3D12Profiler::m_count = 0U;

ID3D12QueryHeap * D3D12Profiler::m_queryHeapCompute = nullptr;
ID3D12Resource * D3D12Profiler::m_queryBufferCompute = nullptr;

UINT64 D3D12Profiler::m_gpuFrequencyCompute = 0U;
UINT64 * D3D12Profiler::m_queryDataCompute = nullptr;
UINT D3D12Profiler::m_countCompute = 0U;

D3D12Profiler::D3D12Profiler()
{
}


D3D12Profiler::~D3D12Profiler()
{
}

bool D3D12Profiler::Init(UINT count, UINT countCompute)
{
	if (m_initialized || m_queryHeap || m_queryBuffer || m_queryHeapCompute || m_queryBufferCompute)
		return false;

	D3D12_QUERY_HEAP_DESC desc{};
	desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	desc.Count = count;
	desc.NodeMask = 0;

	gRenderer.GetDevice()->CreateQueryHeap(&desc, IID_PPV_ARGS(&m_queryHeap));

	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	resDesc.Width = sizeof(UINT64) * count;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES heapProp{};
	heapProp.Type = D3D12_HEAP_TYPE_READBACK;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProp.CreationNodeMask = 0;
	heapProp.VisibleNodeMask = 0;

	gRenderer.GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		NULL,
		IID_PPV_ARGS(&m_queryBuffer)
		);

	desc.Count = countCompute;

	gRenderer.GetDevice()->CreateQueryHeap(&desc, IID_PPV_ARGS(&m_queryHeapCompute));
	
	resDesc.Width = sizeof(UINT64) * countCompute;

	gRenderer.GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		NULL,
		IID_PPV_ARGS(&m_queryBufferCompute)
	);

	if (m_queryHeap && m_queryBuffer && m_queryHeapCompute && m_queryBufferCompute)
	{
		m_queryHeap->SetName(L"QueryHeap");
		m_queryBuffer->SetName(L"QueryHeapBuffer");

		m_queryHeapCompute->SetName(L"QueryHeapCompute");
		m_queryBufferCompute->SetName(L"QueryHeapBufferCompute");

		m_initialized = true;
	}
	return m_initialized;
}

void D3D12Profiler::Release()
{
	SafeRelease(&m_queryHeap);
	SafeRelease(&m_queryBuffer);
	SafeRelease(&m_queryHeapCompute);
	SafeRelease(&m_queryBufferCompute);
	m_initialized = m_hasBegun = m_hasEnded = m_hasAquiredData = false;
}

void D3D12Profiler::Begin()
{
	if (m_initialized && !m_hasBegun && m_hasEnded)
	{
		m_hasBegun = true;
		m_hasEnded = false;
		gRenderer.GetCommandQueue()->GetTimestampFrequency(&m_gpuFrequency);
	}
}

void D3D12Profiler::End()
{
	if (m_initialized && m_hasBegun && !m_hasEnded)
	{
		m_hasBegun = false;
		m_hasEnded = true;
		gRenderer.GetCommandList()->ResolveQueryData(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, m_count, m_queryBuffer, 0);
		gRenderer.GetCommandQueue()->GetTimestampFrequency(&m_gpuFrequency);
	}
}

void D3D12Profiler::BeginCompute()
{
	if (m_initialized && !m_hasBegunCompute && m_hasEndedCompute)
	{
		m_hasBegunCompute = true;
		m_hasEndedCompute = false;
		gRenderer.GetCommandQueue()->GetTimestampFrequency(&m_gpuFrequency);
	}
}

void D3D12Profiler::EndCompute()
{
	if (m_initialized && m_hasBegunCompute && !m_hasEndedCompute)
	{
		m_hasBegunCompute = false;
		m_hasEndedCompute = true;
		gRenderer.GetComputeCmdList()->ResolveQueryData(m_queryHeapCompute, D3D12_QUERY_TYPE_TIMESTAMP, 0, m_countCompute, m_queryBufferCompute, 0);
		gRenderer.GetComputeCmdQueue()->GetTimestampFrequency(&m_gpuFrequencyCompute);
	}
}

void D3D12Profiler::Timestamp(UINT index)
{
	gRenderer.GetCommandList()->EndQuery(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, index);
}

void D3D12Profiler::MapData()
{
	D3D12_RANGE rr{};
	rr.Begin = 0;
	rr.End = sizeof(UINT64) * m_count;
	m_queryBuffer->Map(0, &rr, reinterpret_cast<void**>(&m_queryData));
	if (m_queryData)
		m_hasAquiredData = true;
}

void D3D12Profiler::UnmapData()
{
	if (m_hasAquiredData)
	{
		m_queryBuffer->Unmap(0, NULL);
		m_hasAquiredData = false;
	}
}

double D3D12Profiler::GetDuration(UINT beginTS, UINT endTS)
{
	if (m_hasAquiredData)
	{
		UINT64 begin, end;
		begin = m_queryData[beginTS];
		end = m_queryData[endTS];

		double duration = (double)(((float)(end - begin)) / float(m_gpuFrequency) * 1000.f);
		return duration;
	}
	return 0.0;
}

void D3D12Profiler::TimestampCompute(UINT index)
{
	gRenderer.GetComputeCmdList()->EndQuery(m_queryHeapCompute, D3D12_QUERY_TYPE_TIMESTAMP, index);
}

void D3D12Profiler::MapDataCompute()
{
	D3D12_RANGE rr{};
	rr.Begin = 0;
	rr.End = sizeof(UINT64) * m_countCompute;
	m_queryBufferCompute->Map(0, &rr, reinterpret_cast<void**>(&m_queryDataCompute));
	if (m_queryDataCompute)
		m_hasAquiredDataCompute = true;
}

void D3D12Profiler::UnmapDataCompute()
{
	if (m_hasAquiredDataCompute)
	{
		m_queryBufferCompute->Unmap(0, NULL);
		m_hasAquiredDataCompute = false;
	}
}

double D3D12Profiler::GetDurationCompute(UINT beginTS, UINT endTS)
{
	if (m_hasAquiredDataCompute)
	{
		UINT64 begin, end;
		begin = m_queryData[beginTS];
		end = m_queryData[endTS];

		double duration = (double)(((float)(end - begin)) / float(m_gpuFrequencyCompute) * 1000.f);
		return duration;
	}
	return -1.0;
}
