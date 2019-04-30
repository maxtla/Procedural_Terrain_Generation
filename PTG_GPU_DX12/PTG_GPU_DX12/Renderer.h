#pragma once
#include <d3d12.h>
#include <dxgi1_5.h>
#include "GraphicsMemory.h"

struct AppCtx;

namespace D3D12
{
	struct Fence
	{
		//Synchronization objects
		UINT m_frameIndex = 0;
		HANDLE m_fenceEvent = NULL;
		ID3D12Fence* m_fence = NULL;
		UINT64 m_fenceValue = 0;
	};

	const int MAX_CONSTANT_BUFFERS = 16;
	const int MAX_TEXTURE3D_BUFFERS = 4;
	const UINT SO_BUFFER_SIZE = 4608; //36 vertices * 32 Bytes * 4 cubes
	const int MAX_VERTEX_BUFFER_SIZE = 23040; //sizeof(Triangle) * 5 * nrOfVoxelsPerVolume;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		bool Init(AppCtx appCtx);
		void StartFrame();
		void EndFrame();
		void Present(Fence * fence);
		void Reset();

		Fence * MakeFence(UINT64 initialValue, UINT64 completionValue, D3D12_FENCE_FLAGS flag, const wchar_t * debugName = L"FenceObject");
		bool DestroyFence(Fence * fence);

		void CleanUp();

		void WaitForGPUCompletion(ID3D12CommandQueue * pCmdQ, Fence * fence);
	public:
		//List of Set functions
		void SetClearColor(float r, float g, float b, float a) { m_clearColor[0] = r, m_clearColor[1] = g; m_clearColor[2] = b; m_clearColor[3] = a; }

		//List of Get functions
		ID3D12Device * GetDevice() { return m_device; }
		ID3D12GraphicsCommandList * GetCommandList() { return m_gCmdList; }
		ID3D12CommandQueue * GetCommandQueue() { return m_directQ; }
		ID3D12RootSignature * GetRootSignature() { return m_rootSignature; }
		ID3D12DescriptorHeap * GetCBVHeap() { return m_CBVHeap; }
		ID3D12DescriptorHeap * GetUAVHeap() { return m_UAVHeap; }
		ID3D12DescriptorHeap * GetSRVHeap() { return m_SRVHeap; }

		ID3D12GraphicsCommandList * GetComputeCmdList() { return m_computeCmdList; }
		ID3D12CommandQueue * GetComputeCmdQueue() { return m_computeQ; }
		ID3D12CommandAllocator * GetComputeAllocator() { return m_computeAllocator; }

	private:
		IDXGIAdapter1 * _findDX12Adapter(IDXGIFactory5 ** ppFactory);
		HRESULT _createDevice(IDXGIFactory5 ** ppFactory, IDXGIAdapter1 ** ppAdapter);
		HRESULT _createCommandQueues();
		HRESULT _createCmdAllocatorsAndLists();
		HRESULT _createSwapChain(HWND hwnd, UINT width, UINT height, IDXGIFactory5 ** ppFactory);
		HRESULT _createHeapsAndResources();
		HRESULT _createRootSignature();
		HRESULT _createDepthBuffer(float width, float height);


	private:
		unsigned int BUFFER_COUNT = 2;
		bool initialized = false;
															    
		ID3D12Debug* m_debugController = NULL;									    
		ID3D12Device* m_device = NULL;
		IDXGISwapChain3* m_swapChain = NULL;

		ID3D12CommandQueue * m_directQ = NULL;
		ID3D12CommandAllocator * m_directAllocator = NULL;
		ID3D12GraphicsCommandList * m_gCmdList = NULL; 

		ID3D12CommandQueue * m_computeQ = NULL;
		ID3D12CommandAllocator * m_computeAllocator = NULL;
		ID3D12GraphicsCommandList * m_computeCmdList = NULL;

		ID3D12Resource** m_renderTargets = NULL;
		ID3D12DescriptorHeap* m_rtvHeap = NULL;

		ID3D12Resource * m_depthBuffer = NULL;
		ID3D12DescriptorHeap * m_depthHeap = NULL;

		ID3D12DescriptorHeap * m_CBVHeap = NULL;
		ID3D12DescriptorHeap * m_UAVHeap = NULL;
		ID3D12DescriptorHeap * m_SRVHeap = NULL;

		ID3D12RootSignature* m_rootSignature = NULL;

		float m_clearColor[4] = { 0.f, 0.4f, 0.2f, 1.f };

		D3D12_VIEWPORT m_viewPort = {};
		D3D12_RECT m_scissorRect = {};

		UINT m_frameIndex = 0;
	};
}

