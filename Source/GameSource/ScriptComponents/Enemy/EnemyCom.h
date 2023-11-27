#pragma once

#include "Components\System\Component.h"

#include "Graphics/Sprite/Sprite.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorData.h"
#include "BehaviorTree/NodeBase.h"

#include "TelegramEnemy.h"

class PostEffect;
class CameraCom;


//AI�J��
enum class AI_TREE
{
    NONE_AI,
    ROOT,

    //2�w
    //�e�FROOT 
    BATTLE,
    SCOUT,

    //3�w
    //�e�FSCOUT 
    WANDER,
    IDLE,

    //�e�FBATTLE
    ATTACK,
    ROUTE,
    BACK_MOVE,
    ATTACK_IDLE,
    PURSUIT,

    //4�w
    //�e�FATTACK
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

    // sprite�`��
    virtual void Render2D(float elapsedTime);

    //EnemyCom�N���X
public:
    //mask
    void MaskRender(PostEffect* postEff, std::shared_ptr<CameraCom> maskCamera);

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
    const DirectX::XMFLOAT3& GetTargetPosition() const { return targetPosition_; }

    //�^�[�Q�b�g�ʒu�Ɉړ��A��]
    void GoTargetMove(bool isMove = true, bool isTurn = true);

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

    //�U���O�ҋ@�t���O
    bool GetIsAttackIdleFlag() { return isAttackIdle_; }
    void SetIsAttackIdleFlag(bool flag) { isAttackIdle_ = flag; }

    //��e�[�������オ�胂�[�V����
    void SetStandUpMotion();
    void StandUpUpdate();

    const MoveDataEnemy& GetMoveDataEnemy() const { return moveDataEnemy_; }

    //AI�֌W

    // ���b�Z�[�W��M�����Ƃ��̏���
    virtual bool OnMessage(const Telegram& msg);

    int GetID() { return enemyId_; }
    void SetID(int id) { enemyId_ = id; }

private:
    //�A�j���[�V�����̍X�V
    void AnimationSetting();

private:    //���ꂾ�����̂��h���N���X�Ŏg���Ȃ��̂ŁA�h���N���X�ł��쐬����
    //��e���ɃA�j���[�V�������鎞��AITREE�����߂�
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);

protected:
    //�A�j���[�V�����������ݒ�
    virtual void AnimationInitialize() {}

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

    float damageEffTimer_;

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
    int getUpAnim_;
    bool isStandUpMotion_ = false;
    bool playStandUp_ = false;

    //�U���t���O
    bool isAttackFlag_ = false;

    //�U���O�ҋ@
    bool isAttackIdle_ = false;

    //�ړ�
    MoveDataEnemy moveDataEnemy_;

    //HPBar
    std::unique_ptr<Sprite>     hpSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Enemy/enemyHp.png");
    std::unique_ptr<Sprite>     hpBackSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Enemy/enemyHpGage.png");
    std::unique_ptr<Sprite>     hpMaskSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Enemy/enemyHpMask.png");
    DirectX::XMFLOAT2 sP{0,0};
    DirectX::XMFLOAT3 saP{0,0,0};
    //���ʔԍ�
    int enemyId_ = -1;
};
