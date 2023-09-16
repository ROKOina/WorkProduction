#include "MovePlayer.h"

#include "PlayerCom.h"
#include "Components/MovementCom.h"
#include "Components/AnimatorCom.h"
#include "Components/CameraCom.h"
#include "Components/TransformCom.h"
#include "Components/ColliderCom.h"
#include "Input/Input.h"
#include "SystemStruct/QuaternionStruct.h"
#include "Graphics/Graphics.h"

void MovePlayer::Update(float elapsedTime)
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    //移動が入力されていたら
    if (IsMove(elapsedTime))
    {
        //移動処理
        //回転
        if (isInputTrun_)
            Trun(elapsedTime);

        //ジャスト回避中は通らない
        if (isInputMove_)
        {
            //縦方向移動
            VerticalMove();
            //横方向移動
            HorizonMove();
        }
    }

    //ダッシュ処理
    DashMove(elapsedTime);

    //移動アニメ用に速力長さを取得
    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetFloatValue("moveSpeed", length);
}

//スティック入力値から移動ベクトルを取得
DirectX::XMFLOAT3 MovePlayer::GetMoveVec()
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
bool MovePlayer::IsMove(float elapsedTime)
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    //進行ベクトル取得
    inputMoveVec_ = GetMoveVec();

    if (move->OnGround() && jumpCount_ < 2)
    {
        jumpCount_ = 2;
        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);

        //アニメーター
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("idle");

    }

    //ジャンプ
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_B)
    {
        if (jumpCount_ > 0)
        {
            //ジャスト回避中のみジャンプできない
            if (!player_.lock()->isJustJudge_)
            {
                //攻撃と回避終了フラグ
                player_.lock()->AttackFlagEnd();
                DashEndFlag(false);

                //アニメーター
                std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
                animator->ResetParameterList();
                animator->SetTriggerOn("jump");
                inputMoveVec_.y = jumpSpeed_;

                move->ZeroVelocityY();
                --jumpCount_;

                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP);
            }
        }
    }

    //進行ベクトルがゼロベクトルでない場合は入力された
    if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.y * inputMoveVec_.y + inputMoveVec_.z * inputMoveVec_.z <= 0.1f) {
        DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
        float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

        if (isInputMove_)
            //if (length < 0.1f)
            //if (!isDashJudge_)
        {
            //入力が終わると歩きに
            if (player_.lock()->GetPlayerStatus() != PlayerCom::PLAYER_STATUS::JUMP)
                if (player_.lock()->comboAttackCount_ <= 0)
                    if (!isDashJudge_)
                    {
                        moveParamType_ = MOVE_PARAM::WALK;
                        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);
                        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);

                        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
                    }
        }
        return false;
    }

    //状態更新
    if (player_.lock()->GetPlayerStatus() != PlayerCom::PLAYER_STATUS::JUMP)
        if (!isDashJudge_ && !player_.lock()->isJustJudge_ && player_.lock()->comboAttackCount_ <= 0)
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::MOVE);


    return true;
}

//回転
void MovePlayer::Trun(float elapsedTime)
{
    //入力方向のクォータニオン生成
    QuaternionStruct inputQuaternion = QuaternionStruct::LookRotation(inputMoveVec_, player_.lock()->up_);
    QuaternionStruct playerQuaternion = player_.lock()->GetGameObject()->transform_->GetRotation();

    float worldSpeed = Graphics::Instance().GetWorldSpeed();

    //ゆっくり回転
    DirectX::XMFLOAT4 rota;
    DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&playerQuaternion.dxFloat4),
        DirectX::XMLoadFloat4(&inputQuaternion.dxFloat4), moveParam_[moveParamType_].turnSpeed * (elapsedTime * worldSpeed)));

    player_.lock()->GetGameObject()->transform_->SetRotation(rota);
}

//縦方向移動
void MovePlayer::VerticalMove()
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    move->AddForce({ 0,inputMoveVec_.y,0 });
}

