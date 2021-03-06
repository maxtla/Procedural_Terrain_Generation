#pragma once
class StructuredVertexBuffer
{
public:
	StructuredVertexBuffer();
	~StructuredVertexBuffer();

	void CreateBuffer();
	void Release();

	void BindBuffer(UINT rootParameterIndex, ID3D12GraphicsCommandList * pCmdList, bool compute = false);
	void BindAndDraw(ID3D12GraphicsCommandList * pCmdList, bool doTimestamp);

	UINT GetVertexCount() { return m_vertexCount; }
private:
	ID3D12Resource * m_vertexBuffer = NULL;
	ID3D12Resource * m_counter = NULL;
	UINT m_bufferIndex;
	UINT m_incrementSize;
	D3D12_VERTEX_BUFFER_VIEW m_vbv;
	UINT m_vertexCount = 0;
};

