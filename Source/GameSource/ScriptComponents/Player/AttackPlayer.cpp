#include "AttackPlayer.h"

#include "PlayerCom.h"
#include "Components\ColliderCom.h"
#include "Components\RendererCom.h"
#include "Components\TransformCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "SystemStruct\QuaternionStruct.h"
#include "../Weapon\WeaponCom.h"
#include "Graphics/Graphics.h"
#include "Input/Input.h"
#include <imgui.h>


void AttackPlayer::Update(float elapsedTime)
{
    //コンボ継続確認処理
    ComboJudge();
    
    //先行入力時はtrue
    bool isIeadInput = false;
    if (attackLeadInputKey_ != ATTACK_KEY::NULL_KEY)
    {
        if (DoComboAttack())
            isIeadInput = true;
    }


    //入力を見て処理をする
    if (IsAttackInput(elapsedTime)||isIeadInput)
    {
        if (isIeadInput)
        {
            attackLeadInputKey_ = ATTACK_KEY::NULL_KEY;
        }

        //アニメインデックスで、コンボカウントリセット
        AttackComboCountReset();

        switch (attackKey_)
        {
        case AttackPlayer::ATTACK_KEY::SQUARE:
            //ブレイク処理
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP_FALL)break;
            if (!isSquareAttack_)break;
            SquareInput();
            break;
        case AttackPlayer::ATTACK_KEY::TRIANGLE:
            //ブレイク処理
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP_FALL)break;
            TriangleInput();
            break;
        case AttackPlayer::ATTACK_KEY::DASH:
            DashInput();
            break;
        case AttackPlayer::ATTACK_KEY::NULL_KEY:
            break;
        }

    }

    //コンボ処理
    ComboProcess(elapsedTime);

    //攻撃中の動き
    AttackMoveUpdate(elapsedTime);
}

void AttackPlayer::OnGui()
{
    ImGui::InputInt("comboCount", &comboAttackCount_, ImGuiInputTextFlags_ReadOnly);
    ImGui::Checkbox("isSquareAttack", &isSquareAttack_);
}

//コンボ継続確認処理
void AttackPlayer::ComboJudge()
{
    //現在のアニメインデックス
    int currentAnimIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    //コンボ継続判定処理
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

                    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

                    //空中かの判定
                    if (playerObj->GetComponent<MovementCom>()->OnGround())
                    {
                        //プレイヤーステータスの更新
                        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
                    }
                    else
                    {
                        //プレイヤーステータスの更新
                        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP);
                        //□攻撃できなくする
                        if (isJumpSquareInput_)
                        {
                            isSquareAttack_ = false;
                            isJumpSquareInput_ = false;
                        }
                    }


                }
            }
        }
        oldCount = comboAttackCount_;
        oldAnim = currentAnimIndex;
    }

    //コンボ中処理
    if (comboAttackCount_ > 0)
    {
        player_.lock()->GetMovePlayer()->SetIsInputMove(false);
        //player_.lock()->GetMovePlayer()->SetIsInputTurn(false);
    }
}

//入力情報を保存
bool AttackPlayer::IsAttackInput(float elapsedTime)
{
    //先行入力時は上書きする
    if (attackLeadInputKey_ == ATTACK_KEY::NULL_KEY)
        attackKey_ = ATTACK_KEY::NULL_KEY;
    else
        attackKey_ = attackLeadInputKey_;

    if (!isNormalAttack_)return false;

    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //引き継ぎ攻撃アニメーション
    if (animFlagName_.size() > 0)
    {
        animator->SetTriggerOn(animFlagName_);
        comboAttackCount_++;
        animFlagName_ = "";

        //今のところ、ここで処理は終了にしておく
        return false;
    }
    else
    {
        GamePad& gamePad = Input::Instance().GetGamePad();
        if (gamePad.GetButtonDown() & GamePad::BTN_Y)   //□
        {
            attackKey_ = ATTACK_KEY::SQUARE;
            return true;
        }

        if (gamePad.GetButtonDown() & GamePad::BTN_X)   //△
        {
            attackKey_ = ATTACK_KEY::TRIANGLE;
            return true;
        }

        if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER)   //DASH
        {
            attackKey_ = ATTACK_KEY::DASH;
            return true;
        }
    }

    return false;
}

