#pragma once

#include "Components\System\Component.h"


#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorData.h"
#include "BehaviorTree/NodeBase.h"

class EnemyCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    EnemyCom() {}
    ~EnemyCom() { activeNode_.release(); }

    // ���O�擾
    const char* GetName() const override { return "Enemy"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //EnemyCom�N���X
public:
    // �^�[�Q�b�g�ʒu�������_���ݒ�
    void SetRandomTargetPosition();

    void SetTargetPosition(DirectX::XMFLOAT3 pos) { targetPosition_ = pos; }
    DirectX::XMFLOAT3 GetTargetPosition() { return targetPosition_; }

    //�v���C���[���߂���true
    bool SearchPlayer();

    float GetAttackRange() { return attackRange_; }

private:
    //�A�j���[�V�����������ݒ�
    void AnimationInitialize();

    //AI
    std::unique_ptr<BehaviorTree> aiTree_;
    std::unique_ptr<BehaviorData> behaviorData_;
    std::unique_ptr<NodeBase> activeNode_;

    DirectX::XMFLOAT3 targetPosition_;
    float attackRange_ = 2;
};
