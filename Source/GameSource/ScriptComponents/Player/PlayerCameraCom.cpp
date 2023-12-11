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
    lerpFocusPos_ = GetGameObject()->transform_->GetWorldPosition();
}


// 更新処理
void PlayerCameraCom::Update(float elapsedTime)
{
    if (isJust_)
    {
        //カメラに値を代入
        std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");
        cameraObj->transform_->SetWorldPosition(justPos_);
        oldCameraPos_ = justPos_;
        cameraObj->transform_->LookAtTransform(lerpFocusPos_);
        return;
    }

    if (rangeTimer_ > 0)
    {
        rangeTimer_ -= elapsedTime;

        range_ = rangeDir_;
        if (rangeTimer_ <= 0)range_ = DefaultRange;
    }

    //入力情報を取得
    GamePad& gamePad = Input::Instance().GetGamePad();
    angleY_ += gamePad.GetAxisRX() * 180.0f * elapsedTime;
    angleX_ -= gamePad.GetAxisRY() * 100.0f * elapsedTime;


    //角度制限(縦のみ)
    if (angleX_ > angleLimit_) {
        angleX_ = angleLimit_;
    }
    if (angleX_ < -angleLimit_) {
        angleX_ = -angleLimit_;
    }

    //カメラをプレイヤーにフォーカスする
    DirectX::XMFLOAT3 focusPos = GetGameObject()->transform_->GetWorldPosition();
    focusPos.y += 1;

    DirectX::XMVECTOR FocusPos = DirectX::XMLoadFloat3(&focusPos);

    //少しカメラのフォーカスを遅らせる
    {
        DirectX::XMStoreFloat3(&lerpFocusPos_, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&lerpFocusPos_), FocusPos, 0.1f * lerpSpeed_));
    }

    //カメラ回転値を回転行列に変換
    DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(angleX_), DirectX::XMConvertToRadians(angleY_), 0);

    //回転行列から前方向ベクトルを取り出す
    DirectX::XMVECTOR Front = Transform.r[2];
    DirectX::XMFLOAT3 front;
    DirectX::XMStoreFloat3(&front, Front);

    //レンジを代入
    float range = range_;

    DirectX::XMFLOAT3 cameraPos;
    //注視点から後ろベクトル方向に一定距離離れたカメラ視点を求める
    DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&lerpFocusPos_), DirectX::XMVectorScale(Front, range)));

    // スムーズなカメラ移動
    DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorLerp(XMLoadFloat3(&oldCameraPos_), DirectX::XMLoadFloat3(&cameraPos), 0.1f * lerpSpeed_));

    //カメラめり込み回避処理
    {
        DirectX::XMFLOAT3 kabePlus = GameObjectManager::Instance().Find("kabePlus")->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 kabeMinas = GameObjectManager::Instance().Find("kabeMinas")->transform_->GetWorldPosition();
        //補正
        kabePlus = { kabePlus.x + 1,0,kabePlus.z + 1 };
        kabeMinas = { kabeMinas.x - 1,0,kabeMinas.z - 1 };

        while (cameraPos.y < 0 ||   //地面
            cameraPos.x > kabePlus.x || cameraPos.x < kabeMinas.x ||    //壁
            cameraPos.z > kabePlus.z || cameraPos.z < kabeMinas.z)
        {
            range -= 0.01f;
            DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&lerpFocusPos_), DirectX::XMVectorScale(Front, range)));
            DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorLerp(XMLoadFloat3(&oldCameraPos_), DirectX::XMLoadFloat3(&cameraPos), 0.1f * lerpSpeed_));

            if (range <= 0.1f)break;
        }
    }

    //プレイヤーより前にカメラが行かない処理
    {
        DirectX::XMVECTOR cmDir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&lerpFocusPos_), DirectX::XMLoadFloat3(&cameraPos)));
        DirectX::XMVECTOR cmPlayerDir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(FocusPos, DirectX::XMLoadFloat3(&cameraPos)));

        float dot = DirectX::XMVector3Dot(cmDir, cmPlayerDir).m128_f32[0];
        if (dot < 0.9f)
        {
            //補正
            DirectX::XMStoreFloat3(&lerpFocusPos_, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&lerpFocusPos_), FocusPos, 0.1f * lerpSpeed_));
            DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&lerpFocusPos_), DirectX::XMVectorScale(Front, range)));
            DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorLerp(XMLoadFloat3(&oldCameraPos_), DirectX::XMLoadFloat3(&cameraPos), 0.1f * lerpSpeed_));

            float cross = DirectX::XMVector3Cross(cmDir, cmPlayerDir).m128_f32[1];
            if (cross > 0)
                angleY_ += 1000 * elapsedTime;
            else
                angleY_ -= 1000 * elapsedTime;
        }
    }

    oldCameraPos_ = cameraPos;

    //カメラに値を代入
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");
    cameraObj->transform_->SetWorldPosition(cameraPos);
    cameraObj->transform_->LookAtTransform(lerpFocusPos_);
}


// GUI描画
void PlayerCameraCom::OnGUI()
{
    ImGui::DragFloat("angleX", &angleX_, 0.1f);
    ImGui::DragFloat("angleY", &angleY_, 0.1f);
    ImGui::DragFloat("angleLimit_", &angleLimit_, 0.1f);
    ImGui::DragFloat("range", &range_, 0.1f);
    ImGui::DragFloat("lerpSpeed_", &lerpSpeed_, 0.1f);

    ImGui::Checkbox("isJust", &isJust_);
}