//□入力時処理
void AttackPlayer::SquareInput()
{
    //攻撃中は先行入力に保存
    if (InAttackJudgeNow())
    {
        attackLeadInputKey_ = ATTACK_KEY::SQUARE;
        return;
    }

    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //今の状態で遷移を変える
    if (comboAttackCount_ == 0)
    {
        attackLeadInputKey_ = ATTACK_KEY::NULL_KEY;

        std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
        if (!playerObj->GetComponent<MovementCom>()->OnGround())
        {
            ////ジャンプ中の時
            //if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP)
            //{
            animator->SetTriggerOn("squareJump");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);

            isJumpSquareInput_ = true;
                //return;
            //}
        }
        else
        {
            //ダッシュ攻撃か判定
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
            {
                return;
            }
            else
            {
                animator->ResetParameterList();
                animator->SetTriggerOn("squareIdle");
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);
                NormalAttack();
            }
        }

        comboAttackCount_++;

        //ダッシュ終了フラグ
        player_.lock()->GetMovePlayer()->DashEndFlag();

        return;
    }

    if (comboAttackCount_ >= 3)return;

    if (DoComboAttack())    //コンボ受付
    {
        std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
        if (!playerObj->GetComponent<MovementCom>()->OnGround())
        {
            animator->SetTriggerOn("square");
            comboAttackCount_++;

            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);
            return;
        }
        else
        {
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK)
            {
                animator->SetTriggerOn("square");
                NormalAttack();
                comboAttackCount_++;

                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);
                return;
            }
        }
    }

    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
    {
        if (animator->GetIsStop())return;
        animator->SetTriggerOn("squareIdle");

        //仮で
        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);
        Graphics::Instance().SetWorldSpeed(1.0f);
        comboAttackCount_ = 1;

        return;
    }
}

//△入力時処理
void AttackPlayer::TriangleInput()
{
    //攻撃中は先行入力に保存
    if (InAttackJudgeNow())
    {
        attackLeadInputKey_ = ATTACK_KEY::TRIANGLE;
        return;
    }

    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //ジャンプ中切りおろし
    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP
        || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP_BACK_DASH
        || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP_DASH)
    {
        animator->ResetParameterList();
        animator->SetTriggerOn("triangleJumpDown");
        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);

        player_.lock()->GetGameObject()->GetComponent<MovementCom>()->ZeroVelocity();

        //切りおろし中は移動不可に
        player_.lock()->GetMovePlayer()->SetIsInputMove(false);
        player_.lock()->GetMovePlayer()->SetIsInputTurn(false);
        player_.lock()->GetMovePlayer()->DashEndFlag();
        isJumpFall_ = true;
    }


    if (comboAttackCount_ == 0)
    {
        attackLeadInputKey_ = ATTACK_KEY::NULL_KEY;

        //ダッシュ攻撃か判定
        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
        {
            //ダッシュ後コンボのためダッシュを出来なくする
            player_.lock()->GetMovePlayer()->SetIsDash(false);
            animator->SetTriggerOn("triangleDash");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_DASH);
            DashAttack(1);

            comboAttackCount_++;

            //ダッシュ終了フラグ
            player_.lock()->GetMovePlayer()->DashEndFlag();
        }
        //ジャンプ攻撃
        else if(player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::IDLE
            || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::MOVE)
        {
            animator->SetTriggerOn("triangleJump");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);

            //ジャンプカウント減らす
            player_.lock()->GetMovePlayer()->SetJumpCount(player_.lock()->GetMovePlayer()->GetJumpCount() - 1);

            comboAttackCount_++;
        }

        return;
    }


}

//Dash入力時処理
void AttackPlayer::DashInput()
{
    //ダッシュ後コンボ
    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH)
    {
        if (DoComboAttack())    //コンボ受付
        {
            if (OnHitEnemy() && ComboReadyEnemy())
            {
                Graphics::Instance().SetWorldSpeed(1.0f);
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::DASH);

                //アニメーター
                std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

                //次の攻撃アニメーションフラグを渡す
                animFlagName_ = "dashComboWait";
                animator->ResetParameterList();

                DashAttack(2);
                animator->SetIsStop(true);

                //一度透明に
                player_.lock()->GetGameObject()->GetComponent<RendererCom>()->SetEnabled(false);

                return;
            }
        }
    }
}

