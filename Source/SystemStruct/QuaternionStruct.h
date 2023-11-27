#pragma once
#include <DirectXMath.h>
#include <cmath>

struct QuaternionStruct
{
	QuaternionStruct() :dxFloat4({ 0,0,0,1 }) {}
    QuaternionStruct(DirectX::XMFLOAT4 q) :
        QuaternionStruct(q.x, q.y, q.z, q.w) {}
	QuaternionStruct(float x, float y, float z, float w) :
		x(x), y(y), z(z), w(w), dxFloat4({ x,y,z,w }) {}

	//directX�ɂ��Ή��\�ɂ���
	union {
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};
		DirectX::XMFLOAT4 dxFloat4 = {};
	};


	operator DirectX::XMFLOAT4() const {
		return dxFloat4;
	}

	//�w�肵�������̃N�H�[�^�j�I����Ԃ�
	static QuaternionStruct LookRotation(const DirectX::XMFLOAT3 direction, const DirectX::XMFLOAT3 up = {0,1,0})
    {
		//�������W�̏ꍇ�A0�ŕԂ�
		if (direction.x == 0 && direction.y == 0 && direction.z == 0) return { 0,0,0,0 };

		//�����A���������r���[�s����쐬
		DirectX::XMVECTOR Direction = DirectX::XMLoadFloat3(&direction);
		DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);

		DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH({ 0,0,0 }, Direction, Up);

		//�r���[���t�s�񉻂��A���[���h���W�ɖ߂�
		DirectX::XMMATRIX World = DirectX::XMMatrixInverse(nullptr, View);
		if (!std::isfinite(World.r->m128_f32[0])) return {0,0,0,0};

		DirectX::XMFLOAT4X4 transform;
		DirectX::XMStoreFloat4x4(&transform, World);

		//�s��[���N�H�[�^�j�I���ɕϊ�
		QuaternionStruct rotation;
		DirectX::XMStoreFloat4(&rotation.dxFloat4,
			DirectX::XMQuaternionRotationMatrix(DirectX::XMLoadFloat4x4(&transform)));

		return rotation;
    }

};