#include "MovePlayer.h"

#include "PlayerCom.h"
#include "PlayerCameraCom.h"
#include "../Enemy/EnemyManager.h"
#include "Components/MovementCom.h"
#include "Components/AnimatorCom.h"
#include "Components/AnimationCom.h"
#include "Components/CameraCom.h"
#include "Components/TransformCom.h"
#include "Components/ColliderCom.h"
#include "Input/Input.h"
#include "SystemStruct/QuaternionStruct.h"
#include "Graphics/Graphics.h"
#include <imgui.h>

MovePlayer::MovePlayer(std::shared_ptr<PlayerCom> player) : player_(player)
{
    //移動パラメーター初期化
    moveParam_[MOVE_PARAM::WALK].moveMaxSpeed = 5.0f;
    moveParam_[MOVE_PARAM::WALK].moveAcceleration = 1.0f;
    moveParam_[MOVE_PARAM::WALK].turnSpeed = 4.0f;

    moveParam_[MOVE_PARAM::RUN].moveMaxSpeed = 8.0f;
    moveParam_[MOVE_PARAM::RUN].moveAcceleration = 2.0f;
    moveParam_[MOVE_PARAM::RUN].turnSpeed = 8.0f;

    moveParam_[MOVE_PARAM::JUSTDASH].moveMaxSpeed = 3.0f;
    moveParam_[MOVE_PARAM::JUSTDASH].moveAcceleration = 1.0f;
    moveParam_[MOVE_PARAM::JUSTDASH].turnSpeed = 4.0f;

    moveParam_[MOVE_PARAM::DASH].moveMaxSpeed = 15.0f;
    moveParam_[MOVE_PARAM::DASH].moveAcceleration = 6.0f;
    moveParam_[MOVE_PARAM::DASH].turnSpeed = 8.0f;

    moveParamType_ = MOVE_PARAM::WALK;
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
    move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);

    dashCoolTimer_ = 1;
}

void MovePlayer::Update(float elapsedTime)
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    //移動が入力されていたら
    if (IsMove(elapsedTime))
    {
        //移動処理

        //走り切り替えし判定
        RunTurnJudge();

        if (!isRunTurn_)    //ターン中じゃなければ入る
        {
            //回転
            if (isInputTurn_)
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
    }

    //切り替えし処理
    RunTurnMove();


    //ダッシュ処理
    DashMove(elapsedTime);


    //移動アニメ用に速力長さを取得
    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetFloatValue("moveSpeed", length);
}

void MovePlayer::OnGui()
{
    ImGui::InputInt("paramType", &moveParamType_);
    ImGui::InputInt("dashState", &dashState_);
    ImGui::InputInt("jumpCount", &jumpCount_);
    ImGui::Checkbox("isInputMove", &isInputMove_);
    ImGui::Checkbox("isInputTurn", &isInputTurn_);
}

