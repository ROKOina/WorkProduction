#include "PlayerCom.h"
#include "Input/Input.h"
#include "Components\CameraCom.h"
#include "Components\TransformCom.h"
#include "Components\AnimationCom.h"
#include <imgui.h>
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"

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

    //{   //仮アニメーション
    //    std::shared_ptr<AnimationCom> anim = GetGameObject()->GetComponent<AnimationCom>();
    //    anim->ImportFbxAnimation("Data/Model/pico/attack3Combo.fbx");
    //    anim->PlayAnimation(3, true);
    //}

    //パラメーター初期化
    moveParam_[MOVE_PARAM::WALK].moveMaxSpeed = 3.0f;
    moveParam_[MOVE_PARAM::WALK].moveAcceleration = 0.2f;
    moveParam_[MOVE_PARAM::WALK].turnSpeed =4.0f;

    moveParam_[MOVE_PARAM::RUN].moveMaxSpeed = 8.0f;
    moveParam_[MOVE_PARAM::RUN].moveAcceleration = 0.2f;
    moveParam_[MOVE_PARAM::RUN].turnSpeed =8.0f;

    dashSpeed_ = 15.0f;
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
        //移動が入力されていたら
        if (IsMove(elapsedTime))
        {
            //移動処理
            //回転
            Trun(elapsedTime);
            //縦方向移動
            VerticalMove();
            //横方向移動
            HorizonMove();
        }

        //ダッシュ処理
        DashMove(elapsedTime);

        //縦方向移動更新
        VerticalUpdate(elapsedTime);
        //横方向移動更新
        HorizonUpdate(elapsedTime);

        //速力をポジションに更新
        VelocityAppPosition(elapsedTime);
    }

    //当たり判定
    std::vector<std::shared_ptr<GameObject>> hitGameObj = GetGameObject()->GetComponent<SphereColliderCom>()->OnHitGameObject();
    for (auto& hitObj : hitGameObj)
    {
        if (std::strcmp(hitObj->GetName(), "picolabo") != 0)continue;
        GameObjectManager::Instance().Remove(hitObj);
    }
}

// GUI描画
void PlayerCom::OnGUI()
{
    ImGui::SliderInt("moveParamType", &moveParamType_, 0, 1);
    int i = 0;
    for (auto& moveP : moveParam_)
    {
        std::string m;
        m = "moveSpeed" + std::to_string(i);
        ImGui::DragFloat(m.c_str(), &moveP.moveMaxSpeed, 0.1f);
        m = "moveAcceleration" + std::to_string(i);
        ImGui::DragFloat(m.c_str(), &moveP.moveAcceleration, 0.1f);
        m = "turnSpeed" + std::to_string(i);
        ImGui::DragFloat(m.c_str(), &moveP.turnSpeed, 0.1f);
        i+=10;
    }

    ImGui::DragFloat("jumpSpeed", &jumpSpeed_, 0.1f);
    ImGui::DragFloat("gravity", &gravity_, 0.1f);
    ImGui::DragFloat("friction", &friction_, 0.1f);

    ImGui::DragFloat3("up", &up_.x);
    ImGui::DragFloat3("velocity", &velocity_.x);

    DirectX::XMFLOAT3 p;
    p = GetGameObject()->transform_->GetUp();
    ImGui::DragFloat3("myUp", &p.x);
    

    ImGui::DragFloat("dashSpeed", &dashSpeed_);
    float s1 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&velocity_)));
    float s2 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&dashVelocity_)));
    float speed = s1 + s2;
    ImGui::InputFloat("speedLook", &speed);

    ImGui::Checkbox("aaa", &aaa);
}




////    移動系処理       ////

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
bool PlayerCom::IsMove(float elapsedTime)
{
    //進行ベクトル取得
    inputMoveVec_ = GetMoveVec();

    //ジャンプ
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_SPACE)
    {
        inputMoveVec_.y = jumpSpeed_;
    }

    //進行ベクトルがゼロベクトルでない場合は入力された
    if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.y * inputMoveVec_.y + inputMoveVec_.z * inputMoveVec_.z <= 0.1f) {
        //入力が終わると歩きに
        moveParamType_ = MOVE_PARAM::WALK;
        return false;
    }

    //ダッシュ
    if (gamePad.GetButton() & GamePad::BTN_LSHIFT)
    {
        moveParamType_ = MOVE_PARAM::RUN;
    }

    return true;
}

//回転
void PlayerCom::Trun(float elapsedTime)
{
    //入力方向のクォータニオン生成
    QuaternionStruct inputQuaternion = QuaternionStruct::LookRotation(inputMoveVec_, up_);
    QuaternionStruct playerQuaternion = GetGameObject()->transform_->GetRotation();

    //ゆっくり回転
    DirectX::XMFLOAT4 rota;
    DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&playerQuaternion.dxFloat4),
        DirectX::XMLoadFloat4(&inputQuaternion.dxFloat4), moveParam_[moveParamType_].turnSpeed * elapsedTime));

    GetGameObject()->transform_->SetRotation(rota);
}

//縦方向移動
void PlayerCom::VerticalMove()
{
    AddForce({ 0,inputMoveVec_.y,0 });
}

//横方向移動
void PlayerCom::HorizonMove()
{
    //前方向に移動する力を生成
    DirectX::XMFLOAT3 forward = GetGameObject()->transform_->GetFront();
    DirectX::XMVECTOR Forward = DirectX::XMLoadFloat3(&forward);
    DirectX::XMFLOAT3 moveVec;
    DirectX::XMStoreFloat3(&moveVec, DirectX::XMVectorScale(Forward, moveParam_[moveParamType_].moveAcceleration));

    //力に加える
    AddForce(moveVec);
}

//縦方向移動更新
void PlayerCom::VerticalUpdate(float elapsedTime)
{
    float gravity= gravity_ * (elapsedTime * Graphics::Instance().GetFPS());
    AddForce({ 0,gravity,0 });

    //とりあえず0以下補正
    DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetPosition();
    if (playerPos.y < 0 && velocity_.y < 0)
    {
        velocity_.y = 0;
        playerPos.y = 0;
        GetGameObject()->transform_->SetPosition(playerPos);
    }
}

//横方向移動更新
void PlayerCom::HorizonUpdate(float elapsedTime)
{
    DirectX::XMFLOAT3 horizonVelocity = { velocity_.x,0,velocity_.z };
    DirectX::XMVECTOR HorizonVelocity = DirectX::XMLoadFloat3(&horizonVelocity);
    float horiLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(HorizonVelocity));


    //最大速度設定
    if (horiLength > moveParam_[moveParamType_].moveMaxSpeed)
    {
        DirectX::XMVECTOR MaxSpeed = DirectX::XMVectorScale(DirectX::XMVector3Normalize(HorizonVelocity), moveParam_[moveParamType_].moveMaxSpeed);
        DirectX::XMFLOAT3 newMaxVelocity;
        DirectX::XMStoreFloat3(&newMaxVelocity, MaxSpeed);
        velocity_.x = newMaxVelocity.x;
        velocity_.z = newMaxVelocity.z;
    }

    float friction = friction_ * (elapsedTime * Graphics::Instance().GetFPS());
    //摩擦力
    if (horiLength > friction)
    {
        DirectX::XMVECTOR FriVelocity = DirectX::XMVectorScale(DirectX::XMVector3Normalize(HorizonVelocity), -friction);
        DirectX::XMFLOAT3 newVelocity;
        DirectX::XMStoreFloat3(&newVelocity, FriVelocity);
        AddForce(newVelocity);
    }
    else
    {
        velocity_.x = 0;
        velocity_.z = 0;
    }
}

//速力を更新
void PlayerCom::VelocityAppPosition(float elapsedTime)
{
    DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetPosition();
    DirectX::XMFLOAT3 velocity = velocity_;

    //ダッシュ速力追加
    velocity.x += dashVelocity_.x;
    velocity.y += dashVelocity_.y;
    velocity.z += dashVelocity_.z;

    playerPos.x += velocity.x * elapsedTime;
    playerPos.y += velocity.y * elapsedTime;
    playerPos.z += velocity.z * elapsedTime;
    GetGameObject()->transform_->SetPosition(playerPos);
}

//ダッシュ
void PlayerCom::DashMove(float elapsedTime)
{
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_LSHIFT)
    {
        //移動速力をリセットする
        velocity_.x = 0;
        velocity_.z = 0;

        //入力している場合
        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0)
        {
            dashVelocity_.x = inputMoveVec_.x * dashSpeed_;
            dashVelocity_.z = inputMoveVec_.z * dashSpeed_;

            //ダッシュ時に入力方向にすぐ移動できるように、角度を変える
            QuaternionStruct dashDirection = QuaternionStruct::LookRotation(dashVelocity_, up_);
            GetGameObject()->transform_->SetRotation(dashDirection);
        }
        else
        {
            DirectX::XMFLOAT3 front = GetGameObject()->transform_->GetFront();
            dashVelocity_.x = -front.x * dashSpeed_;
            dashVelocity_.z = -front.z * dashSpeed_;
        }

        isDash_ = true;
    }

    //ダッシュ速力更新
    if (dashVelocity_.x * dashVelocity_.x + dashVelocity_.z * dashVelocity_.z <= 0)return;
    
    DirectX::XMFLOAT3 dash = { dashVelocity_.x,0,dashVelocity_.z };
    DirectX::XMVECTOR Dash = DirectX::XMLoadFloat3(&dash);
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Dash));

    float friction = friction_ * (elapsedTime * Graphics::Instance().GetFPS());

    //摩擦力
    if (length > friction)
    {
        DirectX::XMVECTOR FriVelocity = DirectX::XMVectorScale(DirectX::XMVector3Normalize(Dash), -friction);
        DirectX::XMFLOAT3 newVelocity;
        DirectX::XMStoreFloat3(&newVelocity, FriVelocity);
        dashVelocity_.x += newVelocity.x;
        dashVelocity_.z += newVelocity.z;
    }
    else
    {
        dashVelocity_.x = 0;
        dashVelocity_.z = 0;
        isDash_ = false;
    }

}

/////////////////////////////////////////