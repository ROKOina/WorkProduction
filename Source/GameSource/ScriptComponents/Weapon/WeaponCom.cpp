#include "WeaponCom.h"

#include <imgui.h>

#include "Components\TransformCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"

#include "../CharacterStatusCom.h"

// 開始処理
void WeaponCom::Start()
{
    ////回転
    //GetGameObject()->transform_->SetEulerRotation(DirectX::XMFLOAT3(-154, -85, 82));
    ////当たり大きさ
    //GetGameObject()->GetComponent<CapsuleColliderCom>()->SetRadius(0.19f);
}

// 更新処理
void WeaponCom::Update(float elapsedTime)
{
    assert(parentObject_.lock());
    assert(nodeName_.size() > 0);

    //ヒット確認リセット
    onHit_ = false;

    parentObject_.lock()->UpdateTransform();
    std::shared_ptr<RendererCom> parentRendererCom = parentObject_.lock()->GetComponent<RendererCom>();
    Model::Node* parentNode = parentRendererCom->GetModel()->FindNode(nodeName_.c_str());

    //親にする
    GetGameObject()->transform_->SetParentTransform(parentNode->worldTransform);

    std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
    //イベントから当たり判定を付ける
    if (CollsionFromEventJudge())
        capsule->SetEnabled(true);
    else
        capsule->SetEnabled(false);

    //カプセル当たり判定設定
    if (capsule->GetEnabled())
    {
        //剣先から剣元を設定    
        DirectX::XMFLOAT3 up = GetGameObject()->transform_->GetWorldUp();
        capsule->SetPosition1({
             up.x * colliderUpDown_.y
            ,up.y * colliderUpDown_.y
            ,up.z * colliderUpDown_.y
            });
        capsule->SetPosition2({
             up.x * colliderUpDown_.x
            ,up.y * colliderUpDown_.x
            ,up.z * colliderUpDown_.x
            });

        for (auto& coll : capsule->OnHitGameObject())
        {
            //状態確認
            std::shared_ptr<CharacterStatusCom> status = coll.gameObject.lock()->GetComponent<CharacterStatusCom>();
            if (status->GetIsInvincible())continue;

            //現在のアニメーションインデックス取得
            std::shared_ptr<AnimationCom> animCom = GetGameObject()->GetParent()->GetComponent<AnimationCom>();
            int animIndex = animCom->GetCurrentAnimationIndex();

            //飛ばす処理
            DirectX::XMVECTOR F = DirectX::XMLoadFloat3(&GetGameObject()->GetParent()->transform_->GetWorldFront());
            DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&GetGameObject()->GetParent()->transform_->GetWorldUp());
            //方向の割合
            F = DirectX::XMVectorScale(F, attackStatus_[animIndex].front);
            U = DirectX::XMVectorScale(U, attackStatus_[animIndex].up);
            //合成
            DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(DirectX::XMVectorAdd(F, U));
            DirectX::XMFLOAT3 dir;
            DirectX::XMStoreFloat3(&dir, Dir);

            //吹っ飛ばし
            float power = attackStatus_[animIndex].impactPower;
            status->OnDamage(DirectX::XMFLOAT3(dir.x * power, dir.y * power, dir.z * power)
                , attackStatus_[animIndex].specialType);

            onHit_ = true;
        }
    }

    //攻撃アニメーション処理
    if (isAttackAnim_)
    {
        if (oldIsAnim_)
            if (attackAnimIndex_ != oldAnimIndex)
                isAttackAnim_ = false;  //攻撃コンボの場合、１フレームだけfalseに

        oldAnimIndex = parentObject_.lock()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();
    }
    oldIsAnim_ = isAttackAnim_;

}

// GUI描画
void WeaponCom::OnGUI()
{
    ImGui::DragFloat2("colliderUpDown", &colliderUpDown_.x, 0.01f);
}


void WeaponCom::SetAttackStatus(int animIndex, int damage, float impactPower, float front, float up, float animSpeed, ATTACK_SPECIAL_TYPE specialAttack)
{
    attackStatus_[animIndex].damage = damage;
    attackStatus_[animIndex].impactPower = impactPower;
    attackStatus_[animIndex].front = front;
    attackStatus_[animIndex].up = up;
    attackStatus_[animIndex].specialType = specialAttack;

    //アニメーションスピード
    attackStatus_[animIndex].animSpeed = animSpeed;
}


//アニメイベント名から当たり判定を付けるか判断("AutoCollision"から始まるイベントを自動で取得)
bool WeaponCom::CollsionFromEventJudge()
{
    std::shared_ptr<AnimationCom> animCom = GetGameObject()->GetParent()->GetComponent<AnimationCom>();

    //攻撃速度をいじる
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetParent()->GetComponent<AnimatorCom>();

    //アニメーション速度変更していたら戻す
    if (isAnimSetting)
    {
        animator->SetAnimationSpeedOffset(1);
        isAnimSetting = false;
    }

    for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
    {
        //頭がAutoCollisionなら当たり判定をする
        if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;

        //攻撃アニメーション時はtrue
        isAttackAnim_ = true;
        //現在のアニメーション保存
        attackAnimIndex_ = parentObject_.lock()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

        //エンドフレーム前なら
        if (!animCom->GetCurrentAnimationEventIsEnd(animEvent.name.c_str()))
        {
            //アニメーションスピードを設定
            animator->SetAnimationSpeedOffset(attackStatus_[animCom->GetCurrentAnimationIndex()].animSpeed);
            isAnimSetting = true;
        }

        if (!animCom->GetCurrentAnimationEvent(animEvent.name.c_str(), DirectX::XMFLOAT3()))continue;

        return true;
    }
    return false;
}