//コンボ処理
void AttackPlayer::ComboProcess(float elapsedTime)
{
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();


    //ダッシュコンボ処理
    {
        //ダッシュアタック時の処理(ダッシュに続く)
        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH)
        {
            if (DoComboAttack())    //コンボ受付
            {
                //当たっていた時
                if (OnHitEnemy() && ComboReadyEnemy())
                {
                    Graphics::Instance().SetWorldSpeed(0.3f);
                }
            }
        }

        //ダッシュコンボ入力にする
        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH && comboAttackCount_ == 2)
        {
            if (EndAttackState())
            {
                //透明解除
                player_.lock()->GetGameObject()->GetComponent<RendererCom>()->SetEnabled(true);

                animator->SetIsStop(false);
                Graphics::Instance().SetWorldSpeed(0.3f);
                player_.lock()->GetMovePlayer()->SetIsDash(true);
            }
        }
    }

    //ジャンプ攻撃更新処理
    {
        std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

        //重力設定
        static bool isSetGravity = false;
        //アニメーションイベントを見て重力を設定する
        if (player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("ZeroGravity",DirectX::XMFLOAT3())
            && player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP)
        {
            playerObj->GetComponent<MovementCom>()->ZeroVelocityY();
            playerObj->GetComponent<MovementCom>()->SetGravity(GRAVITY_ZERO);
            isSetGravity = true;
        }
        else if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP_FALL)    //切り下ろし攻撃時
        {
            playerObj->GetComponent<MovementCom>()->SetGravity(GRAVITY_FALL);
            isSetGravity = true;
        }
        else
        {
            if (isSetGravity)   //ここで変えた時だけ反応するように
            {
                playerObj->GetComponent<MovementCom>()->SetGravity(GRAVITY_NORMAL);
                isSetGravity = false;
            }
        }

        int index = playerObj->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();
        //切り下ろし処理
        if (index == ANIMATION_PLAYER::JUMP_ATTACK_DOWN_DO)
        {
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP_FALL);
        }

        //切り下ろし後処理
        static bool isJumpAttackEnd = false;
        if (index == ANIMATION_PLAYER::JUMP_ATTACK_DOWN_END && isJumpFall_)
        {
            isJumpFall_ = false;
            player_.lock()->GetMovePlayer()->SetIsInputMove(false);
            isJumpAttackEnd = true;
        }
        else if (index != ANIMATION_PLAYER::JUMP_ATTACK_DOWN_END && isJumpAttackEnd)
        {
            player_.lock()->GetMovePlayer()->SetIsInputMove(true);
            isJumpAttackEnd = false;
        }

        ////空中攻撃中に地面に着いた時の処理
        //if (playerObj->GetComponent<MovementCom>()->OnGround() && player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP)
        //{
        //    //animator->SetTriggerOn("idle");
        //}
    }
}



//攻撃動き処理
void AttackPlayer::AttackMoveUpdate(float elapsedTime)
{
    //state初期化
    if (EndAttackState())state_ = -1;

    //攻撃の種類によって動きを変える
    switch (attackFlagState_)
    {
    case ATTACK_FLAG::Normal:
        if (NormalAttackUpdate(elapsedTime) == ATTACK_CODE::EnterAttack)
            attackFlagState_ = ATTACK_FLAG::Null;
        break;
    case ATTACK_FLAG::Dash:
        if (DashAttackUpdate(elapsedTime) == ATTACK_CODE::EnterAttack)
            attackFlagState_ = ATTACK_FLAG::Null;
        break;
    }


    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    //武器取得
    std::shared_ptr<WeaponCom> weapon = playerObj->GetChildFind("greatSword")->GetComponent<WeaponCom>();

    //一回でも攻撃が当たっているなら次の攻撃が来るまで、trueにする
    if (weapon->GetOnHit())
    {
        onHitEnemy_ = true;
    }


    //攻撃中の時
    if (comboAttackCount_ > 0)
    {
        //回転不能
        player_.lock()->GetMovePlayer()->SetIsInputTurn(false);

        //攻撃フレーム前だけ回転可能に
        std::shared_ptr<AnimationCom> animCom = playerObj->GetComponent<AnimationCom>();
        for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
        {
            //頭がAutoCollisionなら
            if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;

            //アニメーションイベントのスタートフレーム前なら回転可能
            if (!animCom->GetCurrentAnimationEventIsStart(animEvent.name.c_str()))continue;

            //回転可能
            player_.lock()->GetMovePlayer()->SetIsInputTurn(true);
        }
    }


}

