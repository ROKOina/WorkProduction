#pragma once

#include <DirectXMath.h>

// レンダーコンテキスト
struct RenderContext
{
	DirectX::XMFLOAT4X4		view;
	DirectX::XMFLOAT4X4		projection;
	DirectX::XMFLOAT4		lightDirection;
	DirectX::XMFLOAT4		viewPosition;
};
