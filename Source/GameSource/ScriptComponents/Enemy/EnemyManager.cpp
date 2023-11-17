#include "EnemyManager.h"

#include <imgui.h>

#include "Components/System/GameObject.h"
#include "Components/TransformCom.h"

#include "GameSource/Math/easing.h"

//�ǉ��p
#include "Components\RendererCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"
#include "Components\CameraCom.h"
#include "../\CharacterStatusCom.h"

#include "EnemyCom.h"
#include "EnemyNearCom.h"
#include "EnemyFarCom.h"

#include "Graphics\Shaders\PostEffect.h"

// �X�V����
void EnemyManager::Update(float elapsedTime)
{
    //�X���[�߂菈��
    if (isSlow_)
    {
        //�C�[�W���O�Ŗ߂肩�������߂�
        slowTimer_ += elapsedTime;
        if (slowTimer_ >= slowSeconds_)
        {
            slowTimer_ = slowSeconds_;
            isSlow_ = false;
        }

        float ratio = slowTimer_ / slowSeconds_;
        float speed = Expo::easeIn(ratio, slowSpeed_, 1, 1);
        for (auto& nearEnemy : nearEnemies_)
        {
            nearEnemy.enemy.lock()->SetObjSpeed(speed);
        }

        //�v���C���[���G��
        player_.lock()->GetComponent<CharacterStatusCom>()
            ->SetInvincibleNonDamage(0.2f);
    }
}

//GUI
void EnemyManager::OnGui()
{
    ImGui::SetNextWindowPos(ImVec2(950, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("EnemyManager", nullptr, ImGuiWindowFlags_None))
    {
        //�G�̐�
        int flagCount = GetEnemyCount();
        ImGui::InputInt("enemyCount", &flagCount);

        //�ߐړI�ɍU���t���O�J�E���g
        flagCount = GetCurrentNearAttackCount();
        ImGui::InputInt("nearAttackCount", &flagCount);

        //�ߐړI�ɐڋ߃t���O�J�E���g
        flagCount = GetCurrentNearFlagCount();
        ImGui::InputInt("nearFlagCount", &flagCount);

        //�ߐړI�Ɍo�H�T���t���O�J�E���g
        flagCount = GetCurrentNearPathCount();
        ImGui::InputInt("nearPathCount", &flagCount);

        //�G�ǉ�
        if (ImGui::Button("Add"))
        {
            std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
            obj->SetName("picolabo");
            obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });
            obj->transform_->SetWorldPosition({ -10.0f , 0, 5 });
            obj->transform_->SetEulerRotation({ 0,180,0 });

            const char* filename = "Data/Model/picolabo/picolabo.mdl";
            std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
            r->LoadModel(filename);
            r->SetShaderID(SHADER_ID::UnityChanToon);

            ////����������
            //std::vector<ModelResource::Material>& materials = r->GetModel()->GetResourceShared()->GetMaterialsEdit();
            //materials[0].toonStruct._Emissive_Color.w = 0;


            std::shared_ptr<MovementCom> m = obj->AddComponent<MovementCom>();
            std::shared_ptr<CharacterStatusCom> status = obj->AddComponent<CharacterStatusCom>();

            std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();
            //a->PlayAnimation(5, true);

            std::shared_ptr<AnimatorCom> animator = obj->AddComponent<AnimatorCom>();

            std::shared_ptr<BoxColliderCom> c = obj->AddComponent<BoxColliderCom>();
            c->SetMyTag(COLLIDER_TAG::Enemy);
            c->SetJudgeTag(COLLIDER_TAG::Player | COLLIDER_TAG::Wall);
            c->SetSize(DirectX::XMFLOAT3(0.5f, 1.2f, 0.5f));
            c->SetOffsetPosition(DirectX::XMFLOAT3(0, 0.9f, 0));

            std::shared_ptr<EnemyNearCom> e = obj->AddComponent<EnemyNearCom>();

            //�W���X�g���p
            {
                std::shared_ptr<GameObject> justAttack = obj->AddChildObject();
                justAttack->SetName("picolaboAttackJust");
                std::shared_ptr<BoxColliderCom> justCol = justAttack->AddComponent<BoxColliderCom>();
                justCol->SetMyTag(COLLIDER_TAG::JustAvoid);
                justCol->SetJudgeTag(COLLIDER_TAG::Player);
                justCol->SetSize({ 1.3f,1,1.3f });

                justAttack->transform_->SetLocalPosition({ -1.569f ,0 ,95.493f });
            }

            //�����o���p�����蔻��
            {
                std::shared_ptr<GameObject> pushBack = obj->AddChildObject();
                pushBack->SetName("PushBackObj");
                std::shared_ptr<SphereColliderCom> col = pushBack->AddComponent<SphereColliderCom>();
                col->SetMyTag(COLLIDER_TAG::EnemyPushBack);
                col->SetJudgeTag(COLLIDER_TAG::PlayerPushBack | COLLIDER_TAG::EnemyPushBack);
                col->SetPushBack(true);
                col->SetPushBackObj(obj);
            }

            //��("RightHand")
            {
                std::shared_ptr<GameObject> sword = obj->AddChildObject();
                sword->SetName("Banana");
                sword->transform_->SetScale(DirectX::XMFLOAT3(3, 3, 3));
                sword->transform_->SetEulerRotation(DirectX::XMFLOAT3(7, -85, 108));
                sword->transform_->SetLocalPosition(DirectX::XMFLOAT3(11, -6, -15));

                const char* filename = "Data/Model/Swords/banana/banana.mdl";
                std::shared_ptr<RendererCom> r = sword->AddComponent<RendererCom>();
                r->LoadModel(filename);
                r->SetShaderID(SHADER_ID::UnityChanToon);

                std::shared_ptr<CapsuleColliderCom> attackCol = sword->AddComponent<CapsuleColliderCom>();
                attackCol->SetMyTag(COLLIDER_TAG::EnemyAttack);
                attackCol->SetJudgeTag(COLLIDER_TAG::Player);
                attackCol->SetRadius(0.19f);

                std::shared_ptr<WeaponCom> weapon = sword->AddComponent<WeaponCom>();
                weapon->SetObject(sword->GetParent());
                weapon->SetNodeParent(sword->GetParent());
                weapon->SetNodeName("RightHand");
                weapon->SetColliderUpDown({ 1.36f,0 });
                weapon->SetIsForeverUse(true);
            }

        }

    }

    ImGui::DragInt("A", &killCount_);
    ImGui::DragFloat4("killCountSprite_", &killCountSpritePos_.x, 0.1f);
    ImGui::DragFloat3("killStringSpritePos_", &killStringSpritePos_.x, 0.1f);
    ImGui::DragFloat3("saraSpritePos_", &saraSpritePos_.x, 0.1f);
    ImGui::End();
}

