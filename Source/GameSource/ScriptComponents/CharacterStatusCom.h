#pragma once

#include "Components\System\Component.h"

//�L�����N�^�[�̏�Ԃ��Ǘ�
class CharacterStatusCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    CharacterStatusCom() {}
    ~CharacterStatusCom() {}

    // ���O�擾
    const char* GetName() const override { return "CharacterStatus"; }

    // �J�n����
    void Start() override;

    //�X�V�O
    void UpdateStart() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //CharacterStatusCom�N���X
public:
    //�_���[�W
    void OnDamage(DirectX::XMFLOAT3& power = DirectX::XMFLOAT3(0, 0, 0));

    bool GetIsInvincible() {
        return isDamage_;
    }

    //�_���[�W���󂯂�������true��Ԃ�
    bool GetFrameDamage() { return isFrameDamage_; }



private:
    //��Ԍn
    bool isDamage_ = false; //�_���[�W���󂯂Ă��鎞��true
    bool isFrameDamage_ = false; //�_���[�W���󂯂�������true
    bool oneFrameJudge = false;
    float damageInvincibleTime_ = 0.5f; //�_���[�W���̖��G����
    float damageTimer_ = 0; //�_���[�W���̃^�C�}�[

};