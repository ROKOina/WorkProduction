#include "EnemyManager.h"

#include "Components/System/GameObject.h"
#include "Components/TransformCom.h"
#include "EnemyCom.h"
#include "EnemyNearCom.h"
#include "EnemyFarCom.h"

// �X�V����
void EnemyManager::Update(float elapsedTime)
{
    
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
            //���M�҂̏ꍇ�͔�΂�
            if (telegram.sender == e.enemy.lock()->GetComponent<EnemyCom>()->GetID())continue;

            if (nearCount >= nearEnemyLevel.inRadiusCount)
                break;

            //�ߐړG�ɃL���X�g
            std::shared_ptr<EnemyNearCom> nearEnemy = std::static_pointer_cast<EnemyNearCom>(e.enemy.lock()->GetComponent<EnemyCom>());

            if(nearEnemy->GetIsNearFlag())
                nearCount++;

            //DirectX::XMFLOAT3 enemyPos = nearEnemy.enemy.lock()->transform_->GetWorldPosition();
            ////�v���C���[�Ƃ̋������Z�o
            //float length = DirectX::XMVectorGetX(
            //    DirectX::XMVector3Length(
            //        DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&enemyPos))));
            ////�ڋ߂��Ă���Ȃ�
            //if (length <= nearEnemyLevel.radius)
            //    nearCount++;
        }
        //�߂��ɂ���G�����߂��Ă��鐔��菬������
        if (nearCount < nearEnemyLevel.inRadiusCount)
        {
            //�ڋߋ��𑗂�
            SendMessaging(static_cast<int>(AI_ID::AI_INDEX), telegram.sender, MESSAGE_TYPE::MsgGiveNearRight);
            return true;
        }
        break;
    }
    }
    return false;
}

//ID����G���Q�b�g
std::shared_ptr<GameObject> EnemyManager::GetEnemyFromId(int id)
{
    for (EnemyData& e : nearEnemies_)
    {
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