void EnemyManager::Render2D(float elapsedTime)
{
    ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();

    //�M
    {
        saraSprite_->Render(dc, saraSpritePos_.x, saraSpritePos_.y, saraSprite_->GetTextureWidth() * saraSpritePos_.z, saraSprite_->GetTextureHeight() * saraSpritePos_.z
            , 0, 0, saraSprite_->GetTextureWidth(), saraSprite_->GetTextureHeight()
            , 0, 1, 1, 1, 1);
    }

    //�����J�E���g
    {
        killCount_ = GetEnemyCount();

        //�P��
        enemyCount_->Render(dc, killCountSpritePos_.x, killCountSpritePos_.y, enemyCount_->GetTextureWidth() * 0.1f * killCountSpritePos_.z, enemyCount_->GetTextureHeight() * killCountSpritePos_.z
            , 170 * killCount_, 0, enemyCount_->GetTextureWidth() * 0.1f, enemyCount_->GetTextureHeight()
            , 0, 1, 1, 1, 1);
        //�Q��
        if (killCount_ >= 10)
        {
            int keta = killCount_ / 10 % 10;
            enemyCount_->Render(dc, killCountSpritePos_.x - killCountSpritePos_.w, killCountSpritePos_.y, enemyCount_->GetTextureWidth() * 0.1f * killCountSpritePos_.z, enemyCount_->GetTextureHeight() * killCountSpritePos_.z
                , 170 * keta, 0, enemyCount_->GetTextureWidth() * 0.1f, enemyCount_->GetTextureHeight()
                , 0, 1, 1, 1, 1);
        }
        //�R��
        if (killCount_ >= 100)
        {
            int keta = killCount_ / 100 % 10;
            enemyCount_->Render(dc, killCountSpritePos_.x - killCountSpritePos_.w * 2, killCountSpritePos_.y, enemyCount_->GetTextureWidth() * 0.1f * killCountSpritePos_.z, enemyCount_->GetTextureHeight() * killCountSpritePos_.z
                , 170 * keta, 0, enemyCount_->GetTextureWidth() * 0.1f, enemyCount_->GetTextureHeight()
                , 0, 1, 1, 1, 1);
        }
    }

    //��������
    {
        killStringSprite_->Render(dc, killStringSpritePos_.x, killStringSpritePos_.y, killStringSprite_->GetTextureWidth()  * killStringSpritePos_.z, killStringSprite_->GetTextureHeight() * killStringSpritePos_.z
            , 0, 0, killStringSprite_->GetTextureWidth() , killStringSprite_->GetTextureHeight()
            , 0, 1, 1, 1, 1);
    }
}

