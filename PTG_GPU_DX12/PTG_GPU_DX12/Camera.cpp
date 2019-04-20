#include "pch.h"
#include "Camera.h"


Camera::Camera()
{
	m_viewProj.m_view =  DirectX::XMMatrixLookAtLH(m_pos, m_dir, DefaultUp);
}


Camera::~Camera()
{
}

void Camera::Update(float& dt)
{
	ImGuiUpdate();

	if (!SDL_GetRelativeMouseMode())
		return;

	int numKeys;
	const Uint8 * keyStates = SDL_GetKeyboardState(&numKeys);

	int dx, dy;
	Uint32 mbStates = SDL_GetMouseState(&dx, &dy);

	float speed = m_ms * dt;

	if (keyStates[SDL_SCANCODE_A])
		m_moveLeftRight -= speed;
	if (keyStates[SDL_SCANCODE_D])
		m_moveLeftRight += speed;
	if (keyStates[SDL_SCANCODE_W])
		m_moveBackForward += speed;
	if (keyStates[SDL_SCANCODE_S])
		m_moveBackForward -= speed;
	if (keyStates[SDL_SCANCODE_UP])
		m_moveUpDown += speed;
	if (keyStates[SDL_SCANCODE_DOWN])
		m_moveUpDown -= speed;

	m_yaw += (dx * m_rs);
	m_pitch += (dy * m_rs);

	//Update the camera
	{
		using namespace DirectX;

		m_dir += XMVectorSet(m_moveLeftRight, m_moveUpDown, m_moveBackForward, 1.f);
		m_pos += XMVectorSet(m_moveLeftRight, m_moveUpDown, m_moveBackForward, 1.f);

		m_viewProj.m_view = XMMatrixLookAtLH(m_pos, m_dir, DefaultUp);

		m_yaw = m_pitch = 0.f;
		m_moveUpDown = m_moveLeftRight = m_moveBackForward = 0.f;
	}
}

void Camera::Draw(ID3D12GraphicsCommandList * pCommandList)
{
	m_viewProj.m_view = DirectX::XMMatrixTranspose(m_viewProj.m_view);
	m_viewProj.m_projection = DirectX::XMMatrixTranspose(m_viewProj.m_projection);

	pCommandList->SetGraphicsRoot32BitConstants(0, 32, (void*)&m_viewProj, 0);
}

void Camera::ImGuiUpdate()
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMStoreFloat3(&pos, m_pos);

	DirectX::XMFLOAT3 target;
	DirectX::XMStoreFloat3(&target, m_dir);

	std::stringstream ss;

	ss << "Position:\n X: " << pos.x << " Y: " << pos.y << " Z: " << pos.z;
	ss << "\n\nDirection:\n X: " << target.x << " Y: " << target.y << " Z: " << target.z;

	ImGui::Begin("Camera");
	ImGui::Text(ss.str().c_str());
	ImGui::End();
}
