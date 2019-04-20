#pragma once
#include "RenderState.h"
#include "SOBuffer.h"

class GeometryStage
{
public:
	GeometryStage();
	~GeometryStage();

	void Init();

	void DrawAndExecute();
	void PrepareForRendering(ID3D12GraphicsCommandList * pCommandList);

	void Release();
private:
	RenderState m_rs;
	D3D12::Fence * m_fence;

	ID3D12Resource * m_vtxBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vbv;

	SOBuffer m_SOBuffer;

	struct Point
	{
		float p[4];
	};
private:
	void _initRenderState();
	void _initVertexBuffer();
	void _initSOBuffer();

	Point  m_points[4];
	UINT m_nrOfPoints = 0;
	void GenerateRandomPoints(UINT nrOfPoints, float radius);
};

