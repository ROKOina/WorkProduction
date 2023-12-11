#include "EnemyGrapeFarCom.h"
#include "EnemyManager.h"

#include "Components\TransformCom.h"
#include "Components\AnimatorCom.h"
#include "Components\AnimationCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"

#include "BehaviorTree/JudgmentDerived.h"
#include "BehaviorTree/ActionDerived.h"
#include "../Weapon\WeaponCom.h"
#include "../CharacterStatusCom.h"


#include <imgui.h>


#pragma region �G�{��

enum GRAPE_ANIMATION
{
    AIR_DAMAGE,
    ATTACK_THROW,
    DAMAGE,
    FLY_DAMAGE,
    FALL_END,
    FALL_YARARE,
    GET_UP,
    IDLE,
    WALK,
};

// �J�n����
void EnemyGrapeFarCom::Start()
{
    EnemyManager::Instance().Register(GetGameObject(), EnemyManager::EnemyKind::FAR_ENEMY);

    //�_���[�W�A�j���[�V����������m�[�h��o�^
    OnDamageAnimAI_TREE(AI_TREE::IDLE,AI_TREE::WANDER, AI_TREE::PURSUIT, AI_TREE::BACK_MOVE, AI_TREE::ATTACK_IDLE, AI_TREE::NORMAL);

    //�A�j���[�V����������
    AnimationInitialize();
    std::shared_ptr<EnemyCom> myShared = GetGameObject()->GetComponent<EnemyCom>();
    // �r�w�C�r�A�c���[�ݒ�
    behaviorData_ = std::make_unique<BehaviorData>();
    aiTree_ = std::make_unique<BehaviorTree>(myShared);
    // BehaviorTree�}�����BehaviorTree���\�z
    aiTree_->AddNode(AI_TREE::NONE_AI, AI_TREE::ROOT, 0, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //2�w
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::BATTLE, 1, BehaviorTree::SelectRule::Priority, std::make_shared<BattleJudgment>(myShared), nullptr);
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::SCOUT, 2, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //3�w
    aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::WANDER, 1, BehaviorTree::SelectRule::Non, std::make_shared<WanderJudgment>(myShared), std::make_shared<WanderAction>(myShared));
    aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::IDLE, 2, BehaviorTree::SelectRule::Non, nullptr, std::make_shared<IdleAction>(myShared));

    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::ATTACK, 1, BehaviorTree::SelectRule::Priority, std::make_shared<AttackJudgment>(myShared), nullptr);
    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::BACK_MOVE, 2, BehaviorTree::SelectRule::Non, std::make_shared<BackMoveJudgment>(myShared), std::make_shared<BackMoveAction>(myShared));
    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::PURSUIT, 3, BehaviorTree::SelectRule::Non, nullptr, std::make_shared<PursuitAction>(myShared));

    //4�w
    aiTree_->AddNode(AI_TREE::ATTACK, AI_TREE::ATTACK_IDLE, 1, BehaviorTree::SelectRule::Non, std::make_shared<AttackIdleJudgment>(myShared), std::make_shared<AttackIdleAction>(myShared));
    aiTree_->AddNode(AI_TREE::ATTACK, AI_TREE::NORMAL, 2, BehaviorTree::SelectRule::Non, std::make_shared<AttackJudgment>(myShared), std::make_shared<AttackAction>(myShared));

    SetRandomTargetPosition();

    //�X�e�[�^�X�ݒ�
    {
        //hp
        std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
        status->SetMaxHP(30);
        status->SetHP(30);

        //�U��
        attackRange_ = 8;
        searchRange_ = 14;
    }

    //����������
    std::vector<ModelResource::Material>& materials = GetGameObject()->GetComponent<RendererCom>()->GetModel()->GetResourceShared()->GetMaterialsEdit();
    materials[0].toonStruct._Emissive_Color.w = 0;

    //�N���オ�胂�[�V�����ۑ�
    getUpAnim_ = static_cast<int>(GET_UP);

    ////�A�j���[�V�����C�x���gSE������
    //animSE.emplace_back("SEDamage", "Data/Audio/Enemy/damageEnemy.wav");
}

// �X�V����
void EnemyGrapeFarCom::Update(float elapsedTime)
{
    EnemyFarCom::Update(elapsedTime);

    //�A�j���[�V���������Ēe���o��
    if (GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("AutoCollisionAttack01", DirectX::XMFLOAT3()))
    {
        if (!isSpawnBall_)
        {
            isSpawnBall_ = true;
            SpawnGrapeBall();
        }
    }
    else
    {
        isSpawnBall_ = false;
    }
}

// GUI�`��
void EnemyGrapeFarCom::OnGUI()
{
    EnemyFarCom::OnGUI();
}

