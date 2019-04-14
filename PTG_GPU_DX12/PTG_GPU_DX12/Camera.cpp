#include "pch.h"
#include "Camera.h"


Camera::Camera()
{
}


Camera::~Camera()
{
}

void Camera::Update(float& dt)
{
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

	m_yaw += (dx * m_rs);
	m_pitch += (dy * m_rs);

	//Update the camera
	{
		using namespace DirectX;
		m_camRotMatrix = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0);
		m_target = XMVector3TransformCoord(DefaultForward, m_camRotMatrix);
		m_target = XMVector3Normalize(m_target);

		XMMATRIX tempRotY;
		tempRotY = XMMatrixRotationY(m_yaw);

		m_right = XMVector3TransformCoord(DefaultRight, tempRotY);
		m_up = XMVector3TransformCoord(m_up, tempRotY); //might need to swap this with DefaultUp
		m_forward = XMVector3TransformCoord(DefaultForward, tempRotY);

		m_pos += (m_moveLeftRight * m_right);
		m_pos += (m_moveBackForward * m_forward);

		m_target = m_pos + m_target;

		m_viewProj.m_view = XMMatrixLookAtLH(m_pos, m_target, m_up);
	}
}

void Camera::Draw(ID3D12GraphicsCommandList * pCommandList)
{
	pCommandList->SetGraphicsRoot32BitConstants(0, 32, (void*)&m_viewProj, 0);
}
