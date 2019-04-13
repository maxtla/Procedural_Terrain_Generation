#pragma once
#include <d3d12.h>
#include <dxgi1_5.h>


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
		Renderer();
		~Renderer();

		bool Init(HWND hwnd, UINT width, UINT height);
		void StartFrame();
		void EndFrame();
		void Present(Fence * fence);

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

	private:
		IDXGIAdapter1 * _findDX12Adapter(IDXGIFactory5 ** ppFactory);
		HRESULT _createDevice(IDXGIFactory5 ** ppFactory, IDXGIAdapter1 ** ppAdapter);
		HRESULT _createCommandQueues();
		HRESULT _createCmdAllocatorsAndLists();
		HRESULT _createSwapChain(HWND hwnd, UINT width, UINT height, IDXGIFactory5 ** ppFactory);
		HRESULT _createHeapsAndResources();
		HRESULT _createRootSignature();


	private:
		static const unsigned int BUFFER_COUNT = 2;
		bool initialized = false;
															    
		ID3D12Debug* m_debugController = NULL;									    
		ID3D12Device* m_device = NULL;
		IDXGISwapChain3* m_swapChain = NULL;

		ID3D12CommandQueue * m_directQ = NULL;
		ID3D12CommandAllocator * m_directAllocator = NULL;
		ID3D12GraphicsCommandList * m_gCmdList = NULL; 

		ID3D12Resource* m_renderTargets[BUFFER_COUNT] = { NULL, NULL };
		ID3D12DescriptorHeap* m_rtvHeap = NULL;

		ID3D12RootSignature* m_rootSignature = NULL;

		float m_clearColor[4] = { 0.f, 0.4f, 0.2f, 1.f };
	};
}

