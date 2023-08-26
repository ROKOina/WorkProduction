#include "PlayerCom.h"
#include "Input/Input.h"
#include <imgui.h>
#include "Components\CameraCom.h"
#include "Components\TransformCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"

#include "../Enemy/EnemyCom.h"
#include "../Weapon\WeaponCom.h"
#include "../CharacterStatusCom.h"

//アニメーションリスト
enum ANIMATION_PLAYER
{
    WALK_RUNRUN_1,
    IDEL_1,
    IDEL_2,
    JUMP_1,
    JUMP_2,
    RUN_HARD_1,
    RUN_HARD_2,
    RUN_SOFT_1,
    RUN_SOFT_2,
    WALK_RUNRUN_2,
    PUNCH,
    BIGSWORD_UP,
    BIGSWORD_LEFT,
    BIGSWORD_RIGHT,
    BIGSWORD_DOWN,
    DASH_ANIM,
    BIGSWORD_COM1_01,
    BIGSWORD_COM1_02,
    BIGSWORD_COM1_03,
    BIGSWORD_COM2_01,
    BIGSWORD_COM2_02,
    BIGSWORD_COM2_03,
    BIGSWORD_DASH,

};

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

    //当たり大きさ
    GetGameObject()->GetComponent<CapsuleColliderCom>()->SetRadius(0.17f);

    //移動パラメーター初期化
    moveParam_[MOVE_PARAM::WALK].moveMaxSpeed = 5.0f;
    moveParam_[MOVE_PARAM::WALK].moveAcceleration = 2.0f;
    moveParam_[MOVE_PARAM::WALK].turnSpeed = 4.0f;

    moveParam_[MOVE_PARAM::RUN].moveMaxSpeed = 8.0f;
    moveParam_[MOVE_PARAM::RUN].moveAcceleration = 2.0f;
    moveParam_[MOVE_PARAM::RUN].turnSpeed = 8.0f;

    moveParam_[MOVE_PARAM::JUSTDASH].moveMaxSpeed = 3.0f;
    moveParam_[MOVE_PARAM::JUSTDASH].moveAcceleration = 1.0f;
    moveParam_[MOVE_PARAM::JUSTDASH].turnSpeed = 4.0f;

    moveParam_[MOVE_PARAM::DASH].moveMaxSpeed = 20.0f;
    moveParam_[MOVE_PARAM::DASH].moveAcceleration = 6.0f;
    moveParam_[MOVE_PARAM::DASH].turnSpeed = 8.0f;

    moveParamType_ = MOVE_PARAM::WALK;
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
    move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
    move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);

    //武器ステータス初期化
    std::shared_ptr<WeaponCom> weapon = GetGameObject()->GetChildFind("greatSword")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_RIGHT, 1, 30,0.7f,0.3f);
    weapon->SetAttackStatus(BIGSWORD_UP, 1, 30, 0.2f, 0.8f);
    weapon->SetAttackStatus(BIGSWORD_COM1_01, 1, 15, 0.8f, 0.2f);
    weapon->SetAttackStatus(BIGSWORD_COM1_02, 1, 15, 0.3f, 0.7f);
    weapon->SetAttackStatus(BIGSWORD_COM1_03, 1, 15, 0.9f, 0.1f);
    weapon->SetAttackStatus(BIGSWORD_DASH, 1, 100, 0.9f, 0.1f);

    //攻撃管理を初期化
    attackPlayer_ = std::make_shared<AttackPlayer>(GetGameObject()->GetComponent<PlayerCom>());

    //アニメーション初期化
    AnimationInitialize();

}

