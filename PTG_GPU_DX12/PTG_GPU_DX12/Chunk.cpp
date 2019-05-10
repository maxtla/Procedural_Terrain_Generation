#include "pch.h"
#include "Chunk.h"

extern D3D12::Renderer gRenderer;

extern UINT CHUNK_THREAD_GROUPS_X;
extern UINT CHUNK_THREAD_GROUPS_Y;
extern UINT CHUNK_THREAD_GROUPS_Z;

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


void Chunk::GenerateVertices(TextureBuffer3D * pDensityTexture, bool doTimestamp)
{
	auto cmdList = gRenderer.GetComputeCmdList();


	m_cb.BindBuffer(1, cmdList, true);
	m_svb.BindBuffer(2, cmdList, true);
	pDensityTexture->BindSRV(3, cmdList);

	if (!doTimestamp)
		cmdList->Dispatch(CHUNK_THREAD_GROUPS_X, CHUNK_THREAD_GROUPS_Y, CHUNK_THREAD_GROUPS_Z);
	else
	{
		D3D12Profiler::BeginCompute();
		D3D12Profiler::TimestampCompute(0);
		cmdList->Dispatch(CHUNK_THREAD_GROUPS_X, CHUNK_THREAD_GROUPS_Y, CHUNK_THREAD_GROUPS_Z);
		D3D12Profiler::TimestampCompute(1);
		D3D12Profiler::EndCompute();
	}
}

void Chunk::Render(ID3D12GraphicsCommandList * pCmdList, bool doTimestamp)
{
	m_svb.BindAndDraw(pCmdList, doTimestamp);
}

std::string Chunk::GetChunkInfoStr()
{
	std::stringstream ss;

	UINT cells = (DENSITY_THREAD_GROUPS_X - 1)*NUM_THREADS_PER_GROUP * (DENSITY_THREAD_GROUPS_Y - 1)*NUM_THREADS_PER_GROUP*(DENSITY_THREAD_GROUPS_Z - 1)*NUM_THREADS_PER_GROUP;

	ss << "Cells: " << cells << "\n";
	ss << "Chunk Scaling XYZ: " << m_cbData.m_dimension.x << " ; " << m_cbData.m_dimension.y << " ; " << m_cbData.m_dimension.z << "\n";
	ss << "Chunk origin XYZ: " << m_cbData.m_worldPos.x << " ; " << m_cbData.m_worldPos.y << " ; " << m_cbData.m_worldPos.z << "\n";
	ss << "Cell spacing: " << m_cbData.m_invVoxelDim << "\n";
	ss << "Generated Vertices: " << m_svb.GetVertexCount() << "\n";
	ss << "Generated Triangles: " << (m_svb.GetVertexCount() / 3) << "\n";

	return ss.str();
}