//アニメインデックスで、コンボカウントリセット
void AttackPlayer::AttackComboCountReset()
{
    //現在のアニメインデックス
    int currentAnimIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    if (currentAnimIndex == ANIMATION_PLAYER::JUMP_ATTACK_01)
    {
        comboAttackCount_ = 1;
    }
}


void AttackPlayer::NormalAttack()
{
    state_ = 0;
    onHitEnemy_ = false;    //攻撃が入力されたらfalseに
    attackFlagState_ = ATTACK_FLAG::Normal;
}

int AttackPlayer::NormalAttackUpdate(float elapsedTime)
{
    switch (state_)
    {
    case 0:
        //範囲内に敵はいるか
        enemyCopy_ = AssistGetNearEnemy();
        if (!enemyCopy_)
        {
            state_ = ATTACK_CODE::EnterAttack;
            break;
        }
        state_++;
        break;

    case 1:
        //回転
        if (ForcusEnemy(elapsedTime, enemyCopy_, 10))
            state_++;
       break;

    case 2:
        //接近
        if (ApproachEnemy(enemyCopy_, 1.5f))
            state_ = ATTACK_CODE::EnterAttack;
        break;
    }

    return state_;
}

void AttackPlayer::DashAttack(int comboNum)
{
    if (comboNum == 1)
        state_ = 0;
    if (comboNum == 2)
        state_ = 10;
    onHitEnemy_ = false;    //攻撃が入力されたらfalseに
    attackFlagState_ = ATTACK_FLAG::Dash;
}

int AttackPlayer::DashAttackUpdate(float elapsedTime)
{
    switch (state_)
    {
        //コンボ１
    case 0:
        //範囲内に敵はいるか
        enemyCopy_ = AssistGetNearEnemy();
        if (!enemyCopy_)
        {
            state_ = ATTACK_CODE::EnterAttack;
            break;
        }
        state_++;
        break;

    case 1:
        //回転
        if (ForcusEnemy(elapsedTime, enemyCopy_, 10))
            state_ = ATTACK_CODE::EnterAttack;
        break;

        //コンボ２
    case 10:
        //接近
        if (!enemyCopy_)
        {
            state_ = ATTACK_CODE::EnterAttack;
            break;
        }

        if (ApproachEnemy(enemyCopy_, 1.5f, 20))
            state_ = ATTACK_CODE::EnterAttack;
        break;
    }

    return state_;
}


//コンボ出来るか判定
bool AttackPlayer::DoComboAttack()
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    std::shared_ptr<AnimationCom> animCom = playerObj->GetComponent<AnimationCom>();
    for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
    {
        //頭がAutoCollisionなら
        if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;
        //アニメーションイベントのエンドフレーム後ならコンボ可能
        if (!animCom->GetCurrentAnimationEventIsEnd(animEvent.name.c_str()))continue;

        return true;
    }
    return false;
}

//攻撃判定中か判定
bool AttackPlayer::InAttackJudgeNow()
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    std::shared_ptr<AnimationCom> animCom = playerObj->GetComponent<AnimationCom>();
    for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
    {
        //頭がAutoCollisionなら
        if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;
        //アニメーションイベント外ならfalse
        if (!animCom->GetCurrentAnimationEvent(animEvent.name.c_str(), DirectX::XMFLOAT3()))continue;

        return true;
    }
    return false;
}

//アシスト範囲を索敵して近い敵を返す
std::shared_ptr<GameObject> AttackPlayer::AssistGetNearEnemy()
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
    std::shared_ptr<GameObject> enemyNearObj;   //一番近い敵を入れる

    //アシスト判定取得
    std::shared_ptr<Collider> assistColl = playerObj->GetChildFind("attackAssist")->GetComponent<Collider>();
    std::vector<HitObj> hitGameObj = assistColl->OnHitGameObject();
    for (auto& hitObj : hitGameObj)
    {
        if (COLLIDER_TAG::Enemy == hitObj.gameObject.lock()->GetComponent<Collider>()->GetMyTag())
        {
            //最初はそのまま入れる
            if (!enemyNearObj)enemyNearObj = hitObj.gameObject.lock();
            //一番近い敵を見つける
            else
            {
                DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemyNearObj->transform_->GetWorldPosition());
                DirectX::XMVECTOR EN = DirectX::XMLoadFloat3(&hitObj.gameObject.lock()->transform_->GetWorldPosition());
                DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());

                DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(E, P);
                DirectX::XMVECTOR PEN = DirectX::XMVectorSubtract(EN, P);

                float lenE = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
                float lenEN = DirectX::XMVectorGetX(DirectX::XMVector3Length(PEN));
                if (lenE > lenEN)enemyNearObj = hitObj.gameObject.lock();
            }
        }
    }

    //敵がいない場合return
    return enemyNearObj;
}

//敵に接近する( true:接近完了　false:接近中 )
bool AttackPlayer::ApproachEnemy(std::shared_ptr<GameObject> enemy, float dist, float speed)
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());
    DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->transform_->GetWorldPosition());

    DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(E, P);
    //距離が縮まればtrue
    if (dist > DirectX::XMVectorGetX(DirectX::XMVector3Length(PE)))
    {
        playerObj->GetComponent<MovementCom>()->ZeroNonMaxSpeedVelocity();
        return true;
    }

    //距離を詰める
    PE = DirectX::XMVector3Normalize(PE);
    DirectX::XMFLOAT3 vec;
    DirectX::XMStoreFloat3(&vec, DirectX::XMVectorScale(PE, speed));
    
    //プレイヤー移動
    playerObj->GetComponent<MovementCom>()->AddNonMaxSpeedForce(vec);
    return false;
}


//敵の方向へ回転する ( true:完了 )
bool AttackPlayer::ForcusEnemy(float elapsedTime, std::shared_ptr<GameObject> enemy, float rollSpeed)
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());
    DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->transform_->GetWorldPosition());

    //プレイヤーからエネミーの方向のクォータニオンを取得
    DirectX::XMFLOAT3 peR;
    DirectX::XMStoreFloat3(&peR, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(E, P)));
    QuaternionStruct peQ;
    peQ = QuaternionStruct::LookRotation({ peR.x,0,peR.z });

    DirectX::XMVECTOR PEQ = DirectX::XMLoadFloat4(&peQ.dxFloat4);
    DirectX::XMVECTOR PQ = DirectX::XMLoadFloat4(&playerObj->transform_->GetRotation());

    float rot = DirectX::XMVectorGetX(DirectX::XMQuaternionDot(PQ, PEQ));

    if (1 - rot * rot < 0.01f)return true;

    //プレイヤー向きから、エネミーの方向へ補間する
    PQ = DirectX::XMQuaternionSlerp(PQ, PEQ, rollSpeed * elapsedTime);

    QuaternionStruct playerQuaternion;
    DirectX::XMStoreFloat4(&playerQuaternion.dxFloat4, PQ);

    playerObj->transform_->SetRotation(playerQuaternion);

    return false;
}

//着地時攻撃処理
void AttackPlayer::AttackOnGround()
{
    isSquareAttack_ = true;
}

//ジャンプ時攻撃処理
void AttackPlayer::AttackJump()
{
    isSquareAttack_ = true;
}

//強制的に攻撃を終わらせる（ジャンプ時等）
void AttackPlayer::AttackFlagEnd()
{
    comboAttackCount_ = 0;
    player_.lock()->GetMovePlayer()->SetIsInputMove(true);
    player_.lock()->GetMovePlayer()->SetIsInputTurn(true);
    player_.lock()->GetMovePlayer()->SetIsDash(true);
    ResetState();
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetAnimationSpeedOffset(1);

    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH ||
        player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
    {
        Graphics::Instance().SetWorldSpeed(1.0f);
    }
}
