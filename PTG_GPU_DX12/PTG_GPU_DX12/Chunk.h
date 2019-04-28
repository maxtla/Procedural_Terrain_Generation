#pragma once
#include"ConstantBuffer.h"
#include"TextureBuffer3D.h"
// A chunk represents a cubical volume in space and is made up of voxels; voxels are also uniform cubes
class Chunk
{
public:
	Chunk(DirectX::XMFLOAT3 worldPos, DirectX::XMFLOAT3 dimension = {1.0f, 1.0f, 1.0f}, UINT voxels = 8);
	~Chunk();

	void CreateConstantBuffer();
	void GenerateVertices(ID3D12GraphicsCommandList * pComputeCmdList, TextureBuffer3D * pDensityTexture);
private:
	struct CBData
	{
		DirectX::XMFLOAT3 m_worldPos;
		DirectX::XMFLOAT3 m_dimension;
		UINT m_voxelDimension;
	} m_cbData;

	ConstantBuffer m_cb;


};

