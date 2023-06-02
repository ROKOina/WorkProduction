#include "CameraCom.h"

#include <imgui.h>

// 更新処理
void CameraCom::Update(float elapsedTime)
{
    DirectX::XMFLOAT3 p= GameObjectManager::Instance().Find("pico")->GetPosition();
    SetLookAt(p, {0,1,0});
}

// GUI描画
void CameraCom::OnGUI()
{
    ImGui::DragFloat3("Focus", &focus.x);
}

//指定方向を向く
void CameraCom::SetLookAt(const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up)
{
    //視点、注視点、上方向からビュー行列を作成
    DirectX::XMFLOAT3 cameraPos = GetGameObject()->GetWorldPosition();
    DirectX::XMVECTOR Eye = DirectX::XMLoadFloat3(&cameraPos);
    DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
    DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);
    if (focus.x == cameraPos.x && focus.y == cameraPos.y && focus.z == cameraPos.z)
    {
        cameraPos.y += 0.0001f;
        Eye = DirectX::XMLoadFloat3(&cameraPos);
    }
    DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
    DirectX::XMStoreFloat4x4(&view, View);

    //ビューを逆行列化し、ワールド座標に戻す
    DirectX::XMMATRIX World = DirectX::XMMatrixInverse(nullptr, View);
    DirectX::XMFLOAT4X4 world;
    DirectX::XMStoreFloat4x4(&world, World);

    //カメラの方向を取り出す
    this->right.x   = world._11;
    this->right.y   = world._12;
    this->right.z   = world._13;

    this->up.x      = world._21;
    this->up.y      = world._22;
    this->up.z      = world._23;

    this->front.x   = world._31;
    this->front.y   = world._32;
    this->front.z   = world._33;

    //視点、注視点を保存
    this->focus = focus;
}

//パースペクティブ設定
void CameraCom::SetPerspectiveFov(float fovY, float aspect, float nearZ, float farZ)
{
    //画角、画面比率、クリップ距離からプロジェクション行列を作成
    DirectX::XMMATRIX Projection = DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);	//プロジェクション行列作成
    DirectX::XMStoreFloat4x4(&projection, Projection);	//rcに渡す
}