//横方向移動
void MovePlayer::HorizonMove()
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    //前方向に移動する力を生成
    DirectX::XMFLOAT3 forward = player_.lock()->GetGameObject()->transform_->GetWorldFront();
    DirectX::XMVECTOR Forward = DirectX::XMLoadFloat3(&forward);
    DirectX::XMFLOAT3 moveVec;
    DirectX::XMStoreFloat3(&moveVec, DirectX::XMVectorScale(Forward, moveParam_[moveParamType_].moveSpeed));

    //力に加える
    move->AddForce(moveVec);
}

//ダッシュ
void MovePlayer::DashMove(float elapsedTime)
{
    //ダッシュクールタイム更新
    if (dashCoolTimer_ >= 0)
    {
        dashCoolTimer_ -= elapsedTime;
    }

    if (!isDash_)return;

    //ダッシュ
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER &&
        dashCoolTimer_ < 0)
    {
        isDashJudge_ = true;        //ダッシュフラグON

        dashCoolTimer_ = dashCoolTime_; //クールタイム

        //アニメーター
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

        //ダッシュ後コンボ
        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH)
        {
            if (player_.lock()->attackPlayer_->OnHitEnemy() && player_.lock()->attackPlayer_->ComboReadyEnemy())
            {
                Graphics::Instance().SetWorldSpeed(1.0f);
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::DASH);


                player_.lock()->animFlagName_ = "jump";
                animator->ResetParameterList();
                player_.lock()->attackPlayer_->DashAttack(2);
                animator->SetIsStop(true);
                isDash_ = false;
                return;
            }
        }

        //アタックリセット
        player_.lock()->AttackFlagEnd();

        //入力があればstate=0
        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
        {
            dashState_ = 0;
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP)
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP_DASH);
            else
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::DASH);
        }
        else
        {
            dashState_ = 10;
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP)
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP_BACK_DASH);
            else
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::BACK_DASH);
        }

        animator->SetTriggerOn("dash");


        ////ジャスト回避初期化
        //JustInisialize();
        //isNormalAttack_ = true;

    }

    //ダッシュ時の更新
    DashStateUpdate(elapsedTime);

}

