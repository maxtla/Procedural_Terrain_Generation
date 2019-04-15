#include "pch.h"
#include "Renderer.h"

namespace D3D12
{
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

		initialized = true;

		SafeRelease(&factory);
		SafeRelease(&adapter);

		return hr;
	}

	void Renderer::StartFrame()
	{
		using namespace DirectX;

		UINT frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		m_directAllocator->Reset();
		m_gCmdList->Reset(m_directAllocator, nullptr);

		m_gCmdList->SetGraphicsRootSignature(m_rootSignature);
		
		// Indicate that the back buffer will be used as a render target.
		m_gCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		m_gCmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		// Record commands.
		m_gCmdList->ClearRenderTargetView(rtvHandle, m_clearColor, 0, nullptr);
	}

	void Renderer::EndFrame()
	{
		UINT frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		// Indicate that the back buffer will now be used to present.
		m_gCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		m_gCmdList->Close();

		m_directQ->ExecuteCommandLists(1, (ID3D12CommandList * const*)&m_gCmdList);
	}

	void Renderer::Present(Fence * fence)
	{
		WaitForGPUCompletion(m_directQ, fence);

		DXGI_PRESENT_PARAMETERS pp = {};
		m_swapChain->Present1(0, 0, &pp);
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

		for (int i = 0; i < BUFFER_COUNT; i++)
			SafeRelease(&m_renderTargets[i]);
		
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
		return hr;
	}

	HRESULT Renderer::_createRootSignature()
	{
		//Important things to identify: 
	// 1. How many root constants does this program need? (1 DWORD, 0 indirections)
	// 2. How many Root Descriptors does this program need/have (CBV, SRV, UAV)? (2 DWORDs, 1 indirection)
	// 3. How many Descriptor Tables do we need/want? (1 DWORD, 2 indirections)
	// 4. How many DWORDs did we use (Max size of 64, or 63 with enabled IA)

		//This is the constant buffer
		D3D12_ROOT_CONSTANTS rootConstants[2];
		rootConstants[0].Num32BitValues = 32; //We have two 4x4 Matrices, View and Proj
		rootConstants[0].RegisterSpace = 0;
		rootConstants[0].ShaderRegister = 0; //b0

		rootConstants[1].Num32BitValues = 16; //We have one 4x4 Matrices, World
		rootConstants[1].RegisterSpace = 0;
		rootConstants[1].ShaderRegister = 1; //b1

		D3D12_ROOT_PARAMETER rootParams[2];
		{
			rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			rootParams[0].Constants = rootConstants[0];
			rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			rootParams[1].Constants = rootConstants[1];
			rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		}

		// Create descriptor of static sampler
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
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

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
		}
		//Use d3d12 device to create the root signature
		UINT nodeMask = 0;

		return this->m_device->CreateRootSignature(nodeMask, pSerBlob->GetBufferPointer(), pSerBlob->GetBufferSize(), IID_PPV_ARGS(&this->m_rootSignature));
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
