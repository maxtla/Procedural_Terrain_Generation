#pragma once
#include <d3d12.h>
#include <dxgi1_5.h>


namespace D3D12
{
	struct Fence
	{

	};

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		bool Init();
		void ExecuteCommandLists(ID3D12GraphicsCommandList ** ppCommandLists, Fence ** ppFences);
		void Present();

	private:

	};
}

