#pragma once
#include "Drawable.h"
#include <DirectXMath.h>

class Camera : public Drawable
{
public:
	Camera();
	~Camera();

	void Update(float& dt);
	void Draw(ID3D12GraphicsCommandList * pCommandList);

	void SetWorldPosition(float x, float y, float z) { m_pos = DirectX::XMVectorSet(x, y, z, 0.f); }
	void SetRotationalSpeed(float rs) { m_rs = rs; }
	void SetMoveSpeed(float ms) { m_ms = ms; }
	void SetProjectionMatrix(float fov, float aspect, float nearZ, float farZ) { m_viewProj.m_projection = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ); }

private:
	const DirectX::XMVECTOR DefaultForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	const DirectX::XMVECTOR DefaultRight		= DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	const DirectX::XMVECTOR DefaultUp			= DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMVECTOR m_forward					= DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DirectX::XMVECTOR m_right						= DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR m_up							= DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX m_camRotMatrix			= DirectX::XMMatrixIdentity();
	DirectX::XMVECTOR m_pos							= DirectX::XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f);
	DirectX::XMVECTOR m_dir							= DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);

	float m_moveLeftRight			= 0.f;
	float m_moveBackForward		= 0.f;
	float m_moveUpDown				= 0.f;

	float m_yaw		= 0.f;
	float m_pitch	= 0.f;

	float m_rs		= 0.001f;
	float m_ms	= 15.f;
	
	struct ViewProj
	{
		DirectX::XMMATRIX m_view;
		DirectX::XMMATRIX m_projection;
	} m_viewProj;

	void ImGuiUpdate();
};

