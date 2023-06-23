#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

class TextureFormat
{
public:
	TextureFormat() :TextureFormat(nullptr) {}
    TextureFormat(const char* filename);
    ~TextureFormat() {}

	struct Vertex
	{
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT2	texcoord;
	};


	// バッファ更新
	void Update(
		float dx, float dy,
		float dw, float dh,
		float sx, float sy,
		float sw, float sh,
		float angle,
		float r, float g, float b, float a) const;

	// シェーダーリソースビューの設定
	void SetShaderResourceView(const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv, int width, int height);

	// 頂点バッファの取得
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetVertexBuffer() const { return vertexBuffer_; }

	// シェーダーリソースビューの取得
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetShaderResourceView() const { return shaderResourceView_; }

	// テクスチャ幅取得
	int GetTextureWidth() const { return textureWidth_; }

	// テクスチャ高さ取得
	int GetTextureHeight() const { return textureHeight_; }

protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView_;

	int textureWidth_ = 0;
	int textureHeight_ = 0;
};