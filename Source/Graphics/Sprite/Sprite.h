#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

// スプライト
class Sprite
{
public:
	Sprite();
	Sprite(const char* filename);
	~Sprite() {}

	struct Vertex
	{
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT2	texcoord;
	};

	// 描画実行
	void Render(ID3D11DeviceContext *dc,
		float dx, float dy,
		float dw, float dh,
		float sx, float sy,
		float sw, float sh,
		float angle,
		float r, float g, float b, float a) const;

	// テクスチャ幅取得
	int GetTextureWidth() const { return textureWidth_; }

	// テクスチャ高さ取得
	int GetTextureHeight() const { return textureHeight_; }

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexShader_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelShader_;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			inputLayout_;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBuffer_;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView_;

	int textureWidth_ = 0;
	int textureHeight_ = 0;
};