void EnemyManager::EnemyMaskRender(PostEffect* postEff, std::shared_ptr<CameraCom> maskCamera)
{
    for (auto& e : nearEnemies_)
    {
        e.enemy.lock()->GetComponent<EnemyCom>()->MaskRender(postEff, maskCamera);
    }
}

// �G�o�^
void EnemyManager::Register(std::shared_ptr<GameObject> enemy, EnemyKind enemyKind)
{
    //�G�f�[�^���쐬
    EnemyData e;
    e.enemy = enemy;
    e.enemy.lock()->GetComponent<EnemyCom>()->SetID(AI_ID::ENEMY_INDEX + currentIndex_);

    //�o�^
    switch (enemyKind)
    {
    case EnemyManager::EnemyKind::NEAR_ENEMY:
        nearEnemies_.emplace_back(e);
        break;
    case EnemyManager::EnemyKind::FAR_ENEMY:
        farEnemies_.emplace_back(e);
        break;
    default:
        break;
    }
   
    currentIndex_++;
}

//�ߐړG�̍U���J�E���g�擾
int EnemyManager::GetCurrentNearAttackCount()
{
    int attackCount = 0;
    for (auto& e : nearEnemies_)
    {
        if (e.enemy.expired())continue;
        if (e.enemy.lock()->GetComponent<EnemyCom>()->GetIsAttackFlag())
            attackCount++;
    }

    return attackCount;
}

//�ߐړG�̐ڋ߃J�E���g�擾
int EnemyManager::GetCurrentNearFlagCount()
{
    int nearFlagCount = 0;
    for (auto& e : nearEnemies_)
    {
        if (e.enemy.expired())continue;
        if (e.enemy.lock()->GetComponent<EnemyNearCom>()->GetIsNearFlag())
            nearFlagCount++;
    }

    return nearFlagCount;
}

//�ߐړG�̌o�H�T���J�E���g�擾
int EnemyManager::GetCurrentNearPathCount()
{
    int nearPathCount = 0;
    for (auto& e : nearEnemies_)
    {
        if (e.enemy.expired())continue;
        if (e.enemy.lock()->GetComponent<EnemyNearCom>()->GetIsPathFlag())
            nearPathCount++;
    }

    return nearPathCount;
}


/////   AI�֌W   /////

