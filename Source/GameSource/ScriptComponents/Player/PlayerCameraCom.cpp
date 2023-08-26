#include "PlayerCameraCom.h"

#include <imgui.h>
#include "Components\TransformCom.h"
#include "Input/Input.h"

// 開始処理
void PlayerCameraCom::Start()
{
    angleX_ = 30;
}


// 更新処理
void PlayerCameraCom::Update(float elapsedTime)
{
    //カメラをフォーカスする
    DirectX::XMFLOAT3 focusPos = GetGameObject()->transform_->GetWorldPosition();
    focusPos.y += 1;
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");

    //入力情報を取得
    GamePad& gamePad = Input::Instance().GetGamePad();
    angleY_ += gamePad.GetAxisRX() * 180.0f * elapsedTime;
    angleX_ -= gamePad.GetAxisRY() * 100.0f * elapsedTime;


    //角度制限(縦のみ)
    if (angleX_ > 85) {
        angleX_ = 85;
    }
    if (angleX_ < -85) {
        angleX_ = -85;
    }

    //カメラ回転値を回転行列に変換
    DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(angleX_), DirectX::XMConvertToRadians(angleY_), 0);

    //回転行列から前方向ベクトルを取り出す
    DirectX::XMVECTOR Front = Transform.r[2];
    DirectX::XMVECTOR FocusPos = DirectX::XMLoadFloat3(&focusPos);
    DirectX::XMFLOAT3 cameraPos;
    //注視点から後ろベクトル方向に一定距離離れたカメラ視点を求める
    DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(FocusPos, DirectX::XMVectorScale(Front, range_)));

    //位置代入
    cameraObj->transform_->SetWorldPosition(cameraPos);
    cameraObj->transform_->LookAtTransform(focusPos);
}


// GUI描画
void PlayerCameraCom::OnGUI()
{
    ImGui::DragFloat("angleX", &angleX_, 0.1f);
    ImGui::DragFloat("angleY", &angleY_, 0.1f);
    ImGui::DragFloat("range", &range_, 0.1f);
}

