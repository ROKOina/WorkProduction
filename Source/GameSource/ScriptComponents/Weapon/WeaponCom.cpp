#include "WeaponCom.h"

#include <imgui.h>

#include "Components\TransformCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "Components\ParticleSystemCom.h"
#include "SwordTrailCom.h"

#include "../Player/PlayerCom.h"
#include "../CharacterStatusCom.h"

// 開始処理
void WeaponCom::Start()
{
}

// 更新処理
void WeaponCom::Update(float elapsedTime)
{
    assert(parentObject_.lock());

    //終了演出
    DirectionEnd(elapsedTime);

    //現在のアニメーションインデックス取得
    std::shared_ptr<AnimationCom> animCom = parentObject_.lock()->GetComponent<AnimationCom>();

    //アニメーション速度変更していたら戻す
    if (isAnimSetting_)
    {
        //攻撃速度をいじる
        std::shared_ptr<AnimatorCom> animator = parentObject_.lock()->GetComponent<AnimatorCom>();
        animator->SetAnimationSpeedOffset(1);
        isAnimSetting_ = false;
    }

    //今のアニメーションが登録されているか確認
    if (animCom)
    {
        auto it = attackStatus_.find(animCom->GetCurrentAnimationIndex());
        if (it == attackStatus_.end() && !isForeverUse_)    //登録されない時
        {
            if (isWeaponUse_) //武器使用終了時
            {
                isWeaponUse_ = false;

                //攻撃終了処理
                isAttackAnim_ = false;

                //終了演出起動
                isDirectionEnd_ = true;
                directionState_ = 0;

                //コライダー切る
                std::shared_ptr<Collider> col = GetGameObject()->GetComponent<Collider>();
                if (col)
                    col->SetEnabled(false);

            }
            return;
        }
        else
        {
            if (!isWeaponUse_)
            {
                isWeaponUse_ = true;

                //武器演出起動
                isDirectionStart_ = true;
                directionState_ = 0;

                //パーティクル出す
                if (GetGameObject()->GetChildren().size() > 0)
                {
                    std::shared_ptr<GameObject> particle = GetGameObject()->GetChildren()[0].lock();
                    if (particle->GetComponent<ParticleSystemCom>())
                        particle->GetComponent<ParticleSystemCom>()->SetRoop(true);
                }

            }
        }
    }

    //出現演出
    DirectionStart(elapsedTime);

    //ヒット確認リセット
    onHit_ = false;

    //親子関係更新
    ParentSetting();

    //コリジョンで処理を変える攻撃処理
    CollisionWeaponAttack();
}

// GUI描画
void WeaponCom::OnGUI()
{
    ImGui::DragFloat2("colliderUpDown", &colliderUpDown_.x, 0.01f);
    ImGui::Checkbox("isForeverUse_", &isForeverUse_);
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

void WeaponCom::SetAttackDefaultStatus(int damage, float impactPower, float front, float up, float animSpeed, ATTACK_SPECIAL_TYPE specialAttack, float invincibleTime)
{
    defaultStatus_.damage = damage;
    defaultStatus_.impactPower = impactPower;
    defaultStatus_.front = front;
    defaultStatus_.up = up;
    defaultStatus_.specialType = specialAttack;
    defaultStatus_.weaponInvTime = invincibleTime;
}

void WeaponCom::CollisionWeaponAttack()
{
    //攻撃アニメならtrue
    bool isAttackCollision = CollsionFromEventJudge();

    std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
    if (capsule)    //カプセル当たり判定の場合
    {
        //イベントから当たり判定を付ける
        if (isAttackCollision)
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
                //攻撃処理
                AttackProcess(coll.gameObject.lock());
            }
        }

        //攻撃アニメーション処理
        if (isAttackAnim_)
        {
            if (oldIsAnim_)
                if (attackAnimIndex_ != oldAnimIndex_)
                    isAttackAnim_ = false;  //攻撃コンボの場合、１フレームだけfalseに

            oldAnimIndex_ = parentObject_.lock()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();
        }
        oldIsAnim_ = isAttackAnim_;

        return;
    }

    std::shared_ptr<SphereColliderCom> sphere = GetGameObject()->GetComponent<SphereColliderCom>();
    if (sphere) //球当たり判定の場合
    {
        //親オブジェクトのアニメーションを考慮する場合
        if (isParentAnimUse_)
        {
            if (isAttackCollision)
                sphere->SetEnabled(true);
            else
                sphere->SetEnabled(false);
        }

        for (auto& coll : sphere->OnHitGameObject())
        {
            //円を弧の判定にする
            if (circleArc_)
            {
                float spOffset = 1.5f;
                float spRad = sphere->GetRadius() - spOffset;
                if (spRad > 0)
                {
                    DirectX::XMFLOAT3 collPos = coll.gameObject.lock()->transform_->GetWorldPosition();
                    DirectX::XMFLOAT3 collOffPos = coll.gameObject.lock()->GetComponent<Collider>()->GetOffsetPosition();
                    collPos = { collPos.x + collOffPos.x,collPos.y + collOffPos.y,collPos.z + collOffPos.z };
                    DirectX::XMFLOAT3 spPos = sphere->GetGameObject()->transform_->GetWorldPosition();
                    DirectX::XMFLOAT3 spOffPos = sphere->GetGameObject()->GetComponent<SphereColliderCom>()->GetOffsetPosition();
                    spPos = { spPos.x + spOffPos.x,spPos.y + spOffPos.y,spPos.z + spOffPos.z };

                    //距離求める
                    DirectX::XMVECTOR SpCo = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&collPos), DirectX::XMLoadFloat3(&spPos));
                    float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(SpCo));
                    float radius = spRad;

                    //当たっていたらcontinue
                    if (dist < radius)
                    {
                        continue;
                    }

                    //角度でcontinue
                    DirectX::XMVECTOR SpFront = DirectX::XMLoadFloat3(&sphere->GetGameObject()->transform_->GetWorldFront());
                    float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMVector3Normalize(SpCo), DirectX::XMVector3Normalize(SpFront)));
                    if (dot < 54.0f / 90.0f)
                    {
                        continue;
                    }
                }
            }

            //攻撃処理
            AttackProcess(coll.gameObject.lock(), isParentAnimUse_);
        }

        return;
    }
}

