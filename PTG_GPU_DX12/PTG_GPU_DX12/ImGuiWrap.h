#pragma once
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <DescriptorHeap.h>

struct ImGuiWrap
{
	ImGuiWrap(){}
	~ImGuiWrap(){}

	DirectX::DescriptorHeap * imgui_DescHeap = nullptr;
	ImGuiIO imgui_io;

	inline void Initialize_IMGUI(UINT width, UINT height, HWND hwnd, ID3D12Device * dev)
	{
		if (!imgui_DescHeap)
			imgui_DescHeap = new DirectX::DescriptorHeap(dev, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1);

		if (imgui_DescHeap)
		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			imgui_io = ImGui::GetIO();
			ImGui::StyleColorsClassic();
			// Build atlas
			unsigned char* tex_pixels = NULL;
			int tex_w, tex_h;
			imgui_io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);
			imgui_io.DisplaySize = ImVec2(width, height);
			imgui_io.DeltaTime = 1.0f / 60.0f;

			ImGui_ImplWin32_Init(hwnd);
			ImGui_ImplDX12_Init(dev,
				2,
				DXGI_FORMAT_R8G8B8A8_UNORM,
				imgui_DescHeap->GetFirstCpuHandle(),
				imgui_DescHeap->GetFirstGpuHandle());
		}
	}

	inline void Frame_IMGUI()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	inline void Render_IMGUI(ID3D12GraphicsCommandList * pCmdList)
	{
		auto heap = imgui_DescHeap->Heap();
		pCmdList->SetDescriptorHeaps(1, (ID3D12DescriptorHeap* const*)&heap);
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCmdList);
	}

	inline void Destroy_IMGUI()
	{
		if (imgui_DescHeap)
		{
			delete imgui_DescHeap;
			imgui_DescHeap = nullptr;
		}
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
};