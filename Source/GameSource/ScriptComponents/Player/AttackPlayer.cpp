#include "AttackPlayer.h"

#include "PlayerCom.h"
#include "Components\ColliderCom.h"
#include "Components\TransformCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "SystemStruct\QuaternionStruct.h"

AttackPlayer::AttackPlayer(std::shared_ptr<PlayerCom> player)
    :player(player)
{

}

AttackPlayer::~AttackPlayer()
{

}

void AttackPlayer::Update(float elapsedTime)
{
    //state初期化
    if (DoAnimation())state = -1;

    //攻撃の種類によって動きを変える
    switch (attackFlagState)
    {
    case ATTACK_FLAG::Normal:
        if (NormalAttackUpdate(elapsedTime) == ATTACK_CODE::EnterAttack)
            attackFlagState = ATTACK_FLAG::Null;
        break;
    }
}

void AttackPlayer::NormalAttack()
{
    state = 0;
    attackFlagState = ATTACK_FLAG::Normal;
}

int AttackPlayer::NormalAttackUpdate(float elapsedTime)
{
    switch (state)
    {
    case 0:
        //範囲内に敵はいるか
        enemyCopy = AssistGetNearEnemy();
        if (!enemyCopy)
        {
            state = ATTACK_CODE::EnterAttack;
            break;
        }
        state++;
        break;

    case 1:
        //回転
        if (ForcusEnemy(elapsedTime, enemyCopy, 10))
            state++;
       break;

    case 2:
        //接近
        if (ApproachEnemy(enemyCopy, 1.5f))
            state = ATTACK_CODE::EnterAttack;
        break;
    }

    return state;
}


//コンボ出来るか判定
bool AttackPlayer::DoComboAttack()
{
    std::shared_ptr<GameObject> playerObj = player.lock()->GetGameObject();

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
    std::shared_ptr<GameObject> playerObj = player.lock()->GetGameObject();
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
    std::shared_ptr<GameObject> playerObj = player.lock()->GetGameObject();

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
    std::shared_ptr<GameObject> playerObj = player.lock()->GetGameObject();

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

    if (1 - rot * rot < 0.1f)return true;

    //プレイヤー向きから、エネミーの方向へ補間する
    PQ = DirectX::XMQuaternionSlerp(PQ, PEQ, rollSpeed * elapsedTime);

    QuaternionStruct playerQuaternion;
    DirectX::XMStoreFloat4(&playerQuaternion.dxFloat4, PQ);

    playerObj->transform_->SetRotation(playerQuaternion);

    return false;
}
