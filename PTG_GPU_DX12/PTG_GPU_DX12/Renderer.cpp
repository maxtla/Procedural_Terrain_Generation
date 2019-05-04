#include "pch.h"
#include "Renderer.h"

UINT CHUNK_THREAD_GROUPS = DENSITY_THREAD_GROUPS;

namespace D3D12
{
	UINT Renderer::CONSTANT_BUFFER_COUNT = 0U;
	UINT Renderer::UAV_COUNT = 0U;
	UINT Renderer::SRV_COUNT = 0U;

	Renderer::Renderer()
	{
	}


	Renderer::~Renderer()
	{
	}

	bool Renderer::Init(AppCtx appCtx)
	{
		HRESULT hr = E_FAIL;
		//Debug layer
#ifdef _DEBUG
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&this->m_debugController));
		if (SUCCEEDED(hr))
			m_debugController->EnableDebugLayer();
		else
			OutputDebugStringA("\nFailed to create D3D12 Debug Interface\n");
#endif // _DEBUG

		//Find a suitable DX12 adapter
		IDXGIFactory5 * factory = NULL;
		IDXGIAdapter1 * adapter = _findDX12Adapter(&factory);

		//create our d3d device or a warp device if no suitable adapter was found
		hr = _createDevice(&factory, &adapter);
		if (FAILED(hr))
		{
			return hr;
		}

		//create command queue
		hr = _createCommandQueues();
		if (FAILED(hr))
		{
			return hr;
		}
		//create command allocator and command list
		hr = _createCmdAllocatorsAndLists();
		if (FAILED(hr))
		{
			return hr;
		}
		BUFFER_COUNT = appCtx.backbufferCount;
		//create the swapchain
		hr = _createSwapChain(appCtx.hwnd, appCtx.width, appCtx.height, &factory);
		if (FAILED(hr))
		{
			return hr;
		}
		//create the render targets
		hr = _createHeapsAndResources();
		if (FAILED(hr))
		{
			return hr;
		}
		//create the root signature
		hr = _createRootSignature();
		if (FAILED(hr))
		{
			return hr;
		}
		//create the Depthbuffer and DSV
		hr = _createDepthBuffer((float)appCtx.width, (float)appCtx.height);
		if (FAILED(hr))
		{
			return hr;
		}

		initialized = true;

		SafeRelease(&factory);
		SafeRelease(&adapter);

		//Define the viewport and scissor rect
		{
			this->m_viewPort.TopLeftX = 0.f;
			this->m_viewPort.TopLeftY = 0.f;
			this->m_viewPort.MinDepth = 0.f;
			this->m_viewPort.MaxDepth = 1.f;
			this->m_viewPort.Width = (float)appCtx.width;
			this->m_viewPort.Height = (float)appCtx.height;

			this->m_scissorRect.left = (long)this->m_viewPort.TopLeftX;
			this->m_scissorRect.top = (long)this->m_viewPort.TopLeftY;
			this->m_scissorRect.right = (long)this->m_viewPort.Width;
			this->m_scissorRect.bottom = (long)this->m_viewPort.Height;
		}
		
		//m_device->SetStablePowerState(true);

		return hr;
	}

	void Renderer::StartFrame()
	{
		using namespace DirectX;

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		D3D12_CPU_DESCRIPTOR_HANDLE cdh_DSV = m_depthHeap->GetCPUDescriptorHandleForHeapStart();

		m_directAllocator->Reset();
		m_gCmdList->Reset(m_directAllocator, nullptr);

		m_gCmdList->SetGraphicsRootSignature(m_rootSignature);
		m_gCmdList->RSSetViewports(1, &m_viewPort);
		m_gCmdList->RSSetScissorRects(1, &m_scissorRect);
		
		// Indicate that the back buffer will be used as a render target.
		m_gCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		m_gCmdList->OMSetRenderTargets(1, &rtvHandle, TRUE, &cdh_DSV);

		// Record commands.
		m_gCmdList->ClearRenderTargetView(rtvHandle, m_clearColor, 0, nullptr);
		m_gCmdList->ClearDepthStencilView(cdh_DSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		m_gCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		
	}

	void Renderer::EndFrame()
	{
		// Indicate that the back buffer will now be used to present.
		m_gCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		m_gCmdList->Close();

	}

	void Renderer::RenderFrame(Fence * fence)
	{
		m_directQ->ExecuteCommandLists(1, (ID3D12CommandList * const*)&m_gCmdList);
		WaitForGPUCompletion(m_directQ, fence);
	}

	void Renderer::Present()
	{
		DXGI_PRESENT_PARAMETERS pp = {};
		m_swapChain->Present1(0, 0, &pp);
	}

	void Renderer::Reset()
	{
		m_directAllocator->Reset();
		m_gCmdList->Reset(m_directAllocator, nullptr);
	}

	Fence * Renderer::MakeFence(UINT64 initialValue, UINT64 completionValue, D3D12_FENCE_FLAGS flag, const wchar_t * debugName)
	{
		if (!initialized)
			return nullptr;

		Fence * fence = nullptr;
		fence = new Fence();

		HRESULT hr = m_device->CreateFence(initialValue, flag, IID_PPV_ARGS(&fence->m_fence));
		if (SUCCEEDED(hr))
		{
			fence->m_fenceValue = completionValue;
			fence->m_fence->SetName(debugName);
			fence->m_fenceEvent = CreateEvent(0, false, false, 0);
			return fence;
		}
		else
		{
			delete fence;
			return nullptr;
		}
	}

	bool Renderer::DestroyFence(Fence * fence)
	{
		if (!initialized)
			return false;

		if (fence)
		{
			SafeRelease(&fence->m_fence);
			CloseHandle(fence->m_fenceEvent);
			delete fence;
			return true;
		}
		return false;
	}

	void Renderer::CleanUp()
	{
#ifdef _DEBUG
		SafeRelease(&m_debugController);
#endif // _DEBUG
		SafeRelease(&m_swapChain);
		SafeRelease(&m_directQ);
		SafeRelease(&m_directAllocator);
		SafeRelease(&m_gCmdList);
		SafeRelease(&m_rtvHeap);
		SafeRelease(&m_rootSignature);
		SafeRelease(&m_DescHeap);
		SafeRelease(&m_depthHeap);
		SafeRelease(&m_depthBuffer);
		SafeRelease(&m_computeAllocator);
		SafeRelease(&m_computeQ);
		SafeRelease(&m_computeCmdList);

		for (unsigned int i = 0; i < BUFFER_COUNT; i++)
			SafeRelease(&m_renderTargets[i]);
		delete[] m_renderTargets;

		SafeRelease(&m_device);
		initialized = false;
	}

	IDXGIAdapter1 * Renderer::_findDX12Adapter(IDXGIFactory5 ** ppFactory)
	{
		IDXGIAdapter1 * adapter = NULL;

		//Create the factory and iterate through adapters, find and return the first adapter that supports DX12
		if (!*ppFactory)
			CreateDXGIFactory(IID_PPV_ARGS(ppFactory));
		assert(*ppFactory);

		for (UINT index = 0;; ++index)
		{
			adapter = NULL;
			if (DXGI_ERROR_NOT_FOUND == (*ppFactory)->EnumAdapters1(index, &adapter))
				break;
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr)))
				break;

			SafeRelease(&adapter);
		}

		return adapter;
	}

	HRESULT Renderer::_createDevice(IDXGIFactory5 ** ppFactory, IDXGIAdapter1 ** ppAdapter)
	{
		HRESULT hr = E_FAIL;
		if (*ppAdapter)
		{
			hr = D3D12CreateDevice(*ppAdapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&this->m_device));
			m_device->SetName(L"DX12_DEVICE");
		}
		else
		{
			(*ppFactory)->EnumWarpAdapter(IID_PPV_ARGS(ppAdapter));
			hr = D3D12CreateDevice(*ppAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&this->m_device));
		}
		return hr;
	}

	HRESULT Renderer::_createCommandQueues()
	{
		//Description
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		HRESULT hr = m_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_directQ));
		if (SUCCEEDED(hr))
			m_directQ->SetName(L"Direct Command Queue");

		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

		hr = m_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_computeQ));
		if (SUCCEEDED(hr))
			m_computeQ->SetName(L"Direct Command Queue");
		return hr;
	}

	HRESULT Renderer::_createCmdAllocatorsAndLists()
	{
		HRESULT hr = E_FAIL;

		hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_directAllocator));

		if (FAILED(hr))
			return hr;
		m_directAllocator->SetName(L"Direct Allocator");

		hr = m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_directAllocator,
			nullptr,
			IID_PPV_ARGS(&m_gCmdList)
		);

		if (SUCCEEDED(hr))
		{
			m_gCmdList->Close();
			m_gCmdList->SetName(L"Direct Command List");
		}

		hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&m_computeAllocator));

		if (FAILED(hr))
			return hr;
		m_directAllocator->SetName(L"Compute Allocator");

		hr = m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			m_computeAllocator,
			nullptr,
			IID_PPV_ARGS(&m_computeCmdList)
		);

		if (SUCCEEDED(hr))
		{
			m_computeCmdList->Close();
			m_computeCmdList->SetName(L"Compute Command List");
		}
		return hr;
	}

	HRESULT Renderer::_createSwapChain(HWND hwnd, UINT width, UINT height, IDXGIFactory5 ** ppFactory)
	{
		HRESULT hr = E_FAIL;

		//Description
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		swapChainDesc.BufferCount = BUFFER_COUNT;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapChainDesc.Flags = 0;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;


		hr = (*ppFactory)->CreateSwapChainForHwnd(
			m_directQ,
			hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			reinterpret_cast<IDXGISwapChain1**>(&m_swapChain)
		);


		return hr;
	}

	HRESULT Renderer::_createHeapsAndResources()
	{
		HRESULT hr = E_FAIL;

		//Description for descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
		dhd.NumDescriptors = BUFFER_COUNT;
		dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		hr = m_device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_rtvHeap));
		if (FAILED(hr))
			return hr;
		m_rtvHeap->SetName(L"RTV HEAP");

		//Create resources for render targets
		UINT m_rtvDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_HEAP_PROPERTIES  heapProp;
		ZeroMemory(&heapProp, sizeof(D3D12_HEAP_PROPERTIES));
		heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.CreationNodeMask = 0;
		heapProp.VisibleNodeMask = 0;

		m_renderTargets = new ID3D12Resource*[BUFFER_COUNT];
		//one RTV for each swapchain buffer
		for (UINT i = 0; i < BUFFER_COUNT; i++)
		{
			hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
			m_renderTargets[i]->SetName((std::wstring(L"BackBuffer_") + std::to_wstring(i)).c_str());
			if (SUCCEEDED(hr))
			{
				m_device->CreateRenderTargetView(m_renderTargets[i], nullptr, cdh);
				cdh.ptr += m_rtvDescSize;
			}
		}

		dhd.NumDescriptors = HEAP_DESCRIPTOR_COUNT;
		dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		hr = m_device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_DescHeap));
		if (FAILED(hr))
			return hr;
		m_rtvHeap->SetName(L"COMMON DESCRIPTOR HEAP");

		return hr;
	}

	HRESULT Renderer::_createRootSignature()
	{
		//Important things to identify: 
	// 1. How many root constants does this program need? (1 DWORD, 0 indirections)
	// 2. How many Root Descriptors does this program need/have (CBV, SRV, UAV)? (2 DWORDs, 1 indirection)
	// 3. How many Descriptor Tables do we need/want? (1 DWORD, 2 indirections)
	// 4. How many DWORDs did we use (Max size of 64, or 63 with enabled IA)

		//Dynamic constant buffers
		D3D12_ROOT_CONSTANTS rootConstants[2];
		rootConstants[0].Num32BitValues = 32; //We have two 4x4 Matrices, View and Proj
		rootConstants[0].RegisterSpace = 0;
		rootConstants[0].ShaderRegister = 0; //b0

		rootConstants[1].Num32BitValues = 2; //ThreadGroups and NumofThreadsperGroup
		rootConstants[1].RegisterSpace = 0;
		rootConstants[1].ShaderRegister = 2; //b2

		D3D12_ROOT_DESCRIPTOR_TABLE cbvDescTable;
		D3D12_DESCRIPTOR_RANGE cbvRange;
		//Less Dynamic constant buffers
		{
			cbvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			cbvRange.NumDescriptors = 1; //1 cbv at most
			cbvRange.BaseShaderRegister = 1; //b1
			cbvRange.RegisterSpace = 0;
			cbvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			cbvDescTable.pDescriptorRanges = &cbvRange;
			cbvDescTable.NumDescriptorRanges = 1;
		}

		D3D12_ROOT_DESCRIPTOR_TABLE uavDescTable;
		D3D12_DESCRIPTOR_RANGE uavRange;
		//Less Dynamic constant buffers
		{
			uavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			uavRange.NumDescriptors = 1;
			uavRange.BaseShaderRegister = 0; //u0
			uavRange.RegisterSpace = 0;
			uavRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			uavDescTable.pDescriptorRanges = &uavRange;
			uavDescTable.NumDescriptorRanges = 1;
		}

		D3D12_ROOT_DESCRIPTOR_TABLE srvDescTable;
		D3D12_DESCRIPTOR_RANGE srvRange;
		//Less Dynamic constant buffers
		{
			srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			srvRange.NumDescriptors = 1;
			srvRange.BaseShaderRegister = 0; //t0
			srvRange.RegisterSpace = 0;
			srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			srvDescTable.pDescriptorRanges = &srvRange;
			srvDescTable.NumDescriptorRanges = 1;
		}

		D3D12_ROOT_PARAMETER rootParams[5];
		{
			rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			rootParams[0].Constants = rootConstants[0];
			rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParams[1].DescriptorTable = cbvDescTable;
			rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParams[2].DescriptorTable = uavDescTable;
			rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			rootParams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParams[3].DescriptorTable = srvDescTable;
			rootParams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			rootParams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			rootParams[4].Constants = rootConstants[1];
			rootParams[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		}

		// Create descriptor of static sampler LinearClamp
		D3D12_STATIC_SAMPLER_DESC sampler{};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.MipLODBias = 0.0f;
		sampler.MaxAnisotropy = 16;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = -D3D12_FLOAT32_MAX;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_SIGNATURE_DESC rsDesc{};
		rsDesc.NumParameters = _ARRAYSIZE(rootParams); //How many entries?
		rsDesc.pParameters = rootParams; //Pointer to array of table entries
		rsDesc.NumStaticSamplers = 1;  //One static samplers were defined
		rsDesc.pStaticSamplers = &sampler; // The static sampler
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;

		//Serialize the root signature (no error blob)
		ID3DBlob * pSerBlob = NULL;
		ID3DBlob * pError = NULL;
		HRESULT hr = D3D12SerializeRootSignature(
			&rsDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&pSerBlob,
			&pError
		);

		if (FAILED(hr))
		{
			_com_error err(hr);
			OutputDebugStringW(err.ErrorMessage());
			OutputDebugStringA((const char*)pError->GetBufferPointer());
		}
		//Use d3d12 device to create the root signature
		UINT nodeMask = 0;

		return this->m_device->CreateRootSignature(nodeMask, pSerBlob->GetBufferPointer(), pSerBlob->GetBufferSize(), IID_PPV_ARGS(&this->m_rootSignature));
	}

	HRESULT Renderer::_createDepthBuffer(float width, float height)
	{
		HRESULT hr = E_FAIL;

		// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_depthHeap));
		if (FAILED(hr))
			return hr;

		m_depthHeap->SetName(L"Depth Resource Heap");

		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
		textureDesc.Width = (UINT64)width;
		textureDesc.Height = (UINT)height;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Alignment = 0;
		textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		hr = m_device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_depthBuffer)
		);

		if (FAILED(hr))
			return hr;

		m_depthBuffer->SetName(L"DepthBufferResource");

		m_device->CreateDepthStencilView(m_depthBuffer, &depthStencilDesc, m_depthHeap->GetCPUDescriptorHandleForHeapStart());

		return hr;
	}

	void Renderer::WaitForGPUCompletion(ID3D12CommandQueue * pCmdQ, Fence * fence)
	{
		const UINT64 fenceValue = fence->m_fenceValue;
		pCmdQ->Signal(fence->m_fence, fenceValue);
		fence->m_fenceValue++;

		UINT64 val = fence->m_fence->GetCompletedValue();
		if (val < fenceValue)
		{
			fence->m_fence->SetEventOnCompletion(fenceValue, fence->m_fenceEvent);
			WaitForSingleObject(fence->m_fenceEvent, INFINITE);
		}
	}

}
