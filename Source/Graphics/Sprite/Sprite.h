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
		float r, float g, float b, float a);

	// �e�N�X�`�����擾
	int GetTextureWidth() const { return textureWidth_; }

	// �e�N�X�`�������擾
	int GetTextureHeight() const { return textureHeight_; }

	//�e�N�X�`���ύX
	void SetShaderResourceView(ID3D11ShaderResourceView* srv,float textureWidth,float textureHeight);

	//�f�B�]���u�p�摜�o�^
	void SetDissolveSRV(const char* filename);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexShader_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelShader_;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			inputLayout_;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBuffer_;


	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView_;

public:
	//���ʈꗗ
	struct DissolveConstans
	{
		float isDissolve = false;

		float dissolveThreshold= 0.5f;	// �f�B�]���u
		float edgeThreshold=0.5f; 		// �ӂ���臒l
		float dummy;
		DirectX::XMFLOAT4	edgeColor{1,0,0,1};			// �ӂ��̐F
	};
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	dissolveSRV_;

	//���ʂ܂Ƃ߂�
	struct Effect2DConstans
	{
		DissolveConstans dissolveConstant;
	};
	Effect2DConstans effectConstans_;

	Effect2DConstans& GetEffectSpriteData() { return effectConstans_; }

private:
	//���ʂ��|���鎞�Ɏg�p����o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer>				effectBuffer_;

	int textureWidth_ = 0;
	int textureHeight_ = 0;
};