//�A�j���[�V�����������ݒ�
void EnemyGrapeFarCom::AnimationInitialize()
{
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //�����̃A�j���[�V����
    animator->SetFirstTransition(IDLE);
    animator->SetLoopAnimation(IDLE, true);

    //�A�j���[�V�����p�����[�^�[�ǉ�
    //enemy���ʃp�����[�^�[
    {
        animator->AddTriggerParameter("attack");
        animator->AddTriggerParameter("damage");
        animator->AddTriggerParameter("damageInAir");
        animator->AddTriggerParameter("damageGoFly");
        animator->AddTriggerParameter("damageFallEnd");

        animator->AddFloatParameter("moveSpeed");
    }

    //idle -> walk
    animator->AddAnimatorTransition(IDLE, WALK);
    animator->SetLoopAnimation(IDLE, true);
    animator->SetFloatTransition(IDLE, WALK,
        "moveSpeed", 0.1f, PARAMETER_JUDGE::GREATER);

    //walk -> idle
    animator->AddAnimatorTransition(WALK, IDLE);
    animator->SetLoopAnimation(WALK, true);
    animator->SetFloatTransition(WALK, IDLE,
        "moveSpeed", 0.1f, PARAMETER_JUDGE::LESS);

    //�U��
    animator->AddAnimatorTransition(ATTACK_THROW, false, 0);
    animator->SetTriggerTransition(ATTACK_THROW, "attack");
    animator->AddAnimatorTransition(ATTACK_THROW, IDLE, true);

    //��e
    {
        //�m�[�}��
        animator->AddAnimatorTransition(DAMAGE, false, 0.0f);
        animator->SetTriggerTransition(DAMAGE, "damage");
        animator->AddAnimatorTransition(DAMAGE, IDLE, true);

        //��
        animator->AddAnimatorTransition(AIR_DAMAGE, false, 0.0f);
        animator->SetTriggerTransition(AIR_DAMAGE, "damageInAir");

        animator->AddAnimatorTransition(AIR_DAMAGE, FALL_YARARE, true);

        //�n��[����
        animator->AddAnimatorTransition(FLY_DAMAGE, false, 0.0f);
        animator->SetTriggerTransition(FLY_DAMAGE, "damageGoFly");


        animator->AddAnimatorTransition(FLY_DAMAGE, FALL_YARARE, true);

        animator->SetLoopAnimation(FALL_YARARE, true);

        //��e�[�����n
        animator->AddAnimatorTransition(FALL_END);
        animator->SetTriggerTransition(FALL_END, "damageFallEnd");

        //�N���オ��
        animator->AddAnimatorTransition(FALL_END, GET_UP, true);

        animator->AddAnimatorTransition(GET_UP, IDLE, true);
    }

}

void EnemyGrapeFarCom::SpawnGrapeBall()
{
    //�o���|�W�V�����ݒ�
    DirectX::XMFLOAT3 ballPos;
    DirectX::XMFLOAT4X4 worldTransform = GetGameObject()->GetComponent<RendererCom>()->GetModel()->FindNode("mixamorig:RightHand")->worldTransform;
    ballPos.x = worldTransform._41;
    ballPos.y = worldTransform._42;
    ballPos.z = worldTransform._43;

    std::shared_ptr<GameObject> grapeBall = GameObjectManager::Instance().Create();
    grapeBall->SetName("grapeBall");

    grapeBall->transform_->SetWorldPosition(ballPos);
    grapeBall->transform_->SetScale(DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f));

    const char* filename = "Data/Model/enemies/grape/grapeBall.mdl";
    std::shared_ptr<RendererCom> r = grapeBall->AddComponent<RendererCom>();
    r->LoadModel(filename);
    r->SetShaderID(SHADER_ID::UnityChanToon);
    r->SetIsShadowFall(true);

    grapeBall->AddComponent<GrapeBallCom>();
    
    //�����蔻��
    {
        std::shared_ptr<GameObject> attack = grapeBall->AddChildObject();
        attack->SetName("attack");

        std::shared_ptr<SphereColliderCom> attackCol = attack->AddComponent<SphereColliderCom>();
        attackCol->SetMyTag(COLLIDER_TAG::EnemyAttack);
        attackCol->SetJudgeTag(COLLIDER_TAG::Player);
        attackCol->SetRadius(0.4f);

        std::shared_ptr<WeaponCom> weapon = attack->AddComponent<WeaponCom>();
        weapon->SetObject(grapeBall);
        weapon->SetNodeParent(grapeBall);
        weapon->SetIsForeverUse(true);
        weapon->SetAttackDefaultStatus(1, 30,1,0);
    }

    //�W���X�g���p
    {
        std::shared_ptr<GameObject> justAttack = grapeBall->AddChildObject();
        justAttack->SetName("attackJust");
        std::shared_ptr<BoxColliderCom> justCol = justAttack->AddComponent<BoxColliderCom>();
        justCol->SetMyTag(COLLIDER_TAG::JustAvoid);
        justCol->SetJudgeTag(COLLIDER_TAG::Player);
        justCol->SetSize({ 2.0f,1,2.0f });

        justAttack->transform_->SetLocalPosition({ 0 ,0 ,119.5f });
    }
}



