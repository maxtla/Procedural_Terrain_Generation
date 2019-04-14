#pragma once

struct ID3D12GraphicsCommandList;

class Drawable
{
public:
	virtual ~Drawable() {}
	virtual void Update(float& dt) = 0;
	virtual void Draw(ID3D12GraphicsCommandList * pCommandList) = 0;
};