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
    //�_�b�V���R���{�ԍ��������Ŏw��i�P����j
    void DashAttack(int comboNum);

private:
    int DashAttackUpdate(float elapsedTime);

public:
    //�U���A�j���[�V��������
    bool EndAttackState() { return state_ == ATTACK_CODE::EnterAttack; }

    //�R���{�o���邩����
    bool DoComboAttack();

    //���O�̍U�����������Ă��邩
    bool OnHitEnemy() { return onHitEnemy_; }

    void ResetState() { state_ = -1; }

    //�G�𑨂��Ă��邩
    bool ComboReadyEnemy() { 
        if (enemyCopy_)return true;
        return false;
    }

private:
    //�A�V�X�g�͈͂����G���ċ߂��G��Ԃ�
    std::shared_ptr<GameObject> AssistGetNearEnemy();

    //�G�ɐڋ߂���( true:�ڋߊ����@false:�ڋߒ� )
    bool ApproachEnemy(std::shared_ptr<GameObject> enemy, float dist, float speed = 1);

    //�G�̕����։�]���� ( true:���� )
    bool ForcusEnemy(float elapsedTime, std::shared_ptr<GameObject> enemy, float rollSpeed);

private:
    bool onHitEnemy_ = false;

    std::weak_ptr<PlayerCom> player_;

    std::shared_ptr<GameObject> enemyCopy_;  //�G�ۑ�

    //���݂̍U���̎��
    enum class ATTACK_FLAG
    {
        Normal,
        Dash,

        Null,
    };
    ATTACK_FLAG attackFlagState_ = ATTACK_FLAG::Null;

    //�U���̓������Ǘ�
    int state_ = -1;
};

/*
�R���{�}�i��:���̍s���A�O:�ǂ���ł��q����j

�_�b�V��
DASH������DASH�����O��

*/