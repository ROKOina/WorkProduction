#pragma once

#include <vector>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

class LineRenderer
{
public:
	LineRenderer(ID3D11Device* device, UINT vertexCount);
	~LineRenderer() {}

public:
	// ï`âÊé¿çs
	void Render(ID3D11DeviceContext* context, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

	// í∏ì_í«â¡
	void AddVertex(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color);

private:
	struct ConstantBuffer
	{
		DirectX::XMFLOAT4X4	wvp;
	};

	struct Vertex
	{
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT4	color;
	};


	Microsoft::WRL::ComPtr<ID3D11Buffer>			vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			constantBuffer_;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>		vertexShader_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		pixelShader_;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>		inputLayout_;

	std::vector<Vertex>			vertices_;
	UINT						capacity_ = 0;
};
