#pragma once
class ConstantBuffer
{
public:
	ConstantBuffer();
	~ConstantBuffer();

	HRESULT CreateConstantBuffer(ID3D12DescriptorHeap * pCBVHeap, ID3D12Device * pDev, size_t bufferWidth);
	void Release();

	void UpdateBuffer(void * src, size_t size);
	void BindBuffer(UINT rootParameterIndex, ID3D12GraphicsCommandList * pCommandList, bool compute);

private:
	
	ID3D12Resource * m_buffer = NULL;
	UINT m_bufferSlot;
	UINT m_incrementSize;
};