void WeaponCom::ParentSetting()
{
    assert(nodeParent_.lock());


    nodeParent_.lock()->UpdateTransform();
    DirectX::XMFLOAT4X4 nodeTrasform;

    //ノード名前の有無で判定
    if (nodeName_.size() > 0)
    {
        std::shared_ptr<RendererCom> parentRendererCom = nodeParent_.lock()->GetComponent<RendererCom>();
        nodeTrasform = parentRendererCom->GetModel()->FindNode(nodeName_.c_str())->worldTransform;
    }
    else
        //return;
        nodeTrasform = nodeParent_.lock()->transform_->GetWorldTransform();

    //親にする
    GetGameObject()->transform_->SetParentTransform(nodeTrasform);
}

void WeaponCom::AttackProcess(std::shared_ptr<GameObject> damageObj, bool useAnim)
{
    //状態確認
    std::shared_ptr<CharacterStatusCom> status = damageObj->GetComponent<CharacterStatusCom>();
    if (status->GetIsInvincible())return;

    //アニメーションを使用するとき
    if (useAnim)
    {
        //現在のアニメーションインデックス取得
        std::shared_ptr<AnimationCom> animCom = parentObject_.lock()->GetComponent<AnimationCom>();
        int animIndex = animCom->GetCurrentAnimationIndex();

        //飛ばす処理
        DirectX::XMVECTOR F = DirectX::XMLoadFloat3(&parentObject_.lock()->transform_->GetWorldFront());
        DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&parentObject_.lock()->transform_->GetWorldUp());
        //方向の割合
        F = DirectX::XMVectorScale(F, attackStatus_[animIndex].front);
        U = DirectX::XMVectorScale(U, attackStatus_[animIndex].up);
        //合成
        DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(DirectX::XMVectorAdd(F, U));
        DirectX::XMFLOAT3 dir;
        DirectX::XMStoreFloat3(&dir, Dir);

        //吹っ飛ばし
        float power = attackStatus_[animIndex].impactPower;
        status->OnDamage(GetGameObject()->GetComponent<WeaponCom>(), attackStatus_[animIndex].damage, DirectX::XMFLOAT3(dir.x * power, dir.y * power, dir.z * power)
            , attackStatus_[animIndex].specialType, attackStatus_[animIndex].weaponInvTime);
    }
    else
    {
        //飛ばす処理
        DirectX::XMVECTOR F = DirectX::XMLoadFloat3(&parentObject_.lock()->transform_->GetWorldFront());
        DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&parentObject_.lock()->transform_->GetWorldUp());
        //方向の割合
        F = DirectX::XMVectorScale(F, defaultStatus_.front);
        U = DirectX::XMVectorScale(U, defaultStatus_.up);
        //合成
        DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(DirectX::XMVectorAdd(F, U));
        DirectX::XMFLOAT3 dir;
        DirectX::XMStoreFloat3(&dir, Dir);

        //吹っ飛ばし
        float power = defaultStatus_.impactPower;
        status->OnDamage(GetGameObject()->GetComponent<WeaponCom>(),defaultStatus_.damage, DirectX::XMFLOAT3(dir.x * power, dir.y * power, dir.z * power)
            , defaultStatus_.specialType, defaultStatus_.weaponInvTime);
    }

    onHit_ = true;

    //親がプレイヤーの場合
    std::shared_ptr<PlayerCom> player = parentObject_.lock()->GetComponent<PlayerCom>();
    if (player)
    {
        //コンボカウントを増やす
        player->AddHitCount();
    }
}