// 更新処理
void PlayerCom::Update(float elapsedTime)
{
    //移動
    {
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        //移動が入力されていたら
        if (IsMove(elapsedTime))
        {
            //移動処理
            //回転
            if (isInputTrun_)
                Trun(elapsedTime);

            //ジャスト回避中は通らない
            if (isInputMove_)
            //if (!isJustJudge_)
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
        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetFloatValue("moveSpeed", length);
    }

    //ジャスト回避
    {
        //更新
        JustAvoidanceAttackUpdate(elapsedTime);
    }

    //攻撃
    {
        //攻撃更新
        AttackUpdate();

        attackPlayer_->Update(elapsedTime);

        ////アニメーション許可が出たらアニメーションする
        //if (attackPlayer_->DoAnimation())
        //{
        //    //アニメーター
        //    if (nextAnimName_.size() > 0) {
        //        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
        //        animator->SetTriggerOn(nextAnimName_);
        //        nextAnimName_ = "";
        //    }
        //}


        //当たり判定
        AttackJudgeCollision();
    }

    //カプセル当たり判定設定
    {
        std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
        //足元から頭までカプセルを設定
        capsule->SetPosition1({ 0,0,0 });
        Model::Node* headNode = GetGameObject()->GetComponent<RendererCom>()->GetModel()->FindNode("Head");
        DirectX::XMMATRIX PWorld = DirectX::XMLoadFloat4x4(&headNode->parent->worldTransform);
        DirectX::XMFLOAT3 pos;
        DirectX::XMStoreFloat3(&pos, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&headNode->translate), PWorld));
        DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetWorldPosition();
        capsule->SetPosition2({ pos.x - playerPos.x,pos.y - playerPos.y,pos.z - playerPos.z });
    }

    ////自分との当たり判定
    //std::vector<HitObj> hitGameObj = GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
    //for (auto& hitObj : hitGameObj)
    //{
    //    //名前検索
    //    //if (std::strcmp(hitObj.gameObject->GetName(), "picolabo") != 0)continue;
    //    //タグ検索
    //    //敵との当たり（仮）
    //    if (COLLIDER_TAG::Enemy == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
    //    {
    //        //押し返し
    //        DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetWorldPosition();
    //        DirectX::XMFLOAT3 hitPos = hitObj.gameObject->transform_->GetWorldPosition();

    //        DirectX::XMVECTOR PlayerPos = { playerPos.x,0,playerPos.z };
    //        DirectX::XMVECTOR HitPos = { hitPos.x,0, hitPos.z };

    //        DirectX::XMVECTOR ForceNormal = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(HitPos, PlayerPos));
    //        ForceNormal = DirectX::XMVectorScale(ForceNormal, 5);

    //        DirectX::XMFLOAT3 power;
    //        DirectX::XMStoreFloat3(&power, ForceNormal);

    //        hitObj.gameObject->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);
    //    }
    //}

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

    ImGui::InputInt("state", &dashState_);
    ImGui::InputInt("comboAttackCount_", &comboAttackCount_);

    ImGui::DragFloat("jumpSpeed", &jumpSpeed_, 0.1f);

    ImGui::DragFloat3("up", &up_.x);

    ImGui::Checkbox("dash", &isDashJudge_);

    //入力情報を取得
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();
    ImGui::DragFloat("x", &ax);
    ImGui::DragFloat("y", &ay);

    int stats = (int)playerStatus_;
    ImGui::InputInt("status", &stats);
}


#pragma region 移動系処理

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

    if (move->OnGround() && jumpCount_ < 2)
    {
        jumpCount_ = 2;
        playerStatus_ = PLAYER_STATUS::IDLE;
    }

    //ジャンプ
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_B)
    {
        if (jumpCount_ > 0)
        {
            //ジャスト回避中のみジャンプできない
            if (!isJustJudge_)
            {
                //攻撃と回避終了フラグ
                AttackFlagEnd();
                DashEndFlag();

                //アニメーター
                std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
                animator->SetTriggerOn("jump");
                inputMoveVec_.y = jumpSpeed_;

                move->ZeroVelocityY();
                --jumpCount_;

                playerStatus_ = PLAYER_STATUS::JUMP;
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
            if (playerStatus_ != PLAYER_STATUS::JUMP)
                if (comboAttackCount_ <= 0)
                    if (!isDashJudge_)
                    {
                        moveParamType_ = MOVE_PARAM::WALK;
                        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);
                        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);

                        playerStatus_ = PLAYER_STATUS::IDLE;
                    }
        }
        return false;
    }

    //状態更新
    if (playerStatus_ != PLAYER_STATUS::JUMP)
        if (!isDashJudge_ && !isJustJudge_ && comboAttackCount_ <= 0)
            playerStatus_ = PLAYER_STATUS::MOVE;

    return true;
}

