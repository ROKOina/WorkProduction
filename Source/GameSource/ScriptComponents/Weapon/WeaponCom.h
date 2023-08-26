#pragma once

#include "Components\System\Component.h"

#include "SystemStruct\QuaternionStruct.h"
#include <map>

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
    };
    //�A�j���[�V�����ƃX�e�[�^�X��R�Â���
    void SetAttackStatus(int animIndex, int damage, float impactPower, float front = 1, float up = 0);

    //�q�b�g�m�F
    bool GetOnHit() { return onHit_; }

private:
    //�A�j���C�x���g�����瓖���蔻���t���邩���f("AutoCollision"����n�܂�C�x���g�������Ŏ擾)
    bool CollsionFromEventJudge();

private:
    //�U���̃q�b�g�m�F
    bool onHit_ = false;

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