#pragma once

#include <DirectXMath.h>

#include "System\Component.h"

//�J����
class CameraCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    CameraCom() {}
    ~CameraCom() {}

    // ���O�擾
    const char* GetName() const override { return "Camera"; }

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //Camera�N���X
public:
    //�w�����������
    void SetLookAt(const DirectX::XMFLOAT3& focus,const DirectX::XMFLOAT3& up = { 0,1,0 });

    //�p�[�X�y�N�e�B�u�ݒ�
    void SetPerspectiveFov(float fovY, float aspect, float nearZ, float farZ);

    //�r���[�s��擾
    const DirectX::XMFLOAT4X4& GetView() const { return view; }

    //�v���W�F�N�V�����s��擾
    const DirectX::XMFLOAT4X4& GetProjection() const { return projection; }

    //�����_�擾
    const DirectX::XMFLOAT3& GetFocus()const { return focus; }

    //������擾
    const DirectX::XMFLOAT3& GetUp()const { return up; }

    //�O�����擾
    const DirectX::XMFLOAT3& GetFront()const { return front; }

    //�E�����擾
    const DirectX::XMFLOAT3& GetRight()const { return right; }

private:
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;

    DirectX::XMFLOAT3 focus;

    DirectX::XMFLOAT3 up;
    DirectX::XMFLOAT3 front;
    DirectX::XMFLOAT3 right;
};