//回転
void PlayerCom::Trun(float elapsedTime)
{
    //入力方向のクォータニオン生成
    QuaternionStruct inputQuaternion = QuaternionStruct::LookRotation(inputMoveVec_, up_);
    QuaternionStruct playerQuaternion = GetGameObject()->transform_->GetRotation();

    float worldSpeed = Graphics::Instance().GetWorldSpeed();

    //ゆっくり回転
    DirectX::XMFLOAT4 rota;
    DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&playerQuaternion.dxFloat4),
        DirectX::XMLoadFloat4(&inputQuaternion.dxFloat4), moveParam_[moveParamType_].turnSpeed * (elapsedTime * worldSpeed)));

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
    if (!isDash_)return;

    //ダッシュ
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER)
    {
        isDashJudge_ = true;        //ダッシュフラグON

        //アニメーター
        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();

        //ダッシュ後コンボ
        if (playerStatus_ == PLAYER_STATUS::ATTACK_DASH)
        {
            if (attackPlayer_->OnHitEnemy())
            {
                Graphics::Instance().SetWorldSpeed(1.0f);
                playerStatus_ = PLAYER_STATUS::DASH;

                animFlagName_ = "jump";
                attackPlayer_->DashAttack(2);
                animator->SetIsStop(true);
                return;
            }
        }

        //アタックリセット
        AttackFlagEnd();

        //入力があればstate=0
        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
        {
            dashState_ = 0;
            if (playerStatus_ == PLAYER_STATUS::JUMP)
                playerStatus_ = PLAYER_STATUS::JUMP_DASH;
            else
                playerStatus_ = PLAYER_STATUS::DASH;
        }
        else
        {
            dashState_ = 10;
            if (playerStatus_ == PLAYER_STATUS::JUMP)
                playerStatus_ = PLAYER_STATUS::JUMP_BACK_DASH;
            else
                playerStatus_ = PLAYER_STATUS::BACK_DASH;
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
void PlayerCom::DashStateUpdate(float elapsedTime)
{
    if (!isDashJudge_)return;

    //ジャスト回避の当たり判定
    std::vector<HitObj> hitGameObj = GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
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
        DirectX::XMFLOAT3 pPos = GetGameObject()->transform_->GetWorldPosition();
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


    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

    //ダッシュ更新
    switch (dashState_)
    {
        case 0:
        {
            //角度を変える
            QuaternionStruct dashDirection = QuaternionStruct::LookRotation(inputMoveVec_, up_);
            GetGameObject()->transform_->SetRotation(dashDirection);
        }
        [[fallthrough]];    //フォースルー(1にそのままいく)
        //入力方向ダッシュ
        case 1:
        {
            isInputMove_ = true;
            isNormalAttack_ = false;

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
                isJustJudge_ = true;
                justAvoidState_ = 0;
                dashState_ = -1;
                isDashJudge_ = false;
                justHitEnemy_ = enemy;
                break;
            }

            //最大速度に達したら次のステート
            dashStopTimer_ -= elapsedTime;
            if (length > dashMaxSpeed_ || dashStopTimer_ < 0)
            {
                dashState_++;
                isNormalAttack_ = true;
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
                isJustJudge_ = true;
                justAvoidState_ = 0;
                dashState_ = -1;
                isDashJudge_ = false;
                justHitEnemy_ = enemy;
                break;
            }

            DirectX::XMFLOAT3 front = GetGameObject()->transform_->GetWorldFront();
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

            DirectX::XMFLOAT3 front = GetGameObject()->transform_->GetWorldFront();
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
void PlayerCom::DashEndFlag()
{
    isDashJudge_ = false;
    dashState_ = -1;

    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
    moveParamType_ = MOVE_PARAM::WALK;
    move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);
    move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
}

#pragma endregion



#pragma region ジャスト回避

//ジャスト回避初期化
void PlayerCom::JustInisialize()
{
    justAvoidState_ = -1;
    isJustJudge_ = false;
}

//ジャスト回避反撃更新処理
void PlayerCom::JustAvoidanceAttackUpdate(float elapsedTime)
{
    //ジャスト回避判定時
    if (isJustJudge_)
    {
        //ジャスト回避移動処理
        JustAvoidanceMove(elapsedTime);

        //ジャスト回避後反撃入力確認
        JustAvoidanceAttackInput();
    }

    //反撃処理更新
    switch (justAvoidKey_)
    {
        //□反撃
    case PlayerCom::JUST_AVOID_KEY::SQUARE:
        JustAvoidanceSquare(elapsedTime);
        break;
    case PlayerCom::JUST_AVOID_KEY::TRIANGLE:
        break;
    }

}

//ジャスト回避移動処理
void PlayerCom::JustAvoidanceMove(float elapsedTime)
{
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

    switch (justAvoidState_)
    {
    case 0:
        //ジャスト回避に変更
        moveParamType_ = MOVE_PARAM::JUSTDASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::JUSTDASH].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::JUSTDASH].moveAcceleration);
        justAvoidTimer_ = justAvoidTime_;
        justAvoidState_++;

        isInputMove_ = false;
        isNormalAttack_ = false;
        isDash_ = false;

        //敵の方を向く
        GetGameObject()->transform_->LookAtTransform(justHitEnemy_->transform_->GetWorldPosition());
         
        playerStatus_ = PLAYER_STATUS::JUST;

        break;
    case 1:
    {
        //ジャスト回避終了タイマー
        justAvoidTimer_ -= elapsedTime;
        if (justAvoidTimer_ < 0)
        {
            JustInisialize();
            moveParamType_ = MOVE_PARAM::RUN;
            move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
            move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
            isInputMove_ = true;
            isNormalAttack_ = true;
            isDash_ = true;
            break;
        }

        //移動方向を決める（ジャスト回避中は勝手に移動する）
        DirectX::XMFLOAT3 Direction;
        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
        {
            Direction = { inputMoveVec_.x ,0,inputMoveVec_.z };
        }
        else
        {
            Direction = GetGameObject()->transform_->GetWorldFront();
            //yを消して逆向きにする
            Direction.x *= -1;
            Direction.y = 0;
            Direction.z *= -1;
            //正規化
            DirectX::XMStoreFloat3(&Direction, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&Direction)));
        }
        Direction.x *= moveParam_[MOVE_PARAM::JUSTDASH].moveSpeed;
        Direction.z *= moveParam_[MOVE_PARAM::JUSTDASH].moveSpeed;
        //力に加える
        move->AddForce(Direction);

        break;
    }
    }
}

