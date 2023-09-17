#include "AttackPlayer.h"

#include "PlayerCom.h"
#include "Components\ColliderCom.h"
#include "Components\TransformCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "SystemStruct\QuaternionStruct.h"
#include "../Weapon\WeaponCom.h"
#include "Graphics/Graphics.h"
#include "Input/Input.h"


void AttackPlayer::Update(float elapsedTime)
{
    AttackInputUpdate(elapsedTime);

    AttackMoveUpdate(elapsedTime);
}

//攻撃入力処理
void AttackPlayer::AttackInputUpdate(float elapsedTime)
{
    if (!isNormalAttack_)return;

    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //ダッシュコンボ入力にする
    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH && comboAttackCount_ == 2)
    {
        if (EndAttackState())
        {
            animator->SetIsStop(false);
            Graphics::Instance().SetWorldSpeed(0.3f);
            player_.lock()->GetMovePlayer()->isDash_ = true;
        }
    }

    int currentAnimIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

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

                    player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
                }
            }
        }
        oldCount = comboAttackCount_;
        oldAnim = currentAnimIndex;
    }

    //ダッシュアタック時の処理
    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH)
    {
        if (DoComboAttack())
        {
            //ダッシュ有効か
            player_.lock()->GetMovePlayer()->isDash_ = true;
            //当たっていた時
            if (OnHitEnemy() && ComboReadyEnemy())
            {
                Graphics::Instance().SetWorldSpeed(0.3f);
                player_.lock()->GetMovePlayer()->dashCoolTimer_ = 0;
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
                if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
                {
                    //ダッシュ後コンボのためダッシュを出来なくする
                    player_.lock()->GetMovePlayer()->isDash_ = false;
                    animator->SetTriggerOn("triangleDash");
                    player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_DASH);
                    DashAttack(1);

                    comboAttackCount_++;

                    //ダッシュ終了フラグ
                    player_.lock()->GetMovePlayer()->DashEndFlag();
                }


                return;
            }
        }

        if (gamePad.GetButtonDown() & GamePad::BTN_Y)   //□
        {
            //今の状態で遷移を変える
            if (comboAttackCount_ == 0)
            {
                //ダッシュ攻撃か判定
                if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
                {
                    return;
                }
                else
                {
                    animator->SetTriggerOn("squareIdle");
                    player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);
                    NormalAttack();
                }

                comboAttackCount_++;

                //ダッシュ終了フラグ
                player_.lock()->GetMovePlayer()->DashEndFlag();

                return;
            }

            if (comboAttackCount_ >= 3)return;

            if (DoComboAttack())
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

            ////アニメーションと次の攻撃の種類を選択
            //nextAnimName_ = "squareIdle";
        }
    }

    //コンボ中処理
    if (comboAttackCount_ > 0)
    {
        player_.lock()->GetMovePlayer()->isInputMove_ = false;
        player_.lock()->GetMovePlayer()->isInputTrun_ = false;
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


    //攻撃が当たっているか見る
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
    //武器ステータス初期化
    std::shared_ptr<WeaponCom> weapon = playerObj->GetChildFind("greatSword")->GetComponent<WeaponCom>();

    //一回でも攻撃が当たっているなら次の攻撃が来るまで、trueにする
    if (weapon->GetOnHit())
    {
        onHitEnemy_ = true;
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

        if (ApproachEnemy(enemyCopy_, 1.5f, 2))
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
        if (COLLIDER_TAG::Enemy == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
        {
            //最初はそのまま入れる
            if (!enemyNearObj)enemyNearObj = hitObj.gameObject;
            //一番近い敵を見つける
            else
            {
                DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemyNearObj->transform_->GetWorldPosition());
                DirectX::XMVECTOR EN = DirectX::XMLoadFloat3(&hitObj.gameObject->transform_->GetWorldPosition());
                DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());

                DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(E, P);
                DirectX::XMVECTOR PEN = DirectX::XMVectorSubtract(EN, P);

                float lenE = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
                float lenEN = DirectX::XMVectorGetX(DirectX::XMVector3Length(PEN));
                if (lenE > lenEN)enemyNearObj = hitObj.gameObject;
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
    peQ = QuaternionStruct::LookRotation(peR);

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

//強制的に攻撃を終わらせる（ジャンプ時等）
void AttackPlayer::AttackFlagEnd()
{
    comboAttackCount_ = 0;
    player_.lock()->GetMovePlayer()->isInputMove_ = true;
    player_.lock()->GetMovePlayer()->isInputTrun_ = true;
    ResetState();
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetAnimationSpeedOffset(1);

    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH ||
        player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
    {
        Graphics::Instance().SetWorldSpeed(1.0f);
    }

    //playerStatus_ = PLAYER_STATUS::IDLE;
}
