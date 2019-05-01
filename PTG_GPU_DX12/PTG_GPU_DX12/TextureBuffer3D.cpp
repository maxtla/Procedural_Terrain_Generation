#include "pch.h"
#include "TextureBuffer3D.h"

extern D3D12::Renderer gRenderer;

TextureBuffer3D::TextureBuffer3D()
{
}


TextureBuffer3D::~TextureBuffer3D()
{
	
}

void TextureBuffer3D::Create3DTextureBuffer(Texture3DDesc tex3DDesc = Texture3DDesc())
{
	if (D3D12::Renderer::UAV_COUNT + 1 == HEAP_MAX_UAV)
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
	std::wstring name = L"DensityVolume_" + std::to_wstring(D3D12::Renderer::UAV_COUNT);
	m_3DTextureBuffer->SetName(name.c_str());

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D12_UNORDERED_ACCESS_VIEW_DESC));

	desc.Format = tex3DDesc.format;
	desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	desc.Texture3D = tex3DDesc.tex3D_UAV;

	auto uavHeapHandle = gRenderer.GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	m_incrementSize = gRenderer.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	uavHeapHandle.ptr += m_incrementSize * (HEAP_UAV_OFFSET + D3D12::Renderer::UAV_COUNT);

	gRenderer.GetDevice()->CreateUnorderedAccessView(m_3DTextureBuffer, NULL, &desc, uavHeapHandle);

	m_UAVSlot = D3D12::Renderer::UAV_COUNT++;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));

	srvDesc.Format = tex3DDesc.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 0, 0, 0);//D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.MipLevels = 1;
	srvDesc.Texture3D.ResourceMinLODClamp = 0.f;

	auto srvHeapHandle = gRenderer.GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();

	srvHeapHandle.ptr += m_incrementSize * (HEAP_SRV_OFFSET + D3D12::Renderer::SRV_COUNT);

	gRenderer.GetDevice()->CreateShaderResourceView(m_3DTextureBuffer, &srvDesc, srvHeapHandle);

	m_SRVSlot = D3D12::Renderer::SRV_COUNT++;
}

void TextureBuffer3D::BindUAV(UINT rootParameterIndex, ID3D12GraphicsCommandList * pComputeCmdList)
{
	if (!m_3DTextureBuffer)
		return;

	auto uavHeapHandle = gRenderer.GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	uavHeapHandle.ptr += m_incrementSize * (HEAP_UAV_OFFSET +  m_UAVSlot);

	pComputeCmdList->SetComputeRootDescriptorTable(rootParameterIndex, uavHeapHandle);

}

void TextureBuffer3D::BindSRV(UINT rootParameterIndex, ID3D12GraphicsCommandList * pComputeCmdList)
{
	if (!m_3DTextureBuffer)
		return;

	auto srvHeapHandle = gRenderer.GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	srvHeapHandle.ptr += m_incrementSize * (HEAP_SRV_OFFSET + m_SRVSlot);

	pComputeCmdList->SetComputeRootDescriptorTable(rootParameterIndex, srvHeapHandle);
}