void MovePlayer::AudioRelease()
{
    jumpSE_->AudioRelease();
    dashSE_->AudioRelease();
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
    DirectX::XMFLOAT3 vec = {};
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

    //着地処理
    if (move->OnGround() && jumpCount_ < 2
        && player_.lock()->GetPlayerStatus() != PlayerCom::PLAYER_STATUS::ATTACK_JUMP)
    {
        jumpCount_ = 2;
        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
        isInputMove_ = true;
        isInputTurn_ = true;

        //アニメーター
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("idle");

        //攻撃着地処理
        player_.lock()->GetAttackPlayer()->AttackOnGround();
    }

    //ジャンプ
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_A)
    {
        if (jumpCount_ > 0)
        {
            //ジャスト回避中じゃない時とジャンプフラグOnの時
            if (!player_.lock()->GetJustAvoidPlayer()->GetIsJustJudge()&& isJump_)
            {
                //終了フラグ
                player_.lock()->GetAttackPlayer()->AttackFlagEnd();
                player_.lock()->GetAttackPlayer()->AttackJump();
                DashEndFlag(false);
                EndRunTurn(); 

                //アニメーター
                std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
                animator->ResetParameterList();
                animator->SetTriggerOn("jump");
                isStopRunStop_ = false;

                //速度
                inputMoveVec_.y = jumpSpeed_;
                move->ZeroVelocityY();

                //ジャンプ回数減らす
                --jumpCount_;

                //空中ダッシュリセット
                jumpDashCount_ = 1;

                //カメラを引く
                player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>()->SetChangeRange(0.2f, FarRange);

                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP);

                //SE
                jumpSE_->Stop();
                jumpSE_->Play(false);
            }
        }
    }

    //ジャンプ降下アニメーション
    if (!move->OnGround())
    {
        if (move->GetVelocity().y < 0)
        {
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP)
            {
                std::shared_ptr<AnimationCom> animation = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
                if (animation->GetCurrentAnimationIndex() != ANIMATION_PLAYER::JUMP_FALL)   //切り下ろし攻撃じゃない時
                {
                    //アニメーター
                    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
                    animator->SetTriggerOn("jumpFall");
                }
            }
        }
    }

    //ストップモーションフラグ処理
    if (isStopRunStop_)
    {
        std::shared_ptr<AnimationCom> animation = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
        if (animation->IsPlayAnimation())
        {
            return false;
        }
        else
        {
            isStopRunStop_ = false;
        }
    }

    static float nonInputTime = 0.1f;   //入力されていない時間を測る
    //進行ベクトルがゼロベクトルでない場合は入力された
    if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.y * inputMoveVec_.y + inputMoveVec_.z * inputMoveVec_.z <= 0.1f)
    {
        DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
        float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

        if (isInputMove_)
        {
            //入力が終わると歩きに
            if (player_.lock()->GetPlayerStatus() != PlayerCom::PLAYER_STATUS::JUMP)
                if (player_.lock()->GetAttackPlayer()->GetComboSquareCount() <= 0&&player_.lock()->GetAttackPlayer()->GetComboTriangleCount() <= 0)
                    if (!isDashJudge_)
                    {
                        nonInputTime -= elapsedTime;
                        if (nonInputTime < 0)   //入力されていない猶予時間を過ぎれば歩きに
                        {
                            if (moveParamType_ != MOVE_PARAM::WALK)
                            {                
                                if (player_.lock()->GetPlayerStatusOld() == PlayerCom::PLAYER_STATUS::MOVE
                                    || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::BACK_DASH)
                                {
                                    //アニメーター
                                    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
                                    animator->SetTriggerOn("runStop");
                                    isStopRunStop_ = true;
                                    move->ZeroVelocity();
                                }

                                moveParamType_ = MOVE_PARAM::WALK;
                                move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);
                                move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
                            }
                        }

                        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
                    }
        }
        return false;
    }
    else
    {
        //入力されていない猶予時間
        nonInputTime = 0.05f;
    }

    //状態更新
    if (player_.lock()->GetPlayerStatus() != PlayerCom::PLAYER_STATUS::JUMP && isInputMove_)
    {
        if (!isDashJudge_ && !player_.lock()->GetJustAvoidPlayer()->GetIsJustJudge()
            && player_.lock()->GetAttackPlayer()->GetComboSquareCount() <= 0
            && player_.lock()->GetAttackPlayer()->GetComboTriangleCount() <= 0
            )
        {
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::MOVE);
        }
    }


    return true;
}

//回転
void MovePlayer::Trun(float elapsedTime)
{
    //入力方向のクォータニオン生成
    QuaternionStruct inputQuaternion = QuaternionStruct::LookRotation({ inputMoveVec_.x,0,inputMoveVec_.z });
    QuaternionStruct playerQuaternion = player_.lock()->GetGameObject()->transform_->GetRotation();

    float worldSpeed = Graphics::Instance().GetWorldSpeed();

    //ゆっくり回転
    DirectX::XMFLOAT4 rota;

    //ターンスピード
    float turnSpeed = moveParam_[moveParamType_].turnSpeed;
    if (player_.lock()->GetAttackPlayer()->GetComboSquareCount() > 0
        || player_.lock()->GetAttackPlayer()->GetComboTriangleCount() > 0
        )   //攻撃中なら遅くする
        turnSpeed = 5.5f;

    DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&playerQuaternion.dxFloat4),
        DirectX::XMLoadFloat4(&inputQuaternion.dxFloat4), turnSpeed * (elapsedTime * worldSpeed)));

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
    forward.y = 0;
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

    //空中ダッシュリセット
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    if (move->OnGround())
    {
        jumpDashCount_ = 1;
    }

    if (!isDash_)return;

    //ダッシュ
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER &&
        dashCoolTimer_ < 0 && jumpDashCount_ > 0)
    {
        //ターン処理終了
        EndRunTurn();

        isStopRunStop_ = false;

        isDashJudge_ = true;        //ダッシュフラグON

        dashCoolTimer_ = dashCoolTime_; //クールタイム

        jumpDashCount_--;   //空中ダッシュカウント下げる

        //アニメーター
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

        animator->ResetParameterList();

        //アタックリセット
        player_.lock()->GetAttackPlayer()->AttackFlagEnd();

        //入力があればstate=0
        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
        {
            dashState_ = 0;
            if (!move->OnGround())
                //if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP)
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP_DASH);
            else
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::DASH);

            animator->SetTriggerOn("dash");

            //ダッシュ時ブラー
            if (!player_.lock()->IsBlurPlay())
                player_.lock()->BlurStartPlayer(0.1f, 0.5f, "Head");
        }
        else
        {
            dashState_ = 10;
            if (!move->OnGround())
                //if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP)
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP_BACK_DASH);
            else
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::BACK_DASH);

            animator->SetTriggerOn("dashBack");
        }

        //SE
        dashSE_->Stop();
        dashSE_->Play(false, 0.5f);
    }

    //ダッシュ時の更新
    DashStateUpdate(elapsedTime);

}

