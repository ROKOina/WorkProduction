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

    void AddForce(const DirectX::XMFLOAT3& force) {
        velocity_.x += force.x;
        velocity_.y += force.y;
        velocity_.z += force.z;
    };

    //PlayerCom�N���X
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
    //�c�����ړ��X�V
    void VerticalUpdate(float elapsedTime);
    //�������ړ��X�V
    void HorizonUpdate(float elapsedTime);

    //�_�b�V��
    void DashMove(float elapsedTime);

    //���͂��X�V
    void VelocityAppPosition(float elapsedTime);


    //���͒l�ۑ�
    DirectX::XMFLOAT3 inputMoveVec_;

    //�ړ��p�����[�^�[
    DirectX::XMFLOAT3 velocity_ = { 0,0,0 };

    //����ƕ�����؂�ւ���
    enum MOVE_PARAM
    {
        WALK,
        RUN,
        MAX,
    };
    struct
    {
        float moveMaxSpeed = 10.0f;
        float moveAcceleration = 0.2f;
        float turnSpeed = 8.0f;
    }moveParam_[MOVE_PARAM::MAX];
    int moveParamType = MOVE_PARAM::WALK;

    //�_�b�V���p�̑���
    DirectX::XMFLOAT3 dashVelocity_ = { 0,0,0 };
    //�_�b�V�����Ƀv���X�ő����Ȃ�
    float dashSpeed_ = 10.0f;
    bool isDash_ = false;

    float jumpSpeed_ = 20.0f;
    float gravity_   = -0.25f;  //�d��
    float friction_ = 0.1f;  //���C

    DirectX::XMFLOAT3 up_ = {0,1,0};
};