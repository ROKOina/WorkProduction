#include "CameraCom.h"

#include "TransformCom.h"
#include "Graphics/Graphics.h"
#include "GameSource/Math/Mathf.h"
#include <imgui.h>
#include <cmath>

// 開始処理
void CameraCom::Start()
{

}


// 更新処理
void CameraCom::Update(float elapsedTime)
{
    //カメラシェイク
    if (shakeSec_ > 0)
    {
        shakeSec_ -= elapsedTime;

        DirectX::XMFLOAT3 upDir = GetGameObject()->transform_->GetWorldUp();
        DirectX::XMFLOAT3 rightDir = GetGameObject()->transform_->GetWorldRight();

        float random = Mathf::RandomRange(-1, 1) * shakePower_;
        upDir = { upDir.x * random,upDir.y * random,upDir.z * random };
        random = Mathf::RandomRange(-1, 1) * shakePower_;
        rightDir = { rightDir.x * random,rightDir.y * random,rightDir.z * random };

        shakePos_ = { upDir.x + rightDir.x,upDir.y + rightDir.y,upDir.z + rightDir.z };
    }
    else
    {
        shakePos_ = { 0,0,0 };
    }

    //ヒットストップ
    if (isHitStop_)
    {
        if (hitTimer_ > 0)
        {
            hitTimer_ -= elapsedTime;
            Graphics::Instance().SetWorldSpeed(0);
        }
        else
        {
            isHitStop_ = false;
            Graphics::Instance().SetWorldSpeed(saveWorldSpeed_);
        }
    }

    //LookAt関数使っていないなら更新する
    if (!isLookAt_) {
        //カメラのフォワードをフォーカスする
        DirectX::XMFLOAT3 forwardPoint;
        DirectX::XMFLOAT3 wPos = GetGameObject()->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 forwardNormalVec = GetGameObject()->transform_->GetWorldFront();
        forwardPoint = { forwardNormalVec.x * 2 + wPos.x + shakePos_.x,
            forwardNormalVec.y * 2 + wPos.y + shakePos_.y,
            forwardNormalVec.z * 2 + wPos.z + shakePos_.z };

        SetLookAt(forwardPoint, GetGameObject()->transform_->GetWorldUp());
    }

    isLookAt_ = false;
}

// GUI描画
void CameraCom::OnGUI()
{
    ImGui::DragFloat3("Focus", &focus_.x);

    ImGui::DragFloat3("shakePos_", &shakePos_.x);
    ImGui::DragFloat("shakeSec", &shakeSec_);
    ImGui::DragFloat("shakePower", &shakePower_);
}

//指定方向を向く
void CameraCom::SetLookAt(const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up)
{
    //視点、注視点、上方向からビュー行列を作成
    DirectX::XMFLOAT3 cameraPos = GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMVECTOR Eye = DirectX::XMLoadFloat3(&cameraPos);
    DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
    DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);

    //同じ座標の場合、少しずらす
    if (focus.x == cameraPos.x && focus.y == cameraPos.y && focus.z == cameraPos.z)
    {
        cameraPos.y += 0.0001f;
        Eye = DirectX::XMLoadFloat3(&cameraPos);
    }

    DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
    DirectX::XMStoreFloat4x4(&view_, View);

    //ビューを逆行列化し、ワールド座標に戻す
    DirectX::XMMATRIX World = DirectX::XMMatrixInverse(nullptr, View);
    DirectX::XMFLOAT4X4 world;
    DirectX::XMStoreFloat4x4(&world, World);

    if (!std::isfinite(world._11))return;

    //カメラの方向を取り出す
    this->right_.x   = world._11;
    this->right_.y   = world._12;
    this->right_.z   = world._13;

    this->up_.x      = world._21;
    this->up_.y      = world._22;
    this->up_.z      = world._23;

    this->front_.x   = world._31;
    this->front_.y   = world._32;
    this->front_.z   = world._33;

    GetGameObject()->transform_->SetWorldTransform(world);


    //視点、注視点を保存
    this->focus_ = focus;

    isLookAt_ = true;
}

//パースペクティブ設定
void CameraCom::SetPerspectiveFov(float fovY, float aspect, float nearZ, float farZ)
{
    //画角、画面比率、クリップ距離からプロジェクション行列を作成
    DirectX::XMMATRIX Projection = DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);	//プロジェクション行列作成
    DirectX::XMStoreFloat4x4(&projection_, Projection);	//rcに渡す
}

//ヒットストップ
void CameraCom::HitStop(float sec)
{
    if (!isHitStop_)
        saveWorldSpeed_ = Graphics::Instance().GetWorldSpeed();
    isHitStop_ = true;
    hitTimer_ = sec;
}