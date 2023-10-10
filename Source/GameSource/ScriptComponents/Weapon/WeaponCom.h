#pragma once

#include "Components\System\Component.h"

#include "SystemStruct\QuaternionStruct.h"
#include <map>

//�U���̎�ރ^�O
enum ATTACK_SPECIAL_TYPE : uint64_t
{
    NORMAL = 1 << 0,

    UNSTOP = 1 << 1,
    JUMP_START = 1 << 2,
    JUMP_NOW = 1 << 3,
};
static ATTACK_SPECIAL_TYPE operator| (ATTACK_SPECIAL_TYPE L, ATTACK_SPECIAL_TYPE R)
{
    return static_cast<ATTACK_SPECIAL_TYPE>(static_cast<uint64_t>(L) | static_cast<uint64_t>(R));
}
static ATTACK_SPECIAL_TYPE operator& (ATTACK_SPECIAL_TYPE L, ATTACK_SPECIAL_TYPE R)
{
    return static_cast<ATTACK_SPECIAL_TYPE>(static_cast<uint64_t>(L) & static_cast<uint64_t>(R));
}
static bool operator== (ATTACK_SPECIAL_TYPE L, ATTACK_SPECIAL_TYPE R)
{
    if (static_cast<uint64_t>((static_cast<ATTACK_SPECIAL_TYPE>(L) & static_cast<ATTACK_SPECIAL_TYPE>(R))) == 0)
        return false;
    return true;
}
static bool operator!= (ATTACK_SPECIAL_TYPE L, ATTACK_SPECIAL_TYPE R)
{
    if (static_cast<uint64_t>((static_cast<ATTACK_SPECIAL_TYPE>(L) & static_cast<ATTACK_SPECIAL_TYPE>(R))) == 0)
        return true;
    return false;
}


class WeaponCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    WeaponCom() {}
    ~WeaponCom() {}

    // ���O�擾
    const char* GetName() const override { return "Weapon"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //WeaponCom�N���X
public:
    //�I�u�W�F�N�g���Z�b�g
    void SetObject(std::shared_ptr<GameObject> obj) { parentObject_ = obj; }
    //���O���Z�b�g
    void SetNodeName(const char* name) { nodeName_ = name; }

    //�U���̃X�e�[�^�X�\����
    struct AttackStatus
    {
        int damage = 0;     //�A�^�b�N�_���[�W
        float impactPower = 0;  //������΂���
        //������΂�����
        float front = 1;    //1~0�ŋ��������߂�
        float up = 0;       //����1�Ȃ�΂߂ɂȂ�

        //�U�����̃A�j���[�V�����X�s�[�h
        float animSpeed = 1;

        //����ȍU�����i�W�����v�Ȃǁj
        ATTACK_SPECIAL_TYPE specialType = ATTACK_SPECIAL_TYPE::NORMAL;
    };
    //�A�j���[�V�����ƃX�e�[�^�X��R�Â���
    void SetAttackStatus(int animIndex, int damage, float impactPower, float front = 1, float up = 0, float animSpeed = 1, ATTACK_SPECIAL_TYPE specialAttack = ATTACK_SPECIAL_TYPE::NORMAL);

    //�q�b�g�m�F
    bool GetOnHit() { return onHit_; }

    //�U���A�j���[�V������
    bool GetIsAttackAnim() { return isAttackAnim_; }

    //�����蔻�蒲���Z�b�g
    void SetColliderUpDown(DirectX::XMFLOAT2 upDown) {
        colliderUpDown_.x = upDown.x;
        colliderUpDown_.y = upDown.y;
    }

private:
    //�A�j���C�x���g�����瓖���蔻���t���邩���f("AutoCollision"����n�܂�C�x���g�������Ŏ擾)
    bool CollsionFromEventJudge();

private:
    //�U���̃q�b�g�m�F
    bool onHit_ = false;

    //�A�j���[�V�������x��ύX�����t���O
    bool isAnimSetting = false;

    //�U���A�j���[�V������true��
    bool isAttackAnim_ = false;
    int attackAnimIndex_ = -1;
    //�U���I���^�C�~���O�p
    bool oldIsAnim_ = false;
    int oldAnimIndex = -1;

    //����̓����蔻�蒲��({up,down})
    DirectX::XMFLOAT2 colliderUpDown_ = { 0,0 };

    //����̐e�ɂȂ�I�u�W�F�N�g
    std::shared_ptr<GameObject> parentObject_;
    //����̎q�ɂ��������m�[�h�̖��O
    std::string nodeName_;

    //�U�����̃X�e�[�^�X�iint:�A�j���[�V����Index�j
    std::map<int, AttackStatus> attackStatus_;
};


/*
    ------  �A�j���[�V���������蔻��ɂ��� ---------
�����蔻����A�j���[�V�����ɍ��킹�ĕt����ꍇ�́A
�A�j���[�V�����C�x���g�̖��O�̓���"AutoCollision"��t���āA
�t����t���[�����w�肷��B
node�͂ǂ��ł������i����̑傫���͂������Œ�`���Ă��邩��j
*/