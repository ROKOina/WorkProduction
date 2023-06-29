#include "PlayerCom.h"
#include "Input/Input.h"
#include "Components\CameraCom.h"
#include "Components\TransformCom.h"
#include "Components\AnimationCom.h"
#include <imgui.h>
#include "Components\RendererCom.h"

// 開始処理
void PlayerCom::Start()
{
    //カメラをプレイヤーにフォーカスする
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");

    //プレイヤーのワールドポジションを取得
    DirectX::XMFLOAT3 wp = GetGameObject()->transform_->GetWorldPosition();
    wp.z -= 10;
    wp.y += 6;
    cameraObj->transform_->SetPosition(wp);

    {   //仮アニメーション
        std::shared_ptr<AnimationCom> anim = GetGameObject()->GetComponent<AnimationCom>();
        anim->ImportFbxAnimation("Data/Model/pico/attack3Combo.fbx");
        anim->PlayAnimation(3, true);
    }
}
static bool aaa = true;
// 更新処理
void PlayerCom::Update(float elapsedTime)
{
    //カメラ設定
    if(aaa)
    {
        //カメラをプレイヤーにフォーカスする
        std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");
        cameraObj->transform_->LookAtTransform(GetGameObject()->transform_->GetPosition());

        //プレイヤーのワールドポジションを取得
        DirectX::XMFLOAT3 wp = GetGameObject()->transform_->GetWorldPosition();
        wp.z -= 10;
        wp.y += 6;
        cameraObj->transform_->SetPosition(wp);

    }

    //移動
    {
        Move(elapsedTime);

        //速力処理更新
        DirectX::XMFLOAT3 p = GetGameObject()->transform_->GetPosition();
        DirectX::XMFLOAT4 r = GetGameObject()->transform_->GetRotation();
        //UpdateVelocity(elapsedTime, p, r);
        //UpdateVelocity(elapsedTime, p, DirectX::XMFLOAT4(0, 0, 0, 0),up_);
        GetGameObject()->transform_->SetPosition(p);
        GetGameObject()->transform_->SetRotation(r);
    }

    //仮回転
    {
        //DirectX::XMFLOAT3 pos = GameObjectManager::Instance().Find("miru")->transform->GetPosition();
        //GetGameObject()->transform->LookAtTransform(pos, { 0,1,0 });
    }
}

// GUI描画
void PlayerCom::OnGUI()
{
    ImGui::DragFloat("moveSpeed", &moveSpeed_);
    ImGui::DragFloat("turnSpeed", &turnSpeed_);

    ImGui::DragFloat3("lookP", &look_.x);
    ImGui::DragFloat3("up", &up_.x);

    DirectX::XMFLOAT3 p;
    p = GetGameObject()->transform_->GetUp();
    ImGui::DragFloat3("myUp", &p.x);
    ImGui::Checkbox("aaa", &aaa);
}


//スティック入力値から移動ベクトルを取得
DirectX::XMFLOAT3 PlayerCom::GetMoveVec()
{
    //入力情報を取得
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();

    //カメラ方向とスティックの入力値によって進行方向を計算する
    std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
    const DirectX::XMFLOAT3& cameraRight = camera->GetRight();
    const DirectX::XMFLOAT3& cameraFront = camera->GetFront();

    //移動ベクトルはXZ平面に水平なベクトルになるようにする

    //カメラ右方向ベクトルをXZ単位ベクトルに変換
    float cameraRightX = cameraRight.x;
    float cameraRightZ = cameraRight.z;
    float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
    if (cameraRightLength > 0.0f)
    {
        //単位ベクトル化
        cameraRightX /= cameraRightLength;
        cameraRightZ /= cameraRightLength;
    }

    //カメラ前方向ベクトルをXZ単位ベクトルに変換
    float cameraFrontX = cameraFront.x;
    float cameraFrontZ = cameraFront.z;
    float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
    if (cameraFrontLength > 0.0f)
    {
        //単位ベクトル化
        cameraFrontX /= cameraFrontLength;
        cameraFrontZ /= cameraFrontLength;
    }

    //スティックの水平入力値をカメラ右方向に反映し、
    //スティックの垂直入力値をカメラ前方向に反映し、
    //進行ベクトルを計算する
    DirectX::XMFLOAT3 vec;
    //XZ軸方向に移動
    vec.x = cameraFrontX * ay + cameraRightX * ax;
    vec.z = cameraFrontZ * ay + cameraRightZ * ax;
    //Y軸方向には移動しない
    vec.y = 0.0f;

    return vec;
}

//移動入力処理
bool PlayerCom::Move(float elapsedTime)
{
    //進行ベクトル取得
    inputMoveVec_ = GetMoveVec();

    //進行ベクトルがゼロベクトルでない場合は入力された
    if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.y * inputMoveVec_.y + inputMoveVec_.z * inputMoveVec_.z <= 0.1f) {
        return false;
    }

    ////移動処理
    //Move(moveVec.x, moveVec.z, moveSpeed_);


    //旋回処理＆地面の法線で回転
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    look_ = { inputMoveVec_.x * 3 + pos.x
        ,pos.y
        ,inputMoveVec_.z * 3 + pos.z };

    //入力方向のクォータニオン生成
    QuaternionStruct inputQuaternion = QuaternionStruct::LookRotation(inputMoveVec_, up_);
    QuaternionStruct playerQuaternion = GetGameObject()->transform_->GetRotation();

    //DirectX::XMQuaternionSlerp()

    //GetGameObject()->transform_->LookAtTransform(look_, up_);
    //GetGameObject()->transform_->SetUpTransform(up_);

    return true;
}