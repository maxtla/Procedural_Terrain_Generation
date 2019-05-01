#include "pch.h"
#include "RenderState.h"

extern AppCtx gAppCtx;

RenderState::RenderState()
{
}


RenderState::~RenderState()
{
	if (m_pso)
	{
		m_pso->Release();
		m_pso = NULL;
	}
}

void RenderState::AddShader(ShaderStages stage, Shader shader)
{
	m_shaders[stage] = shader;
}

HRESULT RenderState::CreatePipelineState(ID3D12RootSignature * pRS, StateDescription stateDesc, bool compute)
{
	m_isCompute = compute;
	if (!compute)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

		size_t descSize = sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC);
		size_t margin = sizeof(StateDescription);

		ZeroMemory(&psoDesc, descSize);

		psoDesc.pRootSignature = pRS;
		psoDesc.VS = { (const void*)m_shaders[VS].compiledShaderCode, m_shaders[VS].bufferSize };
		psoDesc.HS = { (const void*)m_shaders[HS].compiledShaderCode, m_shaders[HS].bufferSize };
		psoDesc.DS = { (const void*)m_shaders[DS].compiledShaderCode, m_shaders[DS].bufferSize };
		psoDesc.GS = { (const void*)m_shaders[GS].compiledShaderCode, m_shaders[GS].bufferSize };
		psoDesc.PS = { (const void*)m_shaders[PS].compiledShaderCode, m_shaders[PS].bufferSize };

		size_t offset = descSize - margin;
		uint8_t * dest = (uint8_t*)&psoDesc;
		dest += offset;
		memcpy((void*)dest, (void*)&stateDesc, margin);

		ID3D12Device * pDev = nullptr;
		pRS->GetDevice(IID_PPV_ARGS(&pDev));
		HRESULT hr = pDev->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso));
		pDev->Release();
		return hr;
	}
	else
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

		psoDesc.pRootSignature = pRS;
		psoDesc.CS = { (const void*)m_shaders[CS].compiledShaderCode, m_shaders[CS].bufferSize };
		psoDesc.NodeMask = stateDesc.NodeMask;
		psoDesc.CachedPSO = stateDesc.CachedPSO;
		psoDesc.Flags = stateDesc.Flags;

		ID3D12Device * pDev = nullptr;
		pRS->GetDevice(IID_PPV_ARGS(&pDev));
		HRESULT hr = pDev->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_pso));
		pDev->Release();
		return hr;
	}
}

RenderState::StateDescription RenderState::GetDefaultStateDescription()
{
	StateDescription desc;
	ZeroMemory(&desc, sizeof(StateDescription));

	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	desc.SampleMask = 0xffffffff;
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	
	static D3D12_INPUT_ELEMENT_DESC inputElementDescs[2];
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	desc.InputLayout.pInputElementDescs = inputElementDescs;
	desc.InputLayout.NumElements = _ARRAYSIZE(inputElementDescs);
	desc.NumRenderTargets = gAppCtx.backbufferCount;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count = 1;

	return desc;
}

void RenderState::Apply(ID3D12GraphicsCommandList * pCommandList)
{
	pCommandList->SetPipelineState(m_pso);
}
