#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

// �X�v���C�g
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

	// �`����s
	void Render(ID3D11DeviceContext *dc,
		float dx, float dy,
		float dw, float dh,
		float sx, float sy,
		float sw, float sh,
		float angle,
		float r, float g, float b, float a) const;

	// �e�N�X�`�����擾
	int GetTextureWidth() const { return textureWidth_; }

	// �e�N�X�`�������擾
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