//ダッシュ時の更新
void MovePlayer::DashStateUpdate(float elapsedTime)
{
    if (!isDashJudge_)return;

    //ジャスト回避判定
    player_.lock()->GetJustAvoidPlayer()->JustAvoidJudge();

    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

    //ダッシュ更新
    switch (dashState_)
    {
    case 0:
    {
        //角度を変える
        QuaternionStruct dashDirection = QuaternionStruct::LookRotation(inputMoveVec_);
        player_.lock()->GetGameObject()->transform_->SetRotation(dashDirection);

    }
    [[fallthrough]];    //フォースルー(1にそのままいく)
    //入力方向ダッシュ
    case 1:
    {
        isInputMove_ = true;
        player_.lock()->GetAttackPlayer()->SetIsNormalAttack(false);

        //ダッシュに変更
        move->ZeroVelocity();
        moveParamType_ = MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);
        dashStopTimer_ = dashStopTime_;

        //アニメーター
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("idle");

        dashState_ = 2;
        break;
    }
    case 2:
        //ジャスト回避判定
        if (player_.lock()->GetJustAvoidPlayer()->GetJustHitEnemy().lock())
        {
            //敵スロー中じゃない場合
            if (!EnemyManager::Instance().GetIsSlow())
            {
                player_.lock()->GetJustAvoidPlayer()->StartJustAvoid();
                dashState_ = -1;
                isDashJudge_ = false;
                break;
            }
        }

        //空中の時は重力を0に
        if (!move->OnGround() && player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP_DASH)
            move->SetGravity(GRAVITY_ZERO);

        //最大速度に達したら次のステート
        dashStopTimer_ -= elapsedTime;
        if (length > dashMaxSpeed_ || dashStopTimer_ < 0)
        {
            dashState_++;
            player_.lock()->GetAttackPlayer()->SetIsNormalAttack(true);
        }

        break;
    case 3:
    {
        //加速度を下げていく
        float acce = move->GetMoveAcceleration();
        acce -= 10 * elapsedTime;
        if (acce < 0)
            acce = 0;
        move->SetMoveAcceleration(acce);

        if (moveParam_[MOVE_PARAM::RUN].moveMaxSpeed >= length)
            dashState_++;
        break;
    }
    case 4:
    {
        //走りに変更(ダッシュ終了)
        moveParamType_ = MOVE_PARAM::RUN;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
        dashState_ = -1;
        isDashJudge_ = false;
        isInputMove_ = true;

        //アニメーター
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("idle");

        //空中かの判定

        if (move->OnGround())
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
        else
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP);


        if (move->GetGravity() < 10.0f)
            move->SetGravity(GRAVITY_NORMAL);

        break;
    }

        //後ろ向きダッシュ
    case 10:
        //ダッシュに変更
        move->ZeroVelocity();
        moveParamType_ = MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);
        dashStopTimer_ = dashStopTime_;
        dashState_++;

        isInputMove_ = false;

        break;
    case 11:
    {
        //ジャスト回避判定
        if (player_.lock()->GetJustAvoidPlayer()->GetJustHitEnemy().lock())
        {
            //敵スロー中じゃない場合
            if (!EnemyManager::Instance().GetIsSlow())
            {
                player_.lock()->GetJustAvoidPlayer()->StartJustAvoid();
                dashState_ = -1;
                isDashJudge_ = false;
                break;
            }
        }

        //空中の時は重力を0に
        if (!move->OnGround())
            move->SetGravity(GRAVITY_ZERO);

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
        acce -= 30 * elapsedTime;
        if (acce < 0)
            acce = 0;
        move->SetMoveAcceleration(acce);

        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
        {
            dashState_ = 1;
            move->SetGravity(GRAVITY_NORMAL);

            //空中かの判定
            if (move->OnGround())
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
            else
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP);

        }

        if (moveParam_[MOVE_PARAM::RUN].moveMaxSpeed >= length)
            dashState_ = 3;
        break;
    }
    };
}

