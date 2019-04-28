#include "pch.h"
#include "TextureBuffer3D.h"

extern D3D12::Renderer gRenderer;

int TextureBuffer3D::TEXTURE3D_COUNT = 0;

TextureBuffer3D::TextureBuffer3D()
{
}


TextureBuffer3D::~TextureBuffer3D()
{
}

void TextureBuffer3D::Create3DTextureBuffer(Texture3DDesc tex3DDesc = Texture3DDesc())
{
	if (TEXTURE3D_COUNT + 1 == D3D12::MAX_TEXTURE3D_BUFFERS)
		std::exception();

	D3D12_HEAP_PROPERTIES defaultHeap;
	defaultHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeap.CreationNodeMask = 0;
	defaultHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeap.VisibleNodeMask = 0;

	HRESULT hr = gRenderer.GetDevice()->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
		&tex3DDesc.resDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		NULL,
		IID_PPV_ARGS(&m_3DTextureBuffer)
	);

	ThrowIfFailed(hr);

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D12_UNORDERED_ACCESS_VIEW_DESC));

	desc.Format = tex3DDesc.format;
	desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	desc.Texture3D = tex3DDesc.tex3D_UAV;

	auto uavHeapHandle = gRenderer.GetUAVHeap()->GetCPUDescriptorHandleForHeapStart();
	m_incrementSize = gRenderer.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	uavHeapHandle.ptr += (m_incrementSize * TEXTURE3D_COUNT);

	gRenderer.GetDevice()->CreateUnorderedAccessView(m_3DTextureBuffer, NULL, &desc, uavHeapHandle);

	m_bufferSlot = TEXTURE3D_COUNT;
	TEXTURE3D_COUNT++;
}

void TextureBuffer3D::Bind(UINT rootParameterIndex, ID3D12GraphicsCommandList * pComputeCmdList)
{
	if (!m_3DTextureBuffer)
		return;

	auto uavHeapHandle = gRenderer.GetUAVHeap()->GetGPUDescriptorHandleForHeapStart();
	uavHeapHandle.ptr += (m_incrementSize * m_bufferSlot);

	pComputeCmdList->SetGraphicsRootDescriptorTable(rootParameterIndex, uavHeapHandle);

}
