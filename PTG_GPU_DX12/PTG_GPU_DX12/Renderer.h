#pragma once
#include <d3d12.h>
#include <dxgi1_5.h>
#include "GraphicsMemory.h"

struct AppCtx;

//Gloabal constants
const UINT HEAP_CONSTANT_BUFFER_OFFSET = 0U;
const UINT HEAP_MAX_CONSTANT_BUFFERS = 16U;
const UINT HEAP_UAV_OFFSET = HEAP_CONSTANT_BUFFER_OFFSET + HEAP_MAX_CONSTANT_BUFFERS;
const UINT HEAP_MAX_UAV = 16U;
const UINT HEAP_SRV_OFFSET = HEAP_UAV_OFFSET + HEAP_MAX_UAV;
const UINT HEAP_MAX_SRV = 16U;
const UINT HEAP_DESCRIPTOR_COUNT = HEAP_MAX_CONSTANT_BUFFERS + HEAP_MAX_UAV + HEAP_MAX_SRV;

const UINT VERTEX_BYTE_STRIDE = sizeof(float) * 6;
const UINT DENSITY_THREAD_GROUPS_X = 12U; //22 Thread groups * 10 is the absolute maximum because of the vertex buffer
const UINT DENSITY_THREAD_GROUPS_Y = 2U;
const UINT DENSITY_THREAD_GROUPS_Z =	12U;
//UINT CHUNK_THREAD_GROUPS;
const UINT NUM_THREADS_PER_GROUP = 8U;
const UINT DENSITY_VALUES = DENSITY_THREAD_GROUPS_X * NUM_THREADS_PER_GROUP * DENSITY_THREAD_GROUPS_Y * NUM_THREADS_PER_GROUP * DENSITY_THREAD_GROUPS_Z * NUM_THREADS_PER_GROUP;
const UINT MAX_VERTEX_BUFFER_SIZE = 72*5*DENSITY_VALUES;

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


	class Renderer
	{
	public:
		static UINT CONSTANT_BUFFER_COUNT;
		static UINT UAV_COUNT;
		static UINT SRV_COUNT;

		Renderer();
		~Renderer();

		bool Init(AppCtx appCtx);
		void StartFrame();
		void EndFrame();
		void RenderFrame(Fence* fence);
		void Present();
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
		ID3D12CommandAllocator * GetDirectAllocator() { return m_directAllocator; }

		ID3D12RootSignature * GetRootSignature() { return m_rootSignature; }
		ID3D12DescriptorHeap * GetDescriptorHeap() { return m_DescHeap; }

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

		ID3D12DescriptorHeap * m_DescHeap = NULL;

		ID3D12RootSignature* m_rootSignature = NULL;

		float m_clearColor[4] = { 0.f, 0.4f, 0.2f, 1.f };

		D3D12_VIEWPORT m_viewPort = {};
		D3D12_RECT m_scissorRect = {};

		UINT m_frameIndex = 0;
	};
}