// ���b�Z�[�W��M�����Ƃ��̏���
bool EnemyManager::OnMessage(const Telegram& telegram)
{
    switch (telegram.msg)
    {
    case MESSAGE_TYPE::MsgAskNearRight: //�v���C���[�ւ̐ڋ߂�v��
    {
        DirectX::XMFLOAT3 playerPos= player_.lock()->transform_->GetWorldPosition();
        int nearCount = 0;  //�߂��ɂ���G�̐�
        for (EnemyData& e : nearEnemies_)
        {
            if (e.enemy.expired())continue;
            //���M�҂̏ꍇ�͔�΂�
            if (telegram.sender == e.enemy.lock()->GetComponent<EnemyCom>()->GetID())continue;

            if (nearCount >= nearEnemyLevel_.inRadiusCount)
                break;

            //�ߐړG�ɃL���X�g
            std::shared_ptr<EnemyNearCom> nearEnemy = e.enemy.lock()->GetComponent<EnemyNearCom>();

            if(nearEnemy->GetIsNearFlag())
                nearCount++;
        }
        //�߂��ɂ���G�����߂��Ă��鐔��菬������
        if (nearCount < nearEnemyLevel_.inRadiusCount)
        {
            //�ڋߋ��𑗂�
            SendMessaging(static_cast<int>(AI_ID::AI_INDEX), telegram.sender, MESSAGE_TYPE::MsgGiveNearRight);
            return true;
        }
        break;
    }
    case MESSAGE_TYPE::MsgAskAttackRight: //�v���C���[�ւ̍U����v��
    {
        std::shared_ptr<GameObject> enemy = GetEnemyFromId(telegram.sender);
        int attackCount = 0;  //�U�����̓G�J�E���g
        //�ߐڂ����u�����f
        //�ߐړG�ɃL���X�g
        std::shared_ptr<EnemyNearCom> nearEnemy = enemy->GetComponent<EnemyNearCom>();
        //�ߐړG����
        if (nearEnemy)
        {
            attackCount = GetCurrentNearAttackCount();

            //�����U���\����
            if (attackCount < nearEnemyLevel_.togetherAttackCount)
            {
                //�U�����𑗂�
                SendMessaging(static_cast<int>(AI_ID::AI_INDEX), telegram.sender, MESSAGE_TYPE::MsgGiveAttackRight);
                return true;
            }
            break;
        }
        //���u�I����
        else
        {

        }

    }
    }
    return false;
}

//�G�̐��擾
int EnemyManager::GetEnemyCount()
{
    int enemyCount = 0;
    enemyCount += nearEnemies_.size();
    return enemyCount;
}

//ID����G���Q�b�g
std::shared_ptr<GameObject> EnemyManager::GetEnemyFromId(int id)
{
    for (EnemyData& e : nearEnemies_)
    {
        if (e.enemy.expired())continue;
        if (e.enemy.lock()->GetComponent<EnemyCom>()->GetID() == id)
            return e.enemy.lock();
    }
    for (EnemyData& e : farEnemies_)
    {
        if (e.enemy.lock()->GetComponent<EnemyCom>()->GetID() == id)
            return e.enemy.lock();
    }

    return nullptr;
}

//��̓G���폜
void EnemyManager::EraseExpiredEnemy()
{
    //�ߋ���
    for (int i = 0; i < nearEnemies_.size();)
    {
        if (nearEnemies_[i].enemy.expired())
            nearEnemies_.erase(nearEnemies_.begin() + i);
        else
            i++;
    }
}

void EnemyManager::SetEnemySpeed(float speed, float seconds)
{
    isSlow_ = true;
    slowSeconds_ = seconds;
    slowSpeed_ = speed;
    slowTimer_ = 0;
}



// ���b�Z�[�W���M�֐�
void EnemyManager::SendMessaging(int sender, int receiver, MESSAGE_TYPE msg)
{
    if (receiver == static_cast<int>(AI_ID::AI_INDEX))
    {// MetaAI���̎�
        //���b�Z�[�W�f�[�^���쐬
        Telegram telegram(sender, receiver, msg);
        // ���b�Z�[�W���M
        OnMessage(telegram);
    }
    else
    {// �G�l�~�[����M�҂̂Ƃ��A�ǂ̃G�l�~�[�ɑ��M���邩
        // ��M�҂̃|�C���^���擾
        std::weak_ptr<GameObject> receiveEnemy = GetEnemyFromId(receiver);
        //���V�[�o�[���Ȃ��Ƃ��֐����I������
        if(!receiveEnemy.lock()->GetComponent<EnemyCom>()) return;
        //���b�Z�[�W�f�[�^���쐬
        Telegram telegram(sender, receiver, msg);
        //�f�B���C�������b�Z�[�W�i�����z�����b�Z�[�W�j
        receiveEnemy.lock()->GetComponent<EnemyCom>()->OnMessage(telegram);
    }
}
