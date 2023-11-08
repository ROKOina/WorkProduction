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
    //�_���[�W�iinvincibleNonDamage�̓v���C���[�A�j���[�V�����֌W�Ȃ����G���Ԕ���j
    void OnDamage(std::shared_ptr<WeaponCom> weapon, int minasHP, DirectX::XMFLOAT3& power = DirectX::XMFLOAT3(0, 0, 0)
        , ATTACK_SPECIAL_TYPE attackType = ATTACK_SPECIAL_TYPE::NORMAL, float invincibleNonDamage = -1.0f);

    //���G����
    void SetInvincibleTime(float time) { damageInvincibleTime_ = time; }
    float GetInvincibleTime() { return damageInvincibleTime_; }

    bool GetIsInvincible() { return isInvincible_; }

    //�_���[�W�Ƃ͕ʂ̖��G����
    void SetInvincibleNonDamage(float time){
        invincibleSetTimer = time;
        invincibleSetFlag = true;
        isInvincible_ = true;
    }

    //�_���[�W���󂯂�������true��Ԃ�
    bool GetFrameDamage() { return isFrameDamage_; }

    //�_���[�W�A�j���[�V�������Ȃ�true
    bool GetDamageAnimation() { return isAnimDamage_; }

    //�U���ɂ��ړ��𐧌�
    void SetAttackNonMove(bool flag) { isAttackNonMove_ = flag; }
    bool GetAttackNonMove() { return isAttackNonMove_; }

    //hp�擾
    void SetMaxHP(int hp) { maxHp_ = hp; }
    int GetMaxHP() { return maxHp_; }
    void SetHP(int hp) { hp_ = hp; }
    int GetHP() { return hp_; }

    //�_���[�W�̎�ނ��擾
    ATTACK_SPECIAL_TYPE GetDamageType() { return damageType_; }

private:
    //��Ԍn
    bool isInvincible_ = false; //���G���Ԃ�true
    bool isFrameDamage_ = false; //�_���[�W���󂯂�������true
    bool isAnimDamage_ = false; //�_���[�W�A�j���[�V��������true
    bool oneFrameJudge_ = false;    //��t���[���𑪂�

    float damageInvincibleTime_ = 1.5f; //�_���[�W���̖��G����
    float damageTimer_ = 0; //�_���[�W���̃^�C�}�[

    //��莞�Ԗ��G�ɂȂ�
    float invincibleSetTimer = 0;
    bool invincibleSetFlag = false;

    //�_���[�W���
    ATTACK_SPECIAL_TYPE damageType_ = ATTACK_SPECIAL_TYPE::NORMAL; //����ȍU�����P�t���[���ۑ�

    //hp
    int hp_ = 100;
    int maxHp_ = 100;

    //��e���ɕۑ�����
    std::weak_ptr<WeaponCom> saveWeapon_;

    //�U���ɂ��ړ��𐧌�
    bool isAttackNonMove_ = false;
};