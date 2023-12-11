#include "EnemyCom.h"
#include "Components\AnimationCom.h"
#include "Components\TransformCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimatorCom.h"
#include "Components\RendererCom.h"
#include "Components\CameraCom.h"
#include "Components\ParticleSystemCom.h"
#include "Components\ParticleComManager.h"

#include "BehaviorTree/JudgmentDerived.h"
#include "BehaviorTree/ActionDerived.h"
#include "GameSource/Math/Mathf.h"

#include "../Player/PlayerCom.h"
#include "../CharacterStatusCom.h"

#include "Graphics\Shaders\PostEffect.h"

#include <imgui.h>


// 開始処理
void EnemyCom::Start()
{
}

// 更新処理
void EnemyCom::Update(float elapsedTime)
{
    //ゲーム開始フラグ
    if (!GameObjectManager::Instance().GetIsSceneGameStart())
        return;

    //死亡確認
    {
        //ステータス設定
        std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
        if (0 >= status->GetHP())
        {
            GameObjectManager::Instance().Remove(GetGameObject());
        }
    }

    //立ち上がりモーション処理
    StandUpUpdate();

    // 現在実行されているノードが無ければ
    if (activeNode_ == nullptr && !isAnimDamage_ && !isJumpDamage_ && !isStandUpMotion_)
    {
        // 次に実行するノードを推論する。
        activeNode_ = aiTree_->ActiveNodeInference(behaviorData_);
    }
    // 現在実行するノードがあれば
    if (activeNode_ != nullptr)
    {
         //ビヘイビアツリーからノードを実行。
        std::shared_ptr<NodeBase> n = aiTree_->Run(activeNode_, behaviorData_, elapsedTime);
        if (!n)
        {
            //放棄
            activeNode_.reset();
        }
        else
        {
            if (n->GetId() != activeNode_->GetId())
                activeNode_.swap(n);
        }
    }

    //重力設定
    GravityProcess(elapsedTime);

    //ダメージ処理
    DamageProcess(elapsedTime);

    //ジャスト回避用判定出す
    justColliderProcess();

    //アニメーション設定
    AnimationSetting();

    //アニメーションイベントでSE
    PlayAnimationSE();

    //壁判定
    {
        DirectX::XMFLOAT3 kabePlus = GameObjectManager::Instance().Find("kabePlus")->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 kabeMinas = GameObjectManager::Instance().Find("kabeMinas")->transform_->GetWorldPosition();

        DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
        if (pos.x > kabePlus.x)
            pos.x = kabePlus.x;
        if (pos.x < kabeMinas.x)
            pos.x = kabeMinas.x;
        if (pos.z > kabePlus.z)
            pos.z = kabePlus.z;
        if (pos.z < kabeMinas.z)
            pos.z = kabeMinas.z;

        GetGameObject()->transform_->SetWorldPosition(pos);
    }
}

// GUI描画
void EnemyCom::OnGUI()
{
    if (ImGui::Button("delete"))
    {
        GameObjectManager::Instance().Remove(GetGameObject());
    }

    ImGui::DragFloat("walkSpeed", &moveDataEnemy_.walkSpeed);
    ImGui::DragFloat("walkMaxSpeed", &moveDataEnemy_.walkMaxSpeed);
    ImGui::DragFloat("runSpeed", &moveDataEnemy_.runSpeed);
    ImGui::DragFloat("runMaxSpeed", &moveDataEnemy_.runMaxSpeed);

    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float speed= DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));
    ImGui::DragFloat("moveSpeed", &speed);

    ImGui::DragFloat2("PP", &sP.x);
    ImGui::DragFloat2("saP", &saP.x,0.01f);
}

void EnemyCom::Render2D(float elapsedTime)
{
}

void EnemyCom::AudioRelease()
{
    damageSE_->AudioRelease();
    for (auto& se : animSE)
    {
        se.SE->AudioRelease();
    }
}

