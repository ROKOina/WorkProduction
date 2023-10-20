#pragma once

#include "Components\System\Component.h"
#include "GameSource/ScriptComponents/Weapon/WeaponCom.h"

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

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //CharacterStatusCom�N���X
public:
    //�_���[�W
    void OnDamage(DirectX::XMFLOAT3& power = DirectX::XMFLOAT3(0, 0, 0), ATTACK_SPECIAL_TYPE attackType = ATTACK_SPECIAL_TYPE::NORMAL);

    //���G����
    void SetInvincibleTime(float time) { damageInvincibleTime_ = time; }
    float GetInvincibleTime() { return damageInvincibleTime_; }

    bool GetIsInvincible() {
        return isDamage_;
    }

    //�_���[�W���󂯂�������true��Ԃ�
    bool GetFrameDamage() { return isFrameDamage_; }

    //�_���[�W�A�j���[�V�������Ȃ�true
    bool GetDamageAnimation() { return isAnimDamage_; }

    //�U���ɂ��ړ��𐧌�
    void SetAttackNonMove(bool flag) { isAttackNonMove_ = flag; }
    bool GetAttackNonMove() { return isAttackNonMove_; }

    //�_���[�W�̎�ނ��擾
    ATTACK_SPECIAL_TYPE GetDamageType() { return damageType_; }

private:
    //��Ԍn
    bool isDamage_ = false; //�_���[�W���󂯂Ă��鎞��true
    bool isFrameDamage_ = false; //�_���[�W���󂯂�������true
    bool isAnimDamage_ = false; //�_���[�W�A�j���[�V��������true
    bool oneFrameJudge = false;
    ATTACK_SPECIAL_TYPE damageType_ = ATTACK_SPECIAL_TYPE::NORMAL; //����ȍU�����P�t���[���ۑ�
    float damageInvincibleTime_ = 0.5f; //�_���[�W���̖��G����
    float damageTimer_ = 0; //�_���[�W���̃^�C�}�[

    int hp_ = 100;

    //�U���ɂ��ړ��𐧌�
    bool isAttackNonMove_ = false;
};