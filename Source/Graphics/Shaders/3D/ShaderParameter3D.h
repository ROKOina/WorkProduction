#pragma once

#include <DirectXMath.h>
#include <wrl.h>

//	�V���h�E�}�b�v�p���
struct ShadowMapData
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSrvMap;				
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDsvMap;				
	UINT width;
	UINT height;
	DirectX::XMFLOAT4X4			lightViewProjection;	//	���C�g�r���[�v���W�F�N�V�����s��
	DirectX::XMFLOAT3			shadowColor;			//	�e�̐F
	float						shadowBias;				//	�[�x��r�p�̃I�t�Z�b�g�l
	float						shadowRect;				//	�V���h�E�}�b�v�ɕ`�悷��͈�
};

// �R���X�^���g�o�b�t�@�Ɏg���p�����[�^�[
struct ShaderParameter3D
{
	//���_
	DirectX::XMFLOAT4X4		view;
	DirectX::XMFLOAT4X4		projection;
	DirectX::XMFLOAT4		viewPosition;

	//���C�g
	DirectX::XMFLOAT4		lightDirection = { 0.3f,-0.7f,0,0 };
	DirectX::XMFLOAT4		lightColor = { 1,1,1,1 };

	//�e
	struct ShadowMapData shadowMapData;
};