//走り切り替えし判定
void MovePlayer::RunTurnJudge()
{
    if (isRunTurn_)return;

    //空中だった場合は返す
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    if (!move->OnGround())return;

    //アニメーション
    std::shared_ptr<AnimationCom> animation = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
    if (animation->GetCurrentAnimationIndex() != ANIMATION_PLAYER::RUN_HARD_2)return;

    //プレイヤーの向き
    DirectX::XMFLOAT3 front = player_.lock()->GetGameObject()->transform_->GetWorldFront();

    //角度を求める
    float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&front), DirectX::XMLoadFloat3(&inputMoveVec_)));

    //後ろ向きだった場合
    if (dot < -0.7)
    {
        moveParamType_ = MOVE_PARAM::RUN;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);

        saveTurnVec_ = { inputMoveVec_.x,0,inputMoveVec_.z };
        isRunTurn_ = true;
        runTurnState_ = 0;
    }
}


//走り切り替えし処理
void MovePlayer::RunTurnMove()
{
    switch (runTurnState_)
    {
    case 0:
    {
        //アニメーター
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("runTurn");
        animator->SetAnimationSpeedOffset(1.5f);
        runTurnState_++;
        break;
    }
    case 1:
    {
        std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
        move->ZeroVelocity();
        runTurnState_++;
        break;
    }
    case 2:
    {
        //アニメーション
        std::shared_ptr<AnimationCom> animation = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
        if (!animation->IsPlayAnimation())
        {
            runTurnState_++;
        }
        else if (animation->GetCurrentAnimationIndex() != ANIMATION_PLAYER::RUN_TURN_FORWARD)
        {
            runTurnState_++;
        }

        QuaternionStruct inputRotato = QuaternionStruct::LookRotation(saveTurnVec_);
        QuaternionStruct playerRotato = player_.lock()->GetGameObject()->transform_->GetRotation();

        DirectX::XMStoreFloat4(&playerRotato.dxFloat4, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&playerRotato.dxFloat4), DirectX::XMLoadFloat4(&inputRotato.dxFloat4), 0.2f));

        player_.lock()->GetGameObject()->transform_->SetRotation(playerRotato);

        break;
    }
    case 3:
    {
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetAnimationSpeedOffset(1);

        runTurnState_ = -1;
        isRunTurn_ = false;

        break;
    }

    }

}

//強制的にターンを終わらせる
void MovePlayer::EndRunTurn()
{
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetAnimationSpeedOffset(1);

    runTurnState_ = -1;
    isRunTurn_ = false;
}

//強制的にダッシュを終わらせる（攻撃時等）
void MovePlayer::DashEndFlag(bool isWalk)   //歩きに切り替えるか
{
    isDashJudge_ = false;
    dashState_ = -1;
    player_.lock()->GetAttackPlayer()->SetIsNormalAttack(true);

    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    //重力初期化
    move->SetGravity(GRAVITY_NORMAL);

    if (moveParamType_ == MOVE_PARAM::DASH)
    {
        moveParamType_ = MOVE_PARAM::RUN;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
    }

    if (isWalk)
    {
        moveParamType_ = MOVE_PARAM::WALK;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
    }
}

//移動の属性セット
void MovePlayer::SetMoveParamType(MOVE_PARAM moveParam)
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    moveParamType_ = moveParam;
    move->SetMoveMaxSpeed(moveParam_[moveParam].moveMaxSpeed);
    move->SetMoveAcceleration(moveParam_[moveParam].moveAcceleration);
}