//ダッシュ時の更新
void MovePlayer::DashStateUpdate(float elapsedTime)
{
    if (!isDashJudge_)return;

    //ジャスト回避の当たり判定
    std::vector<HitObj> hitGameObj = player_.lock()->GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
    //ジャスト回避したエネミーを保存
    std::shared_ptr<GameObject> enemy;
    for (auto& hitObj : hitGameObj)
    {
        if (COLLIDER_TAG::JustAvoid != hitObj.gameObject->GetComponent<Collider>()->GetMyTag())continue;

        if (!enemy) {
            enemy = hitObj.gameObject->GetParent();
            continue;
        }
        //一番近い敵を保存
        DirectX::XMFLOAT3 pPos = player_.lock()->GetGameObject()->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 ePos = hitObj.gameObject->GetParent()->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 eOldPos = enemy->transform_->GetWorldPosition();

        DirectX::XMVECTOR PPos = DirectX::XMLoadFloat3(&pPos);
        DirectX::XMVECTOR EPos = DirectX::XMLoadFloat3(&ePos);
        DirectX::XMVECTOR EOldPos = DirectX::XMLoadFloat3(&eOldPos);

        float currentLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(PPos, EPos)));
        float oldLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(PPos, EOldPos)));

        if (currentLength < oldLength)
            enemy = hitObj.gameObject->GetParent();
    }


    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

    //ダッシュ更新
    switch (dashState_)
    {
    case 0:
    {
        //角度を変える
        QuaternionStruct dashDirection = QuaternionStruct::LookRotation(inputMoveVec_, player_.lock()->up_);
        player_.lock()->GetGameObject()->transform_->SetRotation(dashDirection);
    }
    [[fallthrough]];    //フォースルー(1にそのままいく)
    //入力方向ダッシュ
    case 1:
    {
        isInputMove_ = true;
        player_.lock()->isNormalAttack_ = false;

        //ダッシュに変更
        move->ZeroVelocity();
        moveParamType_ = MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);
        dashStopTimer_ = dashStopTime_;
        dashState_++;
        break;
    }
    case 2:
        //ジャスト回避判定
        if (enemy)
        {
            player_.lock()->isJustJudge_ = true;
            player_.lock()->justAvoidState_ = 0;
            dashState_ = -1;
            isDashJudge_ = false;
            player_.lock()->justHitEnemy_ = enemy;
            break;
        }

        //最大速度に達したら次のステート
        dashStopTimer_ -= elapsedTime;
        if (length > dashMaxSpeed_ || dashStopTimer_ < 0)
        {
            dashState_++;
            player_.lock()->isNormalAttack_ = true;
        }
        break;
    case 3:
    {
        //加速度を下げていく
        float acce = move->GetMoveAcceleration();
        acce -= 10 * elapsedTime;
        move->SetMoveAcceleration(acce);

        if (moveParam_[MOVE_PARAM::RUN].moveMaxSpeed >= length)
            dashState_++;
        break;
    }
    case 4:
        //走りに変更(ダッシュ終了)
        moveParamType_ = MOVE_PARAM::RUN;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
        dashState_ = -1;
        isDashJudge_ = false;
        isInputMove_ = true;

        break;

        //後ろ向きダッシュ
    case 10:
        //ダッシュに変更
        move->ZeroVelocity();
        moveParamType_ = MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed * 2);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);
        dashStopTimer_ = dashStopTime_;
        dashState_++;

        isInputMove_ = false;

        break;
    case 11:
    {
        //ジャスト回避判定
        if (enemy)
        {
            player_.lock()->isJustJudge_ = true;
            player_.lock()->justAvoidState_ = 0;
            dashState_ = -1;
            isDashJudge_ = false;
            player_.lock()->justHitEnemy_ = enemy;
            break;
        }

        DirectX::XMFLOAT3 front = player_.lock()->GetGameObject()->transform_->GetWorldFront();
        DirectX::XMFLOAT3 back = { -front.x,0,-front.z };
        DirectX::XMStoreFloat3(&back, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&back), moveParam_[moveParamType_].moveSpeed));
        move->AddForce(back);

        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
            dashState_ = 1;

        //最大速度に達したら次のステート
        dashStopTimer_ -= elapsedTime;
        if (length > dashMaxSpeed_ * 0.5f || dashStopTimer_ < 0)
            dashState_++;
        break;
    }
    case 12:
    {
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);

        DirectX::XMFLOAT3 front = player_.lock()->GetGameObject()->transform_->GetWorldFront();
        DirectX::XMFLOAT3 back = { -front.x,0,-front.z };
        DirectX::XMStoreFloat3(&back, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&back), moveParam_[moveParamType_].moveSpeed));
        move->AddForce(back);

        //加速度を下げていく
        float acce = move->GetMoveAcceleration();
        acce -= 10 * elapsedTime;
        move->SetMoveAcceleration(acce);

        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
            dashState_ = 1;

        if (moveParam_[MOVE_PARAM::RUN].moveMaxSpeed >= length)
            dashState_ = 3;
        break;
    }
    };

    //速力がなくなるかつ、ダッシュステートが10以下の場合（後ろダッシュ時は速力は0なので）
    //ダッシュ終了
    if (length < 0.1f && dashState_ < 10 && dashState_ > 1)
    {
        isDashJudge_ = false;
        dashState_ = -1;
    }
}

//強制的にダッシュを終わらせる（攻撃時等）
void MovePlayer::DashEndFlag(bool isWalk)
{
    isDashJudge_ = false;
    dashState_ = -1;

    if (isWalk)
    {
        std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
        moveParamType_ = MOVE_PARAM::WALK;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
    }
}
