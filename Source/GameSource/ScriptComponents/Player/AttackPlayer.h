#pragma once

#include <memory>

//�O���錾
class PlayerCom;
class GameObject;

//�v���C���[�̍U���̓����̂݊Ǘ�����
class AttackPlayer
{
public:
    AttackPlayer(std::shared_ptr<PlayerCom> player);
    AttackPlayer() {}
    ~AttackPlayer();

    void Update(float elapsedTime);

public://int�ŃX�e�[�g��Ԃ��A�X�e�[�g���R�[�h�ɂ���
    enum ATTACK_CODE
    {
        EnterAttack = 99,   //�U���A�j���[�V��������
    };

public: 
    void NormalAttack();

private:
    int NormalAttackUpdate(float elapsedTime);

public:
    //�U���A�j���[�V��������
    bool DoAnimation() { return state == ATTACK_CODE::EnterAttack; }

    //�R���{�o���邩����
    bool DoComboAttack();

private:
    //�A�V�X�g�͈͂����G���ċ߂��G��Ԃ�
    std::shared_ptr<GameObject> AssistGetNearEnemy();

    //�G�ɐڋ߂���( true:�ڋߊ����@false:�ڋߒ� )
    bool ApproachEnemy(std::shared_ptr<GameObject> enemy, float dist, float speed = 1);

    //�G�̕����։�]���� ( true:���� )
    bool ForcusEnemy(float elapsedTime, std::shared_ptr<GameObject> enemy, float rollSpeed);

private:
    std::weak_ptr<PlayerCom> player;

    std::shared_ptr<GameObject> enemyCopy;  //�G�ۑ�

    //���݂̍U���̎��
    enum class ATTACK_FLAG
    {
        Normal,

        Null,
    };
    ATTACK_FLAG attackFlagState = ATTACK_FLAG::Null;

    //�U���̓������Ǘ�
    int state = -1;
};

/*
�R���{�}



*/