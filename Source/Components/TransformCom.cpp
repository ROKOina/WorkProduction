#include <imgui.h>
#include "TransformCom.h"
#include <cmath>

// 開始処理
void TransformCom::Start()
{

}

// 更新処理
void TransformCom::Update(float elapsedTime)
{

}

// GUI描画
void TransformCom::OnGUI()
{
	// トランスフォーム
	ImGui::DragFloat3("Position", &position_.x, 0.1f);
	ImGui::DragFloat3("WorldPosition", &worldPosition_.x, 0.1f);

	ImGui::DragFloat4("Rotation", &rotation_.x, 0.1f);

	{	//オイラー角
		DirectX::XMFLOAT3 euler = eulerRotation_;
		if (ImGui::DragFloat3("EulerRotato", &euler.x)) {
			SetEulerRotation(euler);
		}
	}

	ImGui::DragFloat3("Scale", &scale_.x, 0.1f);
}

//行列更新
void TransformCom::UpdateTransform()
{
	// ワールド行列の更新
	DirectX::XMVECTOR Q = DirectX::XMLoadFloat4(&rotation_);
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale_.x, scale_.y, scale_.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(Q);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);

	DirectX::XMMATRIX L = DirectX::XMLoadFloat4x4(&parentTransform_);
	DirectX::XMMATRIX W = S * R * T * L;

	DirectX::XMStoreFloat4x4(&transform_, W);

	worldPosition_ = { transform_._41,transform_._42,transform_._43 };


}

//指定方向を向く
void TransformCom::LookAtTransform(const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up)
{
	UpdateTransform();

	//位置、注視点、上方向からビュー行列を作成
	DirectX::XMVECTOR Eye = DirectX::XMLoadFloat3(&worldPosition_);
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
	DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);

	//同じ座標の場合、少しずらす
	DirectX::XMFLOAT3 wp = worldPosition_;
	if (focus.x == wp.x && focus.y == wp.y && focus.z == wp.z)
	{
		wp.y += 0.0001f;
		Eye = DirectX::XMLoadFloat3(&wp);
	}

	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);

	//ビューを逆行列化し、ワールド座標に戻す
	DirectX::XMMATRIX World = DirectX::XMMatrixInverse(nullptr, View);
	if (!std::isfinite(World.r->m128_f32[0]))return;

	DirectX::XMStoreFloat4x4(&transform_, World);

	//回転を適用
	DirectX::XMStoreFloat4(&rotation_,
		DirectX::XMQuaternionRotationMatrix(DirectX::XMLoadFloat4x4(&transform_)));
}

//指定のUpに合わせる
void TransformCom::SetUpTransform(const DirectX::XMFLOAT3& up)
{
	DirectX::XMFLOAT3 front = GetFront();
	DirectX::XMFLOAT3 right;
	DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);
	DirectX::XMVECTOR Front = DirectX::XMLoadFloat3(&front);
	DirectX::XMVECTOR Right;

	//upを基準に
	Right = DirectX::XMVector3Cross(Up, Front);
	Front = DirectX::XMVector3Cross(Right, Up);
	
	DirectX::XMStoreFloat3(&DirectX::XMFLOAT3(up), Up);
	DirectX::XMStoreFloat3(&front, Front);
	DirectX::XMStoreFloat3(&right, Right);

	//upを基準に行列を作る
	DirectX::XMFLOAT4X4 matrixUp = {
		right.x, right.y, right.z, 0,
		up.x, up.y, up.z, 0,
		front.x, front.y, front.z, 0,
		0, 0, 0, 1 };
	DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&matrixUp);

	//回転を適用
	DirectX::XMStoreFloat4(&rotation_, DirectX::XMQuaternionRotationMatrix(M));
}