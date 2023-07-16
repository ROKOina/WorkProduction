#include "PlayerCom.h"
#include "Input/Input.h"
#include <imgui.h>
#include "Components\CameraCom.h"
#include "Components\TransformCom.h"
#include "Components\AnimationCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"
int state = -1;
float dashMax = 20;

// 開始処理
void PlayerCom::Start()
{
    //カメラをプレイヤーにフォーカスする
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");

    //プレイヤーのワールドポジションを取得
    DirectX::XMFLOAT3 wp = GetGameObject()->transform_->GetWorldPosition();
    wp.z -= 10;
    wp.y += 6;
    cameraObj->transform_->SetLocalPosition(wp);

    //{   //仮アニメーション
    //    std::shared_ptr<AnimationCom> anim = GetGameObject()->GetComponent<AnimationCom>();
    //    anim->ImportFbxAnimation("Data/Model/pico/attack3Combo.fbx");
    //    anim->PlayAnimation(3, true);
    //}

    //パラメーター初期化
    moveParam_[MOVE_PARAM::WALK].moveMaxSpeed = 5.0f;
    moveParam_[MOVE_PARAM::WALK].moveAcceleration = 1.2f;
    moveParam_[MOVE_PARAM::WALK].turnSpeed =4.0f;

    moveParam_[MOVE_PARAM::RUN].moveMaxSpeed = 8.0f;
    moveParam_[MOVE_PARAM::RUN].moveAcceleration = 1.2f;
    moveParam_[MOVE_PARAM::RUN].turnSpeed =8.0f;

    moveParam_[MOVE_PARAM::DASH].moveMaxSpeed = 20.0f;
    moveParam_[MOVE_PARAM::DASH].moveAcceleration = 1.0f;
    moveParam_[MOVE_PARAM::DASH].turnSpeed =8.0f;

    moveParamType_ = MOVE_PARAM::WALK;
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
    move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
    move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);

    dashSpeed_ = 20.0f;
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
        cameraObj->transform_->LookAtTransform(GetGameObject()->transform_->GetLocalPosition());

        //プレイヤーのワールドポジションを取得
        DirectX::XMFLOAT3 wp = GetGameObject()->transform_->GetWorldPosition();
        wp.z -= 10;
        wp.y += 6;
        cameraObj->transform_->SetLocalPosition(wp);

    }

    //状態
    {
        //ダメージ
        if (isDamage_)
        {
            damageTimer_ += elapsedTime;
            if (damageTimer_ > damageInvincibleTime_)
            {
                isDamage_ = false;
                damageTimer_ = 0;
            }
        }
    }

    //移動
    {
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
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
    }

    //当たり判定
    std::vector<HitObj> hitGameObj = GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
    for (auto& hitObj : hitGameObj)
    {
        //名前検索
        //if (std::strcmp(hitObj.gameObject->GetName(), "picolabo") != 0)continue;
        //タグ検索
        //敵との当たり（仮）
        if (COLLIDER_TAG::Enemy == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
        {

            //削除
            //GameObjectManager::Instance().Remove(hitObj);

            //押し返し
            DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetWorldPosition();
            DirectX::XMFLOAT3 hitPos = hitObj.gameObject->transform_->GetWorldPosition();

            DirectX::XMVECTOR PlayerPos = { playerPos.x,playerPos.y,playerPos.z };
            DirectX::XMVECTOR HitPos = { hitPos.x, hitPos.y, hitPos.z };

            DirectX::XMVECTOR ForceNormal = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(HitPos, PlayerPos));
            DirectX::XMVectorSetY(ForceNormal, 0);
            ForceNormal = DirectX::XMVectorScale(ForceNormal, 0.01f);
            DirectX::XMFLOAT3 force;
            DirectX::XMStoreFloat3(&force, ForceNormal);
            hitObj.gameObject->transform_->SetWorldPosition(
                { hitPos.x + force.x,0,hitPos.z + force.z });
        }

        ////ジャスト回避
        //if (COLLIDER_TAG::JustAvoid == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
        //{
        //    GamePad& gamePad = Input::Instance().GetGamePad();
        //    if (gamePad.GetButtonDown() & GamePad::BTN_LSHIFT)
        //    {
        //        //速力をリセットする
        //        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        //        move->ZeroVelocity();

        //        std::shared_ptr<GameObject> enemy = hitObj.gameObject->GetParent();

        //        DirectX::XMFLOAT3 pos = enemy->transform_->GetWorldPosition();
        //        DirectX::XMFLOAT3 front = enemy->transform_->GetWorldFront();
        //        float dist = 2;
        //        pos = { pos.x - front.x * dist,0,pos.z - front.z * dist };

        //        GetGameObject()->transform_->SetWorldPosition(pos);
        //    }
        //}

    }
}

int oo = 0;
// GUI描画
void PlayerCom::OnGUI()
{

    ImGui::InputInt("nowMoveParamType", &moveParamType_);
    ImGui::SliderInt("moveParamType", &oo, 0, MOVE_PARAM::MAX-1);
    ImGui::DragFloat("moveMaxSpeed", &moveParam_[oo].moveMaxSpeed, 0.1f);
    ImGui::DragFloat("moveSpeed", &moveParam_[oo].moveSpeed, 0.1f);
    ImGui::DragFloat("moveAcceleration", &moveParam_[oo].moveAcceleration, 0.1f);

    ImGui::DragFloat("dashSpeed", &dashSpeed_);
    ImGui::DragFloat("justSpeed", &justSpeed_);
    ImGui::DragFloat("justSpeedTime", &justSpeedTime_);

    ImGui::DragFloat("jumpSpeed", &jumpSpeed_, 0.1f);

    ImGui::DragFloat3("up", &up_.x);

    ImGui::Checkbox("dash", &isDash_);
    ImGui::Checkbox("aaa", &aaa);

    //入力情報を取得
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();
    ImGui::DragFloat("x", &ax);
    ImGui::DragFloat("y", &ay);
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
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

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
        if (!isDash_)
        {
            //入力が終わると歩きに
            moveParamType_ = MOVE_PARAM::WALK;
            move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);
            move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
        }
        return false;
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
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

    move->AddForce({ 0,inputMoveVec_.y,0 });
}

//横方向移動
void PlayerCom::HorizonMove()
{
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

    //前方向に移動する力を生成
    DirectX::XMFLOAT3 forward = GetGameObject()->transform_->GetWorldFront();
    DirectX::XMVECTOR Forward = DirectX::XMLoadFloat3(&forward);
    DirectX::XMFLOAT3 moveVec;
    DirectX::XMStoreFloat3(&moveVec, DirectX::XMVectorScale(Forward, moveParam_[moveParamType_].moveSpeed));

    //力に加える
    move->AddForce(moveVec);
}

//ダッシュ
void PlayerCom::DashMove(float elapsedTime)
{
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();    
    //当たり判定
    std::vector<HitObj> hitGameObj = GetGameObject()->GetComponent<Collider>()->OnHitGameObject();

    std::shared_ptr<GameObject> enemy;
    bool isJust = false;
    for (auto& hitObj : hitGameObj)
    {
        //ジャスト回避した敵を保存＆ジャスト回避フラグをオン
        if (COLLIDER_TAG::JustAvoid == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
        {
            isJust = true;
            enemy = hitObj.gameObject->GetParent();
        }
    }

    //ダッシュ
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER)
    {
        //ダッシュ時に入力方向にすぐ移動できるように、角度を変える
        QuaternionStruct dashDirection = QuaternionStruct::LookRotation(inputMoveVec_, up_);
        GetGameObject()->transform_->SetRotation(dashDirection);

        state = 0;
    }

    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

    switch (state)
    {
    case 0:
        //ダッシュに変更
        move->ZeroVelocity();
        moveParamType_ = MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);
        state++;
        break;
    case 1:
        if (length > dashMax)
            state++;
        break;
    case 2:
    {
        float acce = move->GetMoveAcceleration();
        acce -= 2 * elapsedTime;
        move->SetMoveAcceleration(acce);

        if (moveParam_[MOVE_PARAM::RUN].moveMaxSpeed >= length)
            state++;
        break;
    }
    case 3:
        moveParamType_ = MOVE_PARAM::RUN;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
        state = -1;
        break;
    };


    //GamePad& gamePad = Input::Instance().GetGamePad();
    //if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER)
    //{
    //    //移動速力をリセットする
    //    move->ZeroVelocity();

    //    //ダッシュに変更
    //    moveParamType_ = MOVE_PARAM::DASH;
    //    move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);
    //    move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);

    //    //入力している場合
    //    DirectX::XMFLOAT3 dashVelocity = { 0,0,0 };
    //    if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
    //    {
    //        dashVelocity.x = inputMoveVec_.x * dashSpeed_;
    //        dashVelocity.z = inputMoveVec_.z * dashSpeed_;

    //        //回避orダッシュ
    //        if (isJust) {
    //            DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    //            GetGameObject()->transform_->SetWorldPosition({
    //                pos.x + inputMoveVec_.x * 3 ,pos.y,pos.z + inputMoveVec_.z * 3 });
    //        }
    //        else
    //            move->AddForce(dashVelocity);

    //        //ダッシュ時に入力方向にすぐ移動できるように、角度を変える
    //        QuaternionStruct dashDirection = QuaternionStruct::LookRotation(dashVelocity, up_);
    //        GetGameObject()->transform_->SetRotation(dashDirection);
    //    }
    //    else
    //    {
    //        DirectX::XMFLOAT3 front = GetGameObject()->transform_->GetWorldFront();
    //        dashVelocity.x = -front.x * dashSpeed_ * 2;
    //        dashVelocity.z = -front.z * dashSpeed_ * 2;

    //        //回避orダッシュ
    //        if (isJust) {
    //            DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    //            GetGameObject()->transform_->SetWorldPosition({
    //                pos.x - front.x * 3 ,pos.y,pos.z - front.z * 3 });
    //        }
    //        else
    //            move->AddForce(dashVelocity);

    //        move->SetMoveMaxSpeed(dashSpeed_ * 2);
    //    }


    //    isDash_ = true;
    //}

    //if (!isDash_)return;



    ////ダッシュ速力更新
    //DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());

    //float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

    ////RUNに切り替え
    //if (length < moveParam_[MOVE_PARAM::RUN].moveMaxSpeed)
    //{
    //    moveParamType_ = MOVE_PARAM::RUN;
    //    move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
    //    move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
    //    isDash_ = false;

    //}

}

/////////////////////////////////////////