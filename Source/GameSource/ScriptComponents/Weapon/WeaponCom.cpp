#include "WeaponCom.h"

#include <imgui.h>

#include "Components\TransformCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\AnimationCom.h"

#include "../CharacterStatusCom.h"

// 開始処理
void WeaponCom::Start()
{
    //回転
    GetGameObject()->transform_->SetEulerRotation(DirectX::XMFLOAT3(-154, -85, 82));
    //当たり大きさ
    GetGameObject()->GetComponent<CapsuleColliderCom>()->SetRadius(0.19f);
}

// 更新処理
void WeaponCom::Update(float elapsedTime)
{
    assert(parentObject_);
    assert(nodeName_.size() > 0);

    onHit_ = false;

    std::shared_ptr<RendererCom> rendererCom = parentObject_->GetComponent<RendererCom>();
    Model::Node* node = rendererCom->GetModel()->FindNode(nodeName_.c_str());

    //親にする
    GetGameObject()->transform_->SetParentTransform(node->worldTransform);


    //イベントから当たり判定を付ける
    std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
    if (CollsionFromEventJudge())
        capsule->SetEnabled(true);
    else
        capsule->SetEnabled(false);

    //カプセル当たり判定設定
    if (capsule->GetEnabled())
    {
        //剣先から剣元を設定
        capsule->SetPosition1({ 0,0,0 });
        DirectX::XMFLOAT3 up = GetGameObject()->transform_->GetWorldUp();
        capsule->SetPosition2({ up.x,up.y,up.z });

        for (auto& coll : capsule->OnHitGameObject())
        {
            //状態確認
            std::shared_ptr<CharacterStatusCom> status = coll.gameObject->GetComponent<CharacterStatusCom>();
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
            status->OnDamage(DirectX::XMFLOAT3(dir.x * power, dir.y * power, dir.z * power ));

            onHit_ = true;
        }
    }
}

// GUI描画
void WeaponCom::OnGUI()
{

}


void WeaponCom::SetAttackStatus(int animIndex, int damage, float impactPower, float front, float up)
{
    attackStatus_[animIndex].damage = damage;
    attackStatus_[animIndex].impactPower = impactPower;
    attackStatus_[animIndex].front = front;
    attackStatus_[animIndex].up = up;
}


//アニメイベント名から当たり判定を付けるか判断("AutoCollision"から始まるイベントを自動で取得)
bool WeaponCom::CollsionFromEventJudge()
{
    std::shared_ptr<AnimationCom> animCom = GetGameObject()->GetParent()->GetComponent<AnimationCom>();
    for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
    {
        //頭がAutoCollisionなら当たり判定をする
        if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;
        if (!animCom->GetCurrentAnimationEvent(animEvent.name.c_str(), DirectX::XMFLOAT3()))continue;

        return true;
    }
    return false;
}