//ジャスト回避反撃入力確認
void PlayerCom::JustAvoidanceAttackInput()
{
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

    //ボタンで反撃変える
    GamePad& gamePad = Input::Instance().GetGamePad();

    //□の場合
    if (gamePad.GetButtonDown() & GamePad::BTN_Y)
    {
        JustInisialize();
        moveParamType_ = MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);

        justAvoidKey_ = JUST_AVOID_KEY::SQUARE;

        isInputTrun_ = false;

        playerStatus_ = PLAYER_STATUS::ATTACK;

        //敵の方を向く
        GetGameObject()->transform_->LookAtTransform(justHitEnemy_->transform_->GetWorldPosition());
    }
}

//□反撃
void PlayerCom::JustAvoidanceSquare(float elapsedTime)
{
    //敵に接近する
    {
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

        DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&GetGameObject()->transform_->GetWorldPosition());
        DirectX::XMVECTOR EnemyPos = DirectX::XMLoadFloat3(&justHitEnemy_->transform_->GetWorldPosition());
        DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(EnemyPos, Pos);
        float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
        //敵の近くまで移動する
        if (length < 1.5f)
        {
            JustInisialize();
            moveParamType_ = MOVE_PARAM::RUN;
            move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
            move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
            justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
            isInputMove_ = true;
            isNormalAttack_ = true;
            isDash_ = true;

            //アタック処理に引き継ぐ
            animFlagName_ = "squareJust";
        }

        DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(PE);
        Dir = DirectX::XMVectorScale(Dir, moveParam_[MOVE_PARAM::DASH].moveSpeed);
        DirectX::XMFLOAT3 dir;
        DirectX::XMStoreFloat3(&dir, Dir);

        //力に加える
        move->AddForce(dir);

    }
}

#pragma endregion



#pragma region 攻撃

//攻撃更新
void PlayerCom::AttackUpdate()
{
    if (!isNormalAttack_)return;

    //アニメーター
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();

    //ダッシュコンボ入力にする
    if (playerStatus_ == PLAYER_STATUS::DASH && comboAttackCount_ == 2)
    {
        if (attackPlayer_->EndAttackState())
        {
            animator->SetIsStop(false);
            Graphics::Instance().SetWorldSpeed(0.3f);
        }
    }


    int currentAnimIndex = GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    //コンボ後処理
    {
        static int oldCount = 0;
        static int oldAnim = 0;
        if (comboAttackCount_ > 0) {
            //前フレームのコンボとアニメーションを見て
            //コンボ終了か見る
            if (oldCount == comboAttackCount_)
            {
                if (oldAnim != currentAnimIndex)
                {
                    //コンボ終了
                    AttackFlagEnd();
                }
            }
        }
        oldCount = comboAttackCount_;
        oldAnim = currentAnimIndex;
    }

    if (playerStatus_ == PLAYER_STATUS::ATTACK_DASH)
    {
        if (attackPlayer_->DoComboAttack())
        {
            isDash_ = true;
            if (attackPlayer_->OnHitEnemy())
            {
                Graphics::Instance().SetWorldSpeed(0.3f);
            }
        }
    }


    //引き継ぎ攻撃アニメーション
    if (animFlagName_.size() > 0)
    {
        animator->SetTriggerOn(animFlagName_);
        comboAttackCount_++;
        animFlagName_ = "";
    }
    else
    {
        GamePad& gamePad = Input::Instance().GetGamePad();
        if (gamePad.GetButtonDown() & GamePad::BTN_X)   //△
        {

            if (comboAttackCount_ == 0)
            {
                //ダッシュ攻撃か判定
                if (playerStatus_ == PLAYER_STATUS::DASH)
                {
                    //ダッシュ後コンボのためダッシュを出来なくする
                    isDash_ = false;
                    animator->SetTriggerOn("triangleDash");
                    playerStatus_ = PLAYER_STATUS::ATTACK_DASH;
                    attackPlayer_->DashAttack(1);

                    comboAttackCount_++;

                    //ダッシュ終了フラグ
                    DashEndFlag();
                }


                return;
            }
        }

        if (gamePad.GetButtonDown() & GamePad::BTN_Y)   //□
        {
            //今の状態で遷移を変える
            //if (currentAnimIndex != BIGSWORD_COM1_01) {
            if (comboAttackCount_ == 0)
            {
                //ダッシュ攻撃か判定
                if (playerStatus_ == PLAYER_STATUS::DASH)
                {
                    ////ダッシュ後コンボのためダッシュを出来なくする
                    //isDash_ = false;
                    //animator->SetTriggerOn("squareDash");
                    //playerStatus_ = PLAYER_STATUS::ATTACK_DASH;
                    //attackPlayer_->DashAttack();
                    return;
                }
                else
                {
                    animator->SetTriggerOn("squareIdle");
                    playerStatus_ = PLAYER_STATUS::ATTACK;
                    attackPlayer_->NormalAttack();
                }

                comboAttackCount_++;

                //ダッシュ終了フラグ
                DashEndFlag();


                return;
            }

            if (comboAttackCount_ >= 3)return;

            if (attackPlayer_->DoComboAttack())
            {
                if (playerStatus_ == PLAYER_STATUS::ATTACK)
                {
                    animator->SetTriggerOn("square");
                    attackPlayer_->NormalAttack();
                    comboAttackCount_++;

                    playerStatus_ = PLAYER_STATUS::ATTACK;
                    return;
                }
            }

            if (playerStatus_ == PLAYER_STATUS::DASH)
            {
                animator->SetTriggerOn("square");

                //仮で
                playerStatus_ = PLAYER_STATUS::ATTACK;
                Graphics::Instance().SetWorldSpeed(1.0f);
                comboAttackCount_++;

                return;
            }

            ////アニメーションと次の攻撃の種類を選択
            //nextAnimName_ = "squareIdle";
        }
    }

    //コンボ中処理
    if (comboAttackCount_ > 0)
    {
        isInputMove_ = false;
        isInputTrun_ = false;
    }

}