//��e���ɃA�j���[�V�������鎞��AITREE�����߂�
template<typename... Args>
void EnemyGrapeFarCom::OnDamageAnimAI_TREE(Args... args)
{
    for (AI_TREE i : std::initializer_list<AI_TREE>{ args... }) {
        damageAnimAiTreeId_.emplace_back(static_cast<int>(i));
    }
}

#pragma endregion




#pragma region �e

void GrapeBallCom::Start()
{
    std::shared_ptr<GameObject> player = GameObjectManager::Instance().Find("pico");
    DirectX::XMFLOAT3 playerPos = player->transform_->GetWorldPosition();
    playerPos.y += 1;
    DirectX::XMFLOAT3 myPos = GetGameObject()->transform_->GetWorldPosition();

    DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&myPos)));

    DirectX::XMFLOAT3 dir;
    DirectX::XMStoreFloat3(&dir, Dir);

    DirectX::XMFLOAT4 rota = QuaternionStruct::LookRotation(dir).dxFloat4;
    GetGameObject()->transform_->SetRotation(rota);
}

void GrapeBallCom::Update(float elapsedTime)
{
    float enemySpeed = 1;
    if (EnemyManager::Instance().GetIsSlow())
    {
        enemySpeed = EnemyManager::Instance().GetSlowSpeed();
    }
    //���E�̃X�s�[�h�ƍ��킹��
    float worldSpeed = elapsedTime * enemySpeed * Graphics::Instance().GetWorldSpeed();


    //������ς���
    if(!endFlag_)
    {
        std::shared_ptr<GameObject> player = GameObjectManager::Instance().Find("pico");
        DirectX::XMFLOAT3 playerPos = player->transform_->GetWorldPosition();
        playerPos.y += 1;
        DirectX::XMFLOAT3 myPos = GetGameObject()->transform_->GetWorldPosition();

        DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&myPos)));

        DirectX::XMFLOAT3 dir;
        DirectX::XMStoreFloat3(&dir, Dir);
        DirectX::XMFLOAT3 forward = GetGameObject()->transform_->GetWorldFront();

        //�p�x������ꍇ�͒ǔ��I��
        DirectX::XMVECTOR Forward = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&forward));
        float dot = DirectX::XMVector3Dot(Forward, Dir).m128_f32[0];
        if (dot < 0.5)endFlag_ = true;

        DirectX::XMFLOAT4 dirRota = QuaternionStruct::LookRotation(dir).dxFloat4;
        DirectX::XMFLOAT4 vecRota = QuaternionStruct::LookRotation(forward).dxFloat4;

        DirectX::XMFLOAT4 rota;
        DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&vecRota), DirectX::XMLoadFloat4(&dirRota), rotaAsistPower_ * worldSpeed));
        GetGameObject()->transform_->SetRotation(rota);
    }

    //�ړ�����
    {
        DirectX::XMFLOAT3 myPos = GetGameObject()->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 forward = GetGameObject()->transform_->GetWorldFront();

        DirectX::XMFLOAT3 moveVec;
        moveVec.x = myPos.x + (forward.x * speed_ * worldSpeed);
        moveVec.y = myPos.y + (forward.y * speed_ * worldSpeed);
        moveVec.z = myPos.z + (forward.z * speed_ * worldSpeed);

        //�ړ�
        GetGameObject()->transform_->SetWorldPosition(moveVec);
    }

    //�폜����
    {
        if (endFlag_)
        {
            removeTimer -= worldSpeed;
        }

        //���������炩�폜���Ԃ�������
        if (GetGameObject()->GetChildFind("attack")->GetComponent<WeaponCom>()->GetOnHit() 
            || removeTimer < 0)
        {
            GameObjectManager::Instance().Remove(GetGameObject());
        }
    }
}

void GrapeBallCom::OnGUI()
{
    ImGui::Checkbox("end", &endFlag_);
    ImGui::DragFloat("speed", &speed_, 0.1f);
    ImGui::DragFloat("rotaSpeed", &rotaAsistPower_, 0.1f);
}

#pragma endregion

