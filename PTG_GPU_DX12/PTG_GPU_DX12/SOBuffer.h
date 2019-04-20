#pragma once
class SOBuffer
{
public:
	SOBuffer();
	~SOBuffer();

	void Init();
	void Apply(ID3D12GraphicsCommandList * pCommandList);
	ID3D12Resource * GetSOBuffer() { return m_SOBuffer; }
	UINT GetBufferSize() { return m_bufferSize; }
	UINT64 GetNrOfWrittenVertices();
private:
	ID3D12Resource * m_SOBuffer = NULL;
	ID3D12Resource * m_SizeLocationBuffer = NULL;
	UINT m_bufferSize;
	
	D3D12_STREAM_OUTPUT_BUFFER_VIEW m_SOBV;
};