//攻撃当たり判定
void PlayerCom::AttackJudgeCollision()
{
    //子供の攻撃オブジェクトの当たり
    DirectX::XMFLOAT3 attackPos;
    std::shared_ptr<GameObject> attackChild = GetGameObject()->GetChildFind("picoAttack");
    //AnimationEvent取得して当たり判定をセット
    if (GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("attackPunch", attackPos)) {
        attackChild->GetComponent<Collider>()->SetEnabled(true);
        attackChild->transform_->SetWorldPosition(attackPos);
    }
    else
    {
        attackChild->GetComponent<Collider>()->SetEnabled(false);
    }

    //子供の攻撃オブジェクトの当たり判定
    std::vector<HitObj> hitAttack = attackChild->GetComponent<Collider>()->OnHitGameObject();
    for (auto& hitObj : hitAttack)
    {
        //敵に攻撃があたったら
        if (COLLIDER_TAG::Enemy == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
        {
            std::shared_ptr<EnemyCom> enemy = hitObj.gameObject->GetComponent<EnemyCom>();
            std::shared_ptr<CharacterStatusCom> enemyStatus = hitObj.gameObject->GetComponent<CharacterStatusCom>();
            //無敵中は攻撃できない
            if (enemyStatus->GetIsInvincible())continue;

            DirectX::XMFLOAT3 powerForce = { 0,0,0 };
            float power = 30;

            //プレイヤーの前方向に飛ばす
            DirectX::XMFLOAT3 playerForward = GetGameObject()->transform_->GetWorldFront();
            powerForce.x = playerForward.x * power;
            powerForce.z = playerForward.z * power;


            enemyStatus->OnDamage(powerForce);
        }
    }
}

//強制的に攻撃を終わらせる（ジャンプ時等）
void PlayerCom::AttackFlagEnd()
{
    comboAttackCount_ = 0;
    isInputMove_ = true;
    isInputTrun_ = true;
    attackPlayer_->ResetState();

    if (playerStatus_ == PLAYER_STATUS::ATTACK_DASH ||
        playerStatus_ == PLAYER_STATUS::DASH)
    {
        Graphics::Instance().SetWorldSpeed(1.0f);
    }

    playerStatus_ = PLAYER_STATUS::IDLE;
}


#pragma endregion



//アニメーション初期化設定
void PlayerCom::AnimationInitialize()
{
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //初期のアニメーション
    animator->SetFirstTransition(ANIMATION_PLAYER::IDEL_2);
    animator->SetLoopAnimation(ANIMATION_PLAYER::IDEL_2, true);

    //アニメーションパラメーター追加
    animator->AddTriggerParameter("jump");
    animator->AddTriggerParameter("punch");
    animator->AddTriggerParameter("dash");

    animator->AddTriggerParameter("square");    //□
    animator->AddTriggerParameter("triangle");  //△
    //入力無し
    animator->AddTriggerParameter("squareIdle");
    animator->AddTriggerParameter("squareDash");
    animator->AddTriggerParameter("triangleIdle");
    animator->AddTriggerParameter("triangleDash");
    //ジャスト時
    animator->AddTriggerParameter("squareJust");
    animator->AddTriggerParameter("triangleJust");

    animator->AddFloatParameter("moveSpeed");

    //アニメーション遷移とパラメーター設定をする決める
    {
        //idle
        animator->AddAnimatorTransition(IDEL_2, WALK_RUNRUN_2);
        animator->SetFloatTransition(IDEL_2, WALK_RUNRUN_2,
            "moveSpeed", 0.1f, PATAMETER_JUDGE::GREATER);

        //walk
        animator->AddAnimatorTransition(WALK_RUNRUN_2, IDEL_2);
        animator->SetFloatTransition(WALK_RUNRUN_2, IDEL_2,
            "moveSpeed", 0.1f, PATAMETER_JUDGE::LESS);
        animator->SetLoopAnimation(WALK_RUNRUN_2, true);

        animator->AddAnimatorTransition(WALK_RUNRUN_2, RUN_HARD_2);
        animator->SetFloatTransition(WALK_RUNRUN_2, RUN_HARD_2,
            "moveSpeed", moveParam_[MOVE_PARAM::WALK].moveMaxSpeed + 1, PATAMETER_JUDGE::GREATER);

        //run
        animator->AddAnimatorTransition(RUN_HARD_2, WALK_RUNRUN_2);
        animator->SetFloatTransition(RUN_HARD_2, WALK_RUNRUN_2,
            "moveSpeed", moveParam_[MOVE_PARAM::WALK].moveMaxSpeed + 1, PATAMETER_JUDGE::LESS);
        animator->SetLoopAnimation(RUN_HARD_2, true);

        {   //dashコンボ
            //dash切り
            animator->AddAnimatorTransition(BIGSWORD_DASH);
            animator->SetTriggerTransition(BIGSWORD_DASH, "triangleDash");
            animator->AddAnimatorTransition(BIGSWORD_DASH, IDEL_2, true, 3.5f);

            //コンボ3
            animator->AddAnimatorTransition(JUMP_2, BIGSWORD_COM1_01);
            animator->SetTriggerTransition(JUMP_2, BIGSWORD_COM1_01, "square");
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, IDEL_2, true, 3.5f);


        }

        //どこからでも遷移する
        //jump
        animator->AddAnimatorTransition(JUMP_2);
        animator->SetTriggerTransition(JUMP_2, "jump");
        animator->AddAnimatorTransition(JUMP_2, IDEL_2, true);

        //punch
        animator->AddAnimatorTransition(PUNCH);
        animator->SetTriggerTransition(PUNCH, "punch");
        animator->AddAnimatorTransition(PUNCH, IDEL_2, true);

        //dash
        animator->AddAnimatorTransition(DASH_ANIM);
        animator->SetTriggerTransition(DASH_ANIM, "dash");
        animator->AddAnimatorTransition(DASH_ANIM, IDEL_2, true);

        {   //□□□
            //combo01
            animator->AddAnimatorTransition(BIGSWORD_COM1_01);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, "squareIdle");
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, IDEL_2, true, 3.5f);
            //combo2に行く
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, BIGSWORD_COM1_02);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, BIGSWORD_COM1_02, "square");

            //combo02
            animator->AddAnimatorTransition(BIGSWORD_COM1_02);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, "squareJust");
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, IDEL_2, true, 3.5f);
            //combo3に行く
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, BIGSWORD_COM1_03);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, BIGSWORD_COM1_03, "square");

            //combo03
            animator->AddAnimatorTransition(BIGSWORD_COM1_03);
            //animator->SetTriggerTransition(BIGSWORD_COM1_03, "squareJust");
            animator->AddAnimatorTransition(BIGSWORD_COM1_03, IDEL_2, true, 3.5f);
        }

    }

}
