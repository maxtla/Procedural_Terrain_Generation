#pragma once
class TextureBuffer3D
{
public:
	static int TEXTURE3D_COUNT;

	TextureBuffer3D();
	~TextureBuffer3D();
	struct Texture3DDesc
	{
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		D3D12_TEX3D_UAV tex3D_UAV = { 0U, 0U, 8U };
		D3D12_RESOURCE_DESC resDesc = {
			D3D12_RESOURCE_DIMENSION_TEXTURE3D, //ViewDimension
			0U, //Alignment
			8U, //Width
			8U, //Height
			8U, //Depth
			1U, //MipLevels
			DXGI_FORMAT_R32_FLOAT,
			{1U, 0U}, //SampleDesc
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
		};
	};
	void Create3DTextureBuffer(Texture3DDesc desc);
	void Bind(UINT rootParameterIndex, ID3D12GraphicsCommandList * pComputeCmdList);
private:
	ID3D12Resource * m_3DTextureBuffer = NULL;

	UINT m_bufferSlot;
	UINT m_incrementSize;
};

