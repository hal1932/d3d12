#pragma once
#include <lib.h>

__declspec(align(256))
struct CameraConstant
{
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Proj;
};
