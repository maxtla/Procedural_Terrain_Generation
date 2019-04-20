#include "pch.h"
#include "GeometryStage.h"

extern D3D12::Renderer gRenderer;
extern std::vector<Shader> gShaderCollection;

GeometryStage::GeometryStage()
{
}


GeometryStage::~GeometryStage()
{
}

void GeometryStage::Init()
{
	m_fence = gRenderer.MakeFence(0, 1, D3D12_FENCE_FLAG_NONE, L"GS_Fence");
	_initRenderState();
	_initVertexBuffer();
	_initSOBuffer();
}

void GeometryStage::DrawAndExecute()
{
	gRenderer.Reset();

	auto list = gRenderer.GetCommandList();
	auto queue = gRenderer.GetCommandQueue();

	list->SetGraphicsRootSignature(gRenderer.GetRootSignature());
	list->IASetVertexBuffers(0, 1, &m_vbv);
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	
	m_rs.Apply(list);
	m_SOBuffer.Apply(list);

	list->DrawInstanced(m_nrOfPoints, 1, 0, 0);

	list->Close();

	queue->ExecuteCommandLists(1, (ID3D12CommandList * const*)&list);
	gRenderer.WaitForGPUCompletion(queue, m_fence);
}

void GeometryStage::PrepareForRendering(ID3D12GraphicsCommandList * pCommandList)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SOBuffer.GetSOBuffer(), D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	static D3D12_VERTEX_BUFFER_VIEW vtv = {};
	vtv.BufferLocation = m_SOBuffer.GetSOBuffer()->GetGPUVirtualAddress();
	vtv.SizeInBytes = m_SOBuffer.GetBufferSize();
	vtv.StrideInBytes = sizeof(float) * 8;

	pCommandList->IASetVertexBuffers(0, 1, &vtv);
	pCommandList->DrawInstanced(24*4, 1, 0, 0);

	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SOBuffer.GetSOBuffer(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,D3D12_RESOURCE_STATE_STREAM_OUT));
}

void GeometryStage::Release()
{
	gRenderer.DestroyFence(m_fence);
	SafeRelease(&m_vtxBuffer);
}

void GeometryStage::_initRenderState()
{
	RenderState::StateDescription stateDesc;
	ZeroMemory(&stateDesc, sizeof(RenderState::StateDescription));

	stateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	stateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1];
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	stateDesc.InputLayout.pInputElementDescs = inputElementDescs;
	stateDesc.InputLayout.NumElements = 1;

	stateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

	D3D12_STREAM_OUTPUT_DESC soDesc;
	ZeroMemory(&soDesc, sizeof(D3D12_STREAM_OUTPUT_DESC));
	D3D12_SO_DECLARATION_ENTRY soDeclarations[2];

	soDeclarations[0].Stream = 0;
	soDeclarations[0].SemanticName = "POSITION";
	soDeclarations[0].SemanticIndex = 0;
	soDeclarations[0].StartComponent = 0;
	soDeclarations[0].ComponentCount = 4;
	soDeclarations[0].OutputSlot = 0;

	soDeclarations[1].Stream = 0;
	soDeclarations[1].SemanticName = "COLOR";
	soDeclarations[1].SemanticIndex = 0;
	soDeclarations[1].StartComponent = 0;
	soDeclarations[1].ComponentCount = 4;
	soDeclarations[1].OutputSlot = 0;

	UINT bufferStrides[1];
	bufferStrides[0] = sizeof(float) * 8;

	soDesc.pSODeclaration = soDeclarations;
	soDesc.NumEntries = 2;
	soDesc.pBufferStrides = bufferStrides;
	soDesc.NumStrides = 1;
	soDesc.RasterizedStream = 0;

	stateDesc.StreamOutput = soDesc;

	m_rs.AddShader(VS, gShaderCollection[2]);
	m_rs.AddShader(GS, gShaderCollection[3]);

	m_rs.CreatePipelineState(gRenderer.GetRootSignature(), stateDesc);
}

void GeometryStage::_initVertexBuffer()
{
	GenerateRandomPoints(4, 3.f);

	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	gRenderer.GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(Point) * m_nrOfPoints),
		D3D12_RESOURCE_STATE_COPY_DEST,
		NULL,
		IID_PPV_ARGS(&m_vtxBuffer)
	);

	if (m_vtxBuffer)
	{
		m_vtxBuffer->SetName(L"GSVertexBuffer");

		ID3D12Resource * pUploadResource = NULL;
		gRenderer.GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(Point) * m_nrOfPoints), // resource description for a buffer
			D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
			nullptr,
			IID_PPV_ARGS(&pUploadResource));

		D3D12_SUBRESOURCE_DATA vtxData = {};
		vtxData.pData = (void*)m_points;
		vtxData.RowPitch = sizeof(Point) * m_nrOfPoints;
		vtxData.SlicePitch = sizeof(Point) * m_nrOfPoints;

		gRenderer.Reset();

		UpdateSubresources(gRenderer.GetCommandList(), m_vtxBuffer, pUploadResource, 0, 0, 1, &vtxData);

		gRenderer.GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vtxBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		gRenderer.GetCommandList()->Close();
		auto q = gRenderer.GetCommandQueue();
		auto l = gRenderer.GetCommandList();
		q->ExecuteCommandLists(1, (ID3D12CommandList*const*)&l);

		gRenderer.WaitForGPUCompletion(q, m_fence);

		pUploadResource->Release();
	}

	m_vbv.BufferLocation = m_vtxBuffer->GetGPUVirtualAddress();
	m_vbv.SizeInBytes = sizeof(Point) * m_nrOfPoints;
	m_vbv.StrideInBytes = sizeof(Point);
}

void GeometryStage::_initSOBuffer()
{
	m_SOBuffer.Init();
}

void GeometryStage::GenerateRandomPoints( UINT nrOfPoints, float radius)
{
	m_nrOfPoints = nrOfPoints;

	for (unsigned int i = 0; i < nrOfPoints; i++)
	{
		m_points[i].p[0] = (float)(rand() % 1000) / 1000.f;
		m_points[i].p[1] = (float)(rand() % 1000) / 1000.f;
		m_points[i].p[2] = (float)(rand() % 1000) / 1000.f;
	}
}
