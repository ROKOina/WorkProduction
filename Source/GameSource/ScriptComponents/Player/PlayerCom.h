#pragma once

#include "Components\System\Component.h"
#include "GameSource\Character\Character.h"

class PlayerCom : public Component/*, public Character*/
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    PlayerCom() {}
    ~PlayerCom() {}

    // ���O�擾
    const char* GetName() const override { return "Player"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //PlayerCom�N���X
public:
    void OnDamage() {
        isDamage_ = true;
    }

    bool GetIsInvincible() {
        return isDamage_;
    }

private:
    //�ړ�
    //�X�e�B�b�N���͒l����ړ��x�N�g�����擾
    DirectX::XMFLOAT3 GetMoveVec();

    //�ړ����͏���
    bool IsMove(float elapsedTime);
    //��]
    void Trun(float elapsedTime);
    //�c�����ړ�
    void VerticalMove();
    //�������ړ�
    void HorizonMove();

    //�_�b�V��
    void DashMove(float elapsedTime);

private:

    //�ړ��n
    //���͒l�ۑ�
    DirectX::XMFLOAT3 inputMoveVec_;

    //����ƕ�����؂�ւ���
    enum MOVE_PARAM
    {
        WALK,
        RUN,
        DASH,
        MAX,
    };
    struct
    {
        float moveMaxSpeed = 10.0f;
        float moveAcceleration = 0.2f;
        float turnSpeed = 8.0f;
    }moveParam_[MOVE_PARAM::MAX];
    int moveParamType_ = MOVE_PARAM::WALK;

    //�_�b�V�����Ƀv���X�ő����Ȃ�
    float dashSpeed_ = 10.0f;
    bool isDash_ = false;
    float jumpSpeed_ = 20.0f;


    //��Ԍn
    bool isDamage_ = false; //�_���[�W���󂯂Ă��鎞��true
    float damageInvincibleTime_ = 1; //�_���[�W���̖��G����
    float damageTimer_ = 0; //�_���[�W���̃^�C�}�[


    DirectX::XMFLOAT3 up_ = {0,1,0};
};