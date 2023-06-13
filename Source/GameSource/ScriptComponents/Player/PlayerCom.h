#pragma once

#include "Components\System\Component.h"
#include "GameSource\Character\Character.h"

class PlayerCom : public Component, public Character
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
private:
    //�ړ�
    //�X�e�B�b�N���͒l����ړ��x�N�g�����擾
    DirectX::XMFLOAT3 GetMoveVec();

    //�ړ����͏���
    bool InputMove(float elapsedTime);

    //�ړ��p�����[�^�[
    float moveSpeed_ = 5.0f;
    float turnSpeed_ = DirectX::XMConvertToRadians(720);
    float jumpSpeed_ = 20.0f;

    DirectX::XMFLOAT3 up_;
    DirectX::XMFLOAT3 look_;

};