#pragma once

#include "Components\System\Component.h"

class EnemyCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    EnemyCom() {}
    ~EnemyCom() {}

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
    //�_���[�W
    void OnDamage(DirectX::XMFLOAT3& power = DirectX::XMFLOAT3(0, 0, 0));

    bool GetIsInvincible() {
        return isDamage_;
    }

private:
    //�A�j���[�V�����������ݒ�
    void AnimationInitialize();



private:

    //��Ԍn
    bool isDamage_ = false; //�_���[�W���󂯂Ă��鎞��true
    float damageInvincibleTime_ = 1; //�_���[�W���̖��G����
    float damageTimer_ = 0; //�_���[�W���̃^�C�}�[

};
