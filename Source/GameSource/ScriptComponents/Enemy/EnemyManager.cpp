#include "EnemyManager.h"

#include <imgui.h>

#include "Components/System/GameObject.h"
#include "Components/TransformCom.h"
#include "EnemyCom.h"
#include "EnemyNearCom.h"
#include "EnemyFarCom.h"

// �X�V����
void EnemyManager::Update(float elapsedTime)
{
    
}

//GUI
void EnemyManager::OnGui()
{
    //�ߐړI�ɐڋ߃t���O�J�E���g
    int nearFlagCount = GetCurrentNearFlagCount();
    ImGui::InputInt("nearFlagCount", &nearFlagCount);

    //�ߐړI�Ɍo�H�T���t���O�J�E���g
    int nearPathCount = GetCurrentNearPathCount();
    ImGui::InputInt("nearPathCount", &nearPathCount);
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
    for (int i = 0; i < nearEnemies_.size();)
    {
        if (nearEnemies_[i].enemy.expired())
            nearEnemies_.erase(nearEnemies_.begin() + i);
        else
            i++;
    }
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
