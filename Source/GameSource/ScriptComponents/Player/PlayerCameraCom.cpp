#include "PlayerCameraCom.h"

#include <imgui.h>
#include "Components\TransformCom.h"
#include "Input/Input.h"
#include "GameSource/Math/Collision.h"

// 開始処理
void PlayerCameraCom::Start()
{
    angleX_ = 30;
    oldCameraPos_= GetGameObject()->transform_->GetWorldPosition();
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
    if (angleX_ > 65) {
        angleX_ = 65;
    }
    if (angleX_ < -65) {
        angleX_ = -65;
    }

    //レンジを戻す
    range_ = 4;

    //カメラ回転値を回転行列に変換
    DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(angleX_), DirectX::XMConvertToRadians(angleY_), 0);

    //回転行列から前方向ベクトルを取り出す
    DirectX::XMVECTOR Front = Transform.r[2];
    DirectX::XMFLOAT3 front;
    DirectX::XMStoreFloat3(&front, Front);
    DirectX::XMVECTOR FocusPos = DirectX::XMLoadFloat3(&focusPos);

    DirectX::XMFLOAT3 cameraPos;
    //注視点から後ろベクトル方向に一定距離離れたカメラ視点を求める
    DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(FocusPos, DirectX::XMVectorScale(Front, range_)));

    // スムーズなカメラ移動
    DirectX::XMStoreFloat3(&oldCameraPos_, DirectX::XMVectorLerp(XMLoadFloat3(&cameraPos), DirectX::XMLoadFloat3(&oldCameraPos_), 1.0f - 0.08f));

    // 新しい目標の Target を計算
    DirectX::XMStoreFloat3(&focusPos, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&cameraPos), DirectX::XMLoadFloat3(&focusPos), 1.0f - 0.09f));

    //カメラめり込み回避処理
    DirectX::XMFLOAT3 kabePlus = GameObjectManager::Instance().Find("kabePlus")->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 kabeMinas = GameObjectManager::Instance().Find("kabeMinas")->transform_->GetWorldPosition();
    //補正
    kabePlus = { kabePlus.x + 1,0,kabePlus.z + 1 };
    kabeMinas = { kabeMinas.x - 1,0,kabeMinas.z - 1 };

    while (oldCameraPos_.y < 0 ||   //地面
        oldCameraPos_.x > kabePlus.x || oldCameraPos_.x < kabeMinas.x ||    //壁
        oldCameraPos_.z > kabePlus.z || oldCameraPos_.z < kabeMinas.z)
    {
        range_ -= 0.01f;
        DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(FocusPos, DirectX::XMVectorScale(Front, range_)));
        DirectX::XMStoreFloat3(&oldCameraPos_, DirectX::XMVectorLerp(XMLoadFloat3(&cameraPos), DirectX::XMLoadFloat3(&oldCameraPos_), 1.0f - 0.08f));

        if (range_ <= 0.1f)break;
    }

    //位置代入
    cameraObj->transform_->SetWorldPosition(oldCameraPos_);
    cameraObj->transform_->LookAtTransform(focusPos);
}


// GUI描画
void PlayerCameraCom::OnGUI()
{
    ImGui::DragFloat("angleX", &angleX_, 0.1f);
    ImGui::DragFloat("angleY", &angleY_, 0.1f);
    ImGui::DragFloat("range", &range_, 0.1f);
}