//アニメイベント名から当たり判定を付けるか判断("AutoCollision"から始まるイベントを自動で取得)
bool WeaponCom::CollsionFromEventJudge()
{
    std::shared_ptr<AnimationCom> animCom = parentObject_.lock()->GetComponent<AnimationCom>();

    if (animCom)
    {
        //今のアニメーションが登録されているか確認
        auto it = attackStatus_.find(animCom->GetCurrentAnimationIndex());
        if (it == attackStatus_.end())    //登録されない時
            return false;

        for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
        {

            //頭がAutoCollisionなら当たり判定をする
            if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;

            //攻撃アニメーション時はtrue
            isAttackAnim_ = true;
            //現在のアニメーション保存
            attackAnimIndex_ = parentObject_.lock()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

            //攻撃速度をいじる
            std::shared_ptr<AnimatorCom> animator = parentObject_.lock()->GetComponent<AnimatorCom>();

            //エンドフレーム前なら
            if (!animCom->GetCurrentAnimationEventIsEnd(animEvent.name.c_str()))
            {
                //アニメーションスピードを設定
                animator->SetAnimationSpeedOffset(attackStatus_[animCom->GetCurrentAnimationIndex()].animSpeed);
                isAnimSetting_ = true;
            }
            else
            {
                //トレイル切る               
                std::shared_ptr<SwordTrailCom> trail = GetGameObject()->GetComponent<SwordTrailCom>();
                if (trail)
                {
                    if (trail->GetEnabled())
                    {
                        //trail->SetEnabled(false);
                        trail->EndTrail();
                    }
                }
            }

            if (!animCom->GetCurrentAnimationEvent(animEvent.name.c_str(), DirectX::XMFLOAT3(0, 0, 0)))continue;

            //トレイル出す
            std::shared_ptr<SwordTrailCom> trail = GetGameObject()->GetComponent<SwordTrailCom>();
            if (trail)
            {
                if (!trail->GetEnabled())
                {
                    trail->SetEnabled(true);
                    trail->ResetNodePos();
                }
            }


            return true;
        }
    }

    return false;
}

void WeaponCom::DirectionStart(float elapsedTime)
{
    if (!isDirectionStart_)return;

    std::shared_ptr<RendererCom> renderCom = GetGameObject()->GetComponent<RendererCom>();
    if (!renderCom) //描画するものがない場合
    {
        //演出終了
        isDirectionStart_ = false;
        directionState_ = -1;
    }

    //光らせる処理
    switch (directionState_)
    {
    case 0:
    {
        renderCom->SetEnabled(true);
        renderCom->GetModel()->SetMaterialColor({ 1,2,1,2 });

        directionState_++;
    }
    break;
    case 1:
    {
        DirectX::XMFLOAT4 color = renderCom->GetModel()->GetMaterialColor();
        color.y -= 2 * elapsedTime;
        color.w -= 2 * elapsedTime;
        if (color.y < 1 && color.w < 1)
        {
            //演出終了
            isDirectionStart_ = false;
            directionState_ = -1;
        }
        if (color.y < 1)color.y = 1;
        if (color.w < 1)color.w = 1;
        renderCom->GetModel()->SetMaterialColor(color);
    }
    break;
    }

}

void WeaponCom::DirectionEnd(float elapsedTime)
{
    if (!isDirectionEnd_)return;

    std::shared_ptr<RendererCom> renderCom = GetGameObject()->GetComponent<RendererCom>();
    if (!renderCom) //描画するものがない場合
    {
        //演出終了
        isDirectionEnd_ = false;
        directionState_ = -1;

        //パーティクル切る
        if (GetGameObject()->GetChildren().size() > 0)
        {
            std::shared_ptr<GameObject> particle = GetGameObject()->GetChildren()[0].lock();
            if (particle->GetComponent<ParticleSystemCom>())
                particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
        }
    }

    switch (directionState_)
    {
    case 0:
    {
        DirectX::XMFLOAT4 color = renderCom->GetModel()->GetMaterialColor();
        color.y += 2 * elapsedTime;
        color.w += 2 * elapsedTime;
        if (color.y > 2 && color.w > 2)
        {
            //演出終了
            renderCom->SetEnabled(false);

            isDirectionEnd_ = false;
            directionState_ = -1;

            //パーティクル切る
            if (GetGameObject()->GetChildren().size() > 0)
            {
                std::shared_ptr<GameObject> particle = GetGameObject()->GetChildren()[0].lock();
                if (particle->GetComponent<ParticleSystemCom>())
                    particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
            }

            //トレイル切る
            std::shared_ptr<SwordTrailCom> trail = GetGameObject()->GetComponent<SwordTrailCom>();
            if (trail)
            {
                trail->SetEnabled(false);
            }

        }
        renderCom->GetModel()->SetMaterialColor(color);
    }
    break;
    }
}
