#include "EnemyCom.h"
#include "Components\AnimationCom.h"
#include "Components\TransformCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"

#include "../Player/PlayerCom.h"

// 開始処理
void EnemyCom::Start()
{

}

// 更新処理
void EnemyCom::Update(float elapsedTime)
{
    //仮でジャスト回避当たり判定を切り、アタック当たり判定をしている
    DirectX::XMFLOAT3 pos;
    //ジャスト
    std::shared_ptr<GameObject> justChild = GetGameObject()->GetChildFind("picolaboAttackJust");
    if (GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("kick_left_just", pos)) {
        justChild->GetComponent<Collider>()->SetEnabled(true);
        //justChild->transform_->SetWorldPosition(pos);
    }
    else
    {
        justChild->GetComponent<Collider>()->SetEnabled(false);
    }

    //ジャスト当たり判定
    std::vector<HitObj> hitObj = justChild->GetComponent<Collider>()->OnHitGameObject();
    for (auto& h : hitObj)
    {
        if (h.gameObject->GetComponent<Collider>()->GetMyTag() != COLLIDER_TAG::Player)continue;
        int i = 0;
    }

    //アタック
    std::shared_ptr<GameObject> attackChild = GetGameObject()->GetChildFind("picolaboAttack");
    if (GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("kick_left", pos)) {
        attackChild->GetComponent<Collider>()->SetEnabled(true);
        attackChild->transform_->SetWorldPosition(pos);
    }
    else
    {
        attackChild->GetComponent<Collider>()->SetEnabled(false);
    }

    ////アタック当たり判定
    //hitObj = attackChild->GetComponent<Collider>()->OnHitGameObject();
    //for (auto& h : hitObj)
    //{
    //    if (h.gameObject->GetComponent<Collider>()->GetMyTag() != COLLIDER_TAG::Player)continue;
    //    //プレイヤーにダメージ
    //    std::shared_ptr<GameObject> player = h.gameObject;
    //    if (player->GetComponent<PlayerCom>()->GetIsInvincible())continue;
    //    player->GetComponent<MovementCom>()->AddNonMaxSpeedForce({ 0, 0, -30 });
    //    player->GetComponent<PlayerCom>()->OnDamage();
    //}
}

// GUI描画
void EnemyCom::OnGUI()
{

}