#pragma once
#include"ConstantBuffer.h"
#include "TextureBuffer3D.h"
#include "StructuredVertexBuffer.h"
// A chunk represents a cubical volume in space and is made up of voxels; voxels are also uniform cubes
class Chunk
{
public:
	Chunk(DirectX::XMFLOAT3 worldPos, DirectX::XMFLOAT3 dimension = {1.0f, 1.0f, 1.0f}, UINT voxels = 8);
	~Chunk();

	void CreateConstantBuffer();
	void CreateSVB();
	void Release();

	void GenerateVertices(TextureBuffer3D * pDensityTexture);
private:
	struct CBData
	{
		float m_invVoxelDim;
		DirectX::XMFLOAT3 m_worldPos;
		DirectX::XMFLOAT3 m_dimension;
		UINT m_voxelDimension;
	} m_cbData;

	ConstantBuffer m_cb;
	StructuredVertexBuffer m_svb;
};

