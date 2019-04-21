#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	const DirectX::XMMATRIX& View() { return view_; }
	const DirectX::XMMATRIX& Proj() { return proj_; }

	void SetPosition(DirectX::FXMVECTOR position) { position_ = position; }
	void SetFocus(DirectX::FXMVECTOR focus) { focus_ = focus; }
	void SetUp(DirectX::FXMVECTOR up) { up_ = up; }

	void SetFovY(float fov) { fovY_ = fov; }
	void SetAspect(float aspect) { aspect_ = aspect; }
	void SetNearPlane(float zNear) { near_ = zNear; }
	void SetFarPlane(float zFar) { far_ = zFar; }

	void UpdateMatrix()
	{
		view_ = DirectX::XMMatrixLookAtLH(position_, focus_, up_);
		proj_ = DirectX::XMMatrixPerspectiveFovLH(fovY_, aspect_, near_, far_);
	}

private:
	DirectX::XMMATRIX view_;
	DirectX::XMMATRIX proj_;

	DirectX::XMVECTOR position_;
	DirectX::XMVECTOR focus_;
	DirectX::XMVECTOR up_;

	float fovY_;
	float aspect_;
	float near_;
	float far_;
};

