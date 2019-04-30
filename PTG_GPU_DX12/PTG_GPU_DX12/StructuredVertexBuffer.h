#pragma once
class StructuredVertexBuffer
{
public:
	static const UINT MAX_BUFFERS = 4;
	static ID3D12DescriptorHeap * m_heap;
	static UINT BUFFER_COUNT;
	static void InitHeap();
	static void FreeHeap();

	StructuredVertexBuffer();
	~StructuredVertexBuffer();

	void CreateBuffer();
	void Release();

	void BindBuffer(UINT rootParameterIndex, ID3D12GraphicsCommandList * pCmdList, bool compute = false);
	D3D12_VERTEX_BUFFER_VIEW GetVBV() { return m_vbv; }
private:
	ID3D12Resource * m_vertexBuffer = NULL;
	ID3D12Resource * m_counter = NULL;
	UINT m_bufferIndex;
	UINT m_incrementSize;
	D3D12_VERTEX_BUFFER_VIEW m_vbv;
};

