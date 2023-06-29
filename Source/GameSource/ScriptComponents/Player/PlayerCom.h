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
    bool Move(float elapsedTime);

    //���͒l�ۑ�
    DirectX::XMFLOAT3 inputMoveVec_;

    //�ړ��p�����[�^�[
    float moveSpeed_ = 5.0f;
    float turnSpeed_ = DirectX::XMConvertToRadians(720);
    float jumpSpeed_ = 20.0f;


    DirectX::XMFLOAT3 up_ = {0,1,0};
    DirectX::XMFLOAT3 look_;

};