#include "pch.h"
#include "Camera.h"

extern AppCtx gAppCtx;

Camera::Camera()
{
	m_viewProj.m_view = DirectX::XMMatrixTranspose( DirectX::XMMatrixLookAtLH(m_pos, m_target, DefaultUp) );
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

	float speed = gAppCtx.camSettings.moveSpeed * dt;

	//Movement
	if (keyStates[SDL_SCANCODE_A])
		m_moveLeftRight -= speed;
	if (keyStates[SDL_SCANCODE_D])
		m_moveLeftRight += speed;
	if (keyStates[SDL_SCANCODE_W])
		m_moveBackForward -= speed;
	if (keyStates[SDL_SCANCODE_S])
		m_moveBackForward += speed;
	if (keyStates[SDL_SCANCODE_UP])
		m_moveUpDown += speed;
	if (keyStates[SDL_SCANCODE_DOWN])
		m_moveUpDown -= speed;

	//Rotation
	m_yaw += (gAppCtx.mx * m_rs);
	m_pitch += (gAppCtx.my * m_rs);
	m_pitch = std::fmax(-0.5f*DirectX::XM_PI, std::fmin(m_pitch, 0.5f*DirectX::XM_PI) ); //clamp to pi/2
	gAppCtx.mx = gAppCtx.my = 0;

	//Update the camera
	{
		using namespace DirectX;

		auto dir = XMVector3Normalize(m_pos - m_target);
		auto right = XMVector3Normalize(XMVector3Cross(dir, DefaultUp));

		m_pos += (dir * m_moveBackForward);
		m_pos += (right * m_moveLeftRight);
		m_pos += (DefaultUp * m_moveUpDown);

		m_target += (dir * m_moveBackForward);
		m_target += (right * m_moveLeftRight);
		m_target += (DefaultUp * m_moveUpDown);

		auto rotM = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.f);

		dir = XMVector3Normalize(XMVector3TransformCoord(DefaultForward, rotM));
		right = XMVector3Normalize(XMVector3TransformCoord(DefaultRight, rotM));
		auto up = XMVector3Normalize(XMVector3Cross(dir, right));
		
		m_target = m_pos + dir;

		m_viewProj.m_view = XMMatrixTranspose(XMMatrixLookAtLH(m_pos, m_target, up));

		m_rotLeftRight = m_moveUpDown = m_moveLeftRight = m_moveBackForward = 0.f;
	}
}

void Camera::Draw(ID3D12GraphicsCommandList * pCommandList)
{
	pCommandList->SetGraphicsRoot32BitConstants(0, 32, (const void*)&m_viewProj, 0);
}

void Camera::ImGuiUpdate()
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMStoreFloat3(&pos, m_pos);

	DirectX::XMFLOAT3 target;
	DirectX::XMStoreFloat3(&target, m_target);

	std::stringstream ss;

	ss << "Position:\n X: " << pos.x << " Y: " << pos.y << " Z: " << pos.z;
	ss << "\n\nDirection:\n X: " << target.x << " Y: " << target.y << " Z: " << target.z;
	ss << "\n\nYaw: " << m_yaw << " Pitch: " << m_pitch;

	ImGui::Begin("Camera");
	ImGui::Text(ss.str().c_str());
	ImGui::End();
}
