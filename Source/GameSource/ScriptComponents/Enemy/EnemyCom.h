#pragma once

#include "Components\System\Component.h"


#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorData.h"
#include "BehaviorTree/NodeBase.h"

#include "TelegramEnemy.h"

//�A�j���[�V�������X�g
enum ANIMATION_ENEMY
{
    WALK,
    RUN,
    RUN_BACK,
    JUMP,
    IDEL,
    KICK,
    DAMAGE,
    RIGHT_STRAIGHT01,
    LEFT_UPPER01,
    DAMAGE_FALL,
    DAMAGE_IN_AIR,
    DAMAGE_GO_FLY,
    FALL_STAND_UP,
    DAMAGE_FALL_END,
    ATTACK01_SWORD,
    RUN_SWORD,
    IDLE_SWORD,
    WALK_SWORD,
};

//AI�J��
enum class AI_TREE
{
    NONE,
    ROOT,

    //2�w
    //ROOT 
    BATTLE,
    SCOUT,

    //3�w
    //SCOUT 
    WANDER,
    IDLE,

    //BATTLE
    ATTACK,
    PURSUIT,

    //4�w
    //ATTACK
    NORMAL,
};

class EnemyCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    EnemyCom() {}
    ~EnemyCom() { }

    // ���O�擾
    virtual const char* GetName() const = 0;

    // �J�n����
    virtual void Start();

    // �X�V����
    virtual void Update(float elapsedTime);

    // GUI�`��
    virtual void OnGUI();

    //EnemyCom�N���X
private:
    struct MoveDataEnemy
    {
        float walkSpeed = 1.0f;
        float walkMaxSpeed = 2.0f;
        float runSpeed = 1.0f;
        float runMaxSpeed = 5.0f;
    };

public:
    // �^�[�Q�b�g�ʒu�������_���ݒ�
    void SetRandomTargetPosition();

    void SetTargetPosition(DirectX::XMFLOAT3 pos) { targetPosition_ = pos; }
    DirectX::XMFLOAT3 GetTargetPosition() { return targetPosition_; }

    //�v���C���[���߂���true
    bool SearchPlayer();

    float GetAttackRange() { return attackRange_; }

    //�_���[�W�m�F
    bool OnDamageEnemy();

    //�W���X�g����t���O�I��
    void SetIsJustAvoid(bool flag) { isJustAvoid_ = flag; }
    bool GetIsJustAvoid() { return isJustAvoid_; }

    //�U���t���O
    void SetIsAttackFlag(bool falg) { isAttackFlag_ = falg; }
    bool GetIsAttackFlag() { return isAttackFlag_; }

    //��e�[�������オ�胂�[�V����
    void SetStandUpMotion();
    void StandUpUpdate();

    const MoveDataEnemy& GetMoveDataEnemy() const { return moveDataEnemy; }

    //AI�֌W

    // ���b�Z�[�W��M�����Ƃ��̏���
    virtual bool OnMessage(const Telegram& msg);

    int GetID() { return enemyId_; }
    void SetID(int id) { enemyId_ = id; }

private:    //���ꂾ�����̂��h���N���X�Ŏg���Ȃ��̂ŁA�h���N���X�ł��쐬����
    //��e���ɃA�j���[�V�������鎞��AITREE�����߂�
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);

protected:
    //�A�j���[�V�����������ݒ�
    virtual void AnimationInitialize(){}

    //�_���[�W����
    void DamageProcess(float elapsedTime);

    //�W���X�g���p����o��
    void justColliderProcess();

    //�d�͐ݒ�
    void GravityProcess(float elapsedTime);

    //AI
    std::shared_ptr<BehaviorTree> aiTree_;
    std::shared_ptr<BehaviorData> behaviorData_;
    std::shared_ptr<NodeBase> activeNode_;

    //�_���[�WAITREE�ԍ�
    std::vector<int> damageAnimAiTreeId_;
    bool isAnimDamage_ = false; //�_���[�W�A�j���[�V��������Ƃ���true
    bool oldAnimDamage_ = false;

    DirectX::XMFLOAT3 targetPosition_;
    //�U���͈�
    float attackRange_ = 2;
    //���G�͈�
    float searchRange_ = 7;

    //�W���X�g�����ActionDerived�ƌq������
    bool isJustAvoid_ = false;

    //�W�����v��e��
    bool isJumpDamage_ = false;
    //�W�����v��e��̏d�͂Ȃ����^�C�}�[
    float skyGravityZeroTimer_ = 0;
    //�d�͐ݒ�p
    bool isSetGravity_ = false;

    //�N���オ�胂�[�V������
    bool isStandUpMotion_ = false;
    bool playStandUp_ = false;

    //�U���t���O
    bool isAttackFlag_ = false;

    //�ړ�
    MoveDataEnemy moveDataEnemy;

    //���ʔԍ�
    int enemyId_ = -1;
};