void EnemyCom::MaskRender(PostEffect* postEff, std::shared_ptr<CameraCom> maskCamera)
{
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();

    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");

    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();

    //表示カリング

    //カメラの向き
    DirectX::XMFLOAT3 cameraDir = cameraObj->transform_->GetWorldFront();
    DirectX::XMFLOAT3 cameraPos = cameraObj->transform_->GetWorldPosition();
    DirectX::XMVECTOR CameraToEnemy = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&cameraPos)));
    float dot = DirectX::XMVector3Dot(DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&cameraDir)), CameraToEnemy).m128_f32[0];
    if (dot < 0.5f)return;

    //距離
    float len = DirectX::XMVector3Length(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&playerPos))).m128_f32[0];
    float drawLen = 10; //表示範囲
    if (len > drawLen)return;
    len /= drawLen;
    len = (len - 1.0f) * -1.0f;

    pos.y += 2;

    //ワールド座標からスクリーン座標に
    std::shared_ptr<CameraCom> camera = cameraObj->GetComponent<CameraCom>();
    pos=graphics.WorldToScreenPos(pos, camera);

    DirectX::XMFLOAT2 size{50 * len, 50 * len};
    pos.x = pos.x - size.x / 2;
    pos.y = pos.y - size.y / 2;

    //ワイプ背景
    hpBackSprite_->Render(dc, pos.x, pos.y, size.x, size.y
        , 0, 0, static_cast<float>(hpBackSprite_->GetTextureWidth()), static_cast<float>(hpBackSprite_->GetTextureHeight())
        , 0, 1, 1, 1, 1);

    //HP
    {
        //マスクする側描画
        postEff->CacheMaskBuffer(maskCamera);

        std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
        float hpRatio = float(status->GetHP()) / float(status->GetMaxHP());


        //hpmask
        hpMaskSprite_->Render(dc, pos.x + size.x, pos.y + size.x * 0.98f, size.x, size.y * 0.95f * hpRatio
            , 0, 0, static_cast<float>(hpMaskSprite_->GetTextureWidth()), static_cast<float>(hpMaskSprite_->GetTextureHeight())
            , 180, 1, 1, 1, 0.001f);

        //マスクされる側描画
        postEff->StartBeMaskBuffer();

        //hp
        hpSprite_->Render(dc, pos.x, pos.y, size.x, size.y
            , 0, 0, static_cast<float>(hpSprite_->GetTextureWidth()), static_cast<float>(hpSprite_->GetTextureHeight())
            , 0, 1, 1, 1, 1);

        //マスク処理終了処理
        postEff->RestoreMaskBuffer();

        postEff->DrawMask();
    }

}

// ターゲット位置をランダム設定
void EnemyCom::SetRandomTargetPosition()
{
    DirectX::XMFLOAT3 kabePlus = GameObjectManager::Instance().Find("kabePlus")->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 kabeMinas = GameObjectManager::Instance().Find("kabeMinas")->transform_->GetWorldPosition();

    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    pos.x += Mathf::RandomRange(-3, 3);
    pos.z += Mathf::RandomRange(-3, 3);

    if (pos.x > kabePlus.x)
        pos.x = kabePlus.x;
    if (pos.x < kabeMinas.x)
        pos.x = kabeMinas.x;
    if (pos.z > kabePlus.z)
        pos.z = kabePlus.z;
    if (pos.z < kabeMinas.z)
        pos.z = kabeMinas.z;

    targetPosition_ = pos;
}

//ターゲット位置に移動、回転
void EnemyCom::GoTargetMove(bool isMove, bool isTurn)
{
    // 目的地点までのXZ平面での距離判定
    DirectX::XMFLOAT3 position = GetGameObject()->transform_->GetWorldPosition();

    // 目的地点へ移動
    DirectX::XMVECTOR Pos = { position.x,0,position.z };
    DirectX::XMVECTOR TPos = { targetPosition_.x,0,targetPosition_.z };
    DirectX::XMFLOAT3 force;
    DirectX::XMStoreFloat3(&force, DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(TPos, Pos)), moveDataEnemy_.runSpeed));

    if (isMove)
    {
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        move->AddForce(force);
    }

    if (isTurn)
    {
        //回転する
        QuaternionStruct myQ = GetGameObject()->transform_->GetRotation();
        QuaternionStruct focusQ = QuaternionStruct::LookRotation(force);
        //補完する
        DirectX::XMStoreFloat4(&myQ.dxFloat4, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&myQ.dxFloat4), DirectX::XMLoadFloat4(&focusQ.dxFloat4), 0.1f));
        GetGameObject()->transform_->SetRotation(myQ.dxFloat4);
    }
}


//プレイヤーが近いとtrue
bool EnemyCom::SearchPlayer()
{
    DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();

    float vx = playerPos.x - pos.x;
    float vy = playerPos.y - pos.y;
    float vz = playerPos.z - pos.z;
    float dist = sqrtf(vx * vx + vy * vy + vz * vz);

    if (dist < searchRange_)
    {
        return true;
    }
    return false;
}

//ダメージ確認
bool EnemyCom::OnDamageEnemy()
{
    return GetGameObject()->GetComponent<CharacterStatusCom>()->GetFrameDamage();
}

//被弾ー＞立ち上がりモーション
void EnemyCom::SetStandUpMotion()
{
    //着地アニメーション入れる
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetTriggerOn("damageFallEnd");
    isStandUpMotion_ = true;
    playStandUp_ = false;
    //攻撃を受けても移動しないように
    GetGameObject()->GetComponent<CharacterStatusCom>()->SetAttackNonMove(true);
}
void EnemyCom::StandUpUpdate()
{
    if (isStandUpMotion_)
    {
        std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
        int index = animation->GetCurrentAnimationIndex();
        if (index == getUpAnim_) //起き上がりモーション時
        {
            playStandUp_ = true;
        }
        if (playStandUp_)
        {
            if (!animation->IsPlayAnimation())
            {
                playStandUp_ = false;
                isStandUpMotion_ = false;
                GetGameObject()->GetComponent<CharacterStatusCom>()->SetAttackNonMove(false);
            }
        }
    }
}

void EnemyCom::PlayAnimationSE()
{
    std::shared_ptr<AnimationCom> anim = GetGameObject()->GetComponent<AnimationCom>();

    for (auto& se : animSE)
    {
        if (se.isPlay && se.saveAnimIndex > 0)
        {
            if (anim->GetCurrentAnimationIndex() != se.saveAnimIndex)
            {
                se.saveAnimIndex = -1;
                se.isPlay = false;
            }
            continue;
        }
        if (anim->GetCurrentAnimationEvent(se.animEventName.c_str(), DirectX::XMFLOAT3()))
        {
            se.isPlay = true;
            se.saveAnimIndex = anim->GetCurrentAnimationIndex();
            se.SE->Stop();
            se.SE->Play(false, se.volumeSE);
        }
    }
}

//ダメージ処理
void EnemyCom::DamageProcess(float elapsedTime)
{
    //空中ダメージ時重力を少しの間0にする
    {
        if (!isAnimDamage_ && oldAnimDamage_)
        {
            if (!GetGameObject()->GetComponent<MovementCom>()->OnGround())
            {
                skyGravityZeroTimer_ = 0.3f;
            }
        }
        oldAnimDamage_ = isAnimDamage_;
    }

    //ダメージアニメーション処理
    if (isAnimDamage_)
    {
        isAnimDamage_ = GetGameObject()->GetComponent<CharacterStatusCom>()->GetDamageAnimation();
    }

    //ジャンプ被弾時の着地判定
    if (isJumpDamage_)
    {
        if (!isAnimDamage_)
        {
            if (GetGameObject()->GetComponent<MovementCom>()->OnGround())
            {
                isJumpDamage_ = false;
                //立ち上がりモーション起動
                SetStandUpMotion();
            }
        }
    }

    //ダメージエフェクト間隔管理
    damageEffTimer_ -= elapsedTime;

    //ダメージ処理
    if (OnDamageEnemy())
    {
        bool endTree = false;   //今の遷移を終わらせて被弾アニメーションするならtrue

        //アンストッパブル被弾ならそのままアニメーション
        ATTACK_SPECIAL_TYPE attackType = GetGameObject()->GetComponent<CharacterStatusCom>()->GetDamageType();
        if (attackType == ATTACK_SPECIAL_TYPE::UNSTOP)
            endTree = true;
        //ジャンプ中は無防備
        if (attackType == ATTACK_SPECIAL_TYPE::JUMP_NOW)
            endTree = true;


        //被弾する行動か判断
        for (int& id : damageAnimAiTreeId_)
        {
            //アクションノードがない場合
            if (!activeNode_)
                endTree = true;

            if (endTree)break;


            NodeBase* node = activeNode_.get();
            //被弾時にアニメーションするか確認
            while (1)
            {
                //おなじIDならendTreeをtrueに
                if (node->GetId() == id)
                {
                    endTree = true;
                    break;
                }

                //親も確認する
                node = node->GetParent().get();
                if (!node)break;
            }

        }

            //遷移を終わらせてアニメーションする
        if (endTree)
        {
            if (activeNode_)
            {
                activeNode_->EndActionSetStep();
                activeNode_->Run(GetGameObject()->GetComponent<EnemyCom>(), elapsedTime);
                //放棄
                activeNode_.reset();
            }
        }

        //ダメージ処理
        if (endTree)
        {
            isAnimDamage_ = true;
            std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
            animator->ResetParameterList();

            //アニメーションの種類を判定
            if (isStandUpMotion_)   //立ち上がりモーション中を優先
            {
                SetStandUpMotion();
            }
            else if (attackType == ATTACK_SPECIAL_TYPE::JUMP_START)  //切り上げ被弾時
            {
                animator->SetTriggerOn("damageGoFly");
                isJumpDamage_ = true;

            }
            else if (isJumpDamage_)    //空中被弾時
            {
                animator->SetTriggerOn("damageInAir");
            }
            else
            {
                animator->SetTriggerOn("damage");
            }
        }

        //エフェクト&SE
        if (damageEffTimer_ < 0)
        {
            GetGameObject()->GetChildFind("Particle")->GetComponent<ParticleSystemCom>()->Restart();
            damageEffTimer_ = 0.5f;

            damageSE_->Stop();
            damageSE_->Play(false);
        }
    }
}

//ジャスト回避用判定出す
void EnemyCom::justColliderProcess()
{
    //ジャスト回避当たり判定を切り、アタック当たり判定をしている
    DirectX::XMFLOAT3 pos;
    //ジャスト
    std::shared_ptr<GameObject> justChild = GetGameObject()->GetChildFind("attackJust");

    //ジャスト当たり判定を切っておく
    justChild->GetComponent<Collider>()->SetEnabled(false);

    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
    for (auto& animEvent : animation->GetCurrentAnimationEventsData())
    {
        //justが入っているならなら
        if (animEvent.name.find("just") == std::string::npos)continue;
        if (isJustAvoid_)justChild->GetComponent<Collider>()->SetEnabled(true);
        //イベント中なら当たり判定を出す
        if (animation->GetCurrentAnimationEvent(animEvent.name.c_str(), pos))
        {
            isJustAvoid_ = false;
            justChild->GetComponent<Collider>()->SetEnabled(true);
        }
    }
}

//重力設定
void EnemyCom::GravityProcess(float elapsedTime)
{
    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();

    //アニメーションイベントを見て重力をなくす
    if (animation->GetCurrentAnimationEvent("ZeroGravity", DirectX::XMFLOAT3()) || skyGravityZeroTimer_ > 0)
    {
        skyGravityZeroTimer_ -= elapsedTime;
        isSetGravity_ = true;
        GetGameObject()->GetComponent<MovementCom>()->SetGravity(GRAVITY_ZERO);
        GetGameObject()->GetComponent<MovementCom>()->ZeroVelocityY();
    }
    else
    {
        if (isSetGravity_)   //重力設定をした時だけ入る
        {
            isSetGravity_ = false;
            GetGameObject()->GetComponent<MovementCom>()->SetGravity(GRAVITY_NORMAL);
            GetGameObject()->GetComponent<MovementCom>()->ZeroVelocityY();
        }
    }
}

bool EnemyCom::OnMessage(const Telegram& msg)
{
    return false;
}

//アニメーションの更新
void EnemyCom::AnimationSetting()
{
    //移動アニメーション設定
    {
        //移動スピード代入
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetFloatValue("moveSpeed", DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity)));
    }

}




//被弾時にアニメーションする時のAITREEを決める
template<typename... Args>
void EnemyCom::OnDamageAnimAI_TREE(Args... args)
{
    for (AI_TREE i : std::initializer_list<AI_TREE>{ args... }) {
        damageAnimAiTreeId_.emplace_back(static_cast<int>(i));
    }
}
