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


	// �o�b�t�@�X�V
	void Update(
		float dx, float dy,
		float dw, float dh,
		float sx, float sy,
		float sw, float sh,
		float angle,
		float r, float g, float b, float a) const;

	// �V�F�[�_�[���\�[�X�r���[�̐ݒ�
	void SetShaderResourceView(const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv, int width, int height);

	// ���_�o�b�t�@�̎擾
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetVertexBuffer() const { return vertexBuffer_; }

	// �V�F�[�_�[���\�[�X�r���[�̎擾
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetShaderResourceView() const { return shaderResourceView_; }

	// �e�N�X�`�����擾
	int GetTextureWidth() const { return textureWidth_; }

	// �e�N�X�`�������擾
	int GetTextureHeight() const { return textureHeight_; }

protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView_;

	int textureWidth_ = 0;
	int textureHeight_ = 0;
};