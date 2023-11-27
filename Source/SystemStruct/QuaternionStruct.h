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

	//directXにも対応可能にする
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

	//指定した方向のクォータニオンを返す
	static QuaternionStruct LookRotation(const DirectX::XMFLOAT3 direction, const DirectX::XMFLOAT3 up = {0,1,0})
    {
		//同じ座標の場合、0で返す
		if (direction.x == 0 && direction.y == 0 && direction.z == 0) return { 0,0,0,0 };

		//方向、上方向からビュー行列を作成
		DirectX::XMVECTOR Direction = DirectX::XMLoadFloat3(&direction);
		DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);

		DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH({ 0,0,0 }, Direction, Up);

		//ビューを逆行列化し、ワールド座標に戻す
		DirectX::XMMATRIX World = DirectX::XMMatrixInverse(nullptr, View);
		if (!std::isfinite(World.r->m128_f32[0])) return {0,0,0,0};

		DirectX::XMFLOAT4X4 transform;
		DirectX::XMStoreFloat4x4(&transform, World);

		//行列ー＞クォータニオンに変換
		QuaternionStruct rotation;
		DirectX::XMStoreFloat4(&rotation.dxFloat4,
			DirectX::XMQuaternionRotationMatrix(DirectX::XMLoadFloat4x4(&transform)));

		return rotation;
    }

};