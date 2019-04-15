#pragma once
struct Shader;

const UINT MAX_SHADERS = 6U; //VS, HS, DS, GS, PS, CS
enum ShaderStages
{
	VS = 0, HS = 1, DS = 2, GS = 3, PS = 4, CS = 5
};

class RenderState
{
public:
	struct StateDescription //Partial D3D12_GRAPHICS_PIPELINE_STATE_DESC
	{
		D3D12_STREAM_OUTPUT_DESC						StreamOutput;
		D3D12_BLEND_DESC										BlendState;
		UINT																SampleMask;
		D3D12_RASTERIZER_DESC								RasterizerState;
		D3D12_DEPTH_STENCIL_DESC						DepthStencilState;
		D3D12_INPUT_LAYOUT_DESC							InputLayout;
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE		IBStripCutValue;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE				PrimitiveTopologyType;
		UINT																NumRenderTargets;
		DXGI_FORMAT													RTVFormats[8];
		DXGI_FORMAT													DSVFormat;
		DXGI_SAMPLE_DESC										SampleDesc;
		UINT																NodeMask;
		D3D12_CACHED_PIPELINE_STATE					CachedPSO;
		D3D12_PIPELINE_STATE_FLAGS						Flags;
	};

	RenderState();
	~RenderState();

	void AddShader(ShaderStages stage, Shader shader);
	HRESULT CreatePipelineState(ID3D12RootSignature * pRS, StateDescription stateDesc, bool compute = false);

	bool IsCompute() { return m_isCompute; }
	StateDescription GetDefaultStateDescription();
private:
	Shader m_shaders[MAX_SHADERS];
	ID3D12PipelineState * m_pso = nullptr;
	bool m_isCompute = false;
};

