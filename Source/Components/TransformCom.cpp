#include <imgui.h>
#include "TransformCom.h"

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
	ImGui::DragFloat3("Position", &position.x, 0.1f);
	ImGui::DragFloat3("WorldPosition", &worldPosition.x, 0.1f);
	ImGui::DragFloat4("Rotation", &rotation.x, 0.1f);
	ImGui::DragFloat3("Scale", &scale.x, 0.1f);
}

//行列更新
void TransformCom::UpdateTransform()
{
	// ワールド行列の更新
	DirectX::XMVECTOR Q = DirectX::XMLoadFloat4(&rotation);
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(Q);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	DirectX::XMMATRIX L = DirectX::XMLoadFloat4x4(&parentTransform);
	DirectX::XMMATRIX W = S * R * T * L;

	DirectX::XMStoreFloat4x4(&transform, W);

	worldPosition = { transform._41,transform._42,transform._43 };
}
