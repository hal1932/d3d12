#pragma once

#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include <wrl/client.h>
#include <tchar.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "d3dx12.h"

#include <vector>
#include <string>
#include <memory>
#include <map>

using Microsoft::WRL::ComPtr;
