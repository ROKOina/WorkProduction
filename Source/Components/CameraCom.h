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

    // �J�n����
    void Start() override;

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
    const DirectX::XMFLOAT4X4& GetView() const { return view_; }

    //�v���W�F�N�V�����s��擾
    const DirectX::XMFLOAT4X4& GetProjection() const { return projection_; }

    //�����_�擾
    const DirectX::XMFLOAT3& GetFocus()const { return focus_; }

    //������擾
    const DirectX::XMFLOAT3& GetUp()const { return up_; }

    //�O�����擾
    const DirectX::XMFLOAT3& GetFront()const { return front_; }

    //�E�����擾
    const DirectX::XMFLOAT3& GetRight()const { return right_; }

    //�J�����V�F�C�N���s
    void CameraShake(float power, float seconds) { 
        shakePower_ = power; 
        shakeSec_ = seconds;
    }

    //�q�b�g�X�g�b�v
    void HitStop(float sec);
    bool GetIsHitStop() { return isHitStop_; }


private:
    //���W�n
    DirectX::XMFLOAT4X4 view_;
    DirectX::XMFLOAT4X4 projection_;

    DirectX::XMFLOAT3 focus_ = {0,0,0};

    DirectX::XMFLOAT3 up_ = { 0,1,0 };
    DirectX::XMFLOAT3 front_ = { 0,0,1 };
    DirectX::XMFLOAT3 right_ = { 1,0,0 };

    bool isLookAt_ = false;


    //���o�n
    //�J�����V�F�C�N
    float shakeSec_;    //�b��
    float shakePower_;  //����
    DirectX::XMFLOAT3 shakePos_;

    //�q�b�g�X�g�b�v
    float hitTimer_ = 0;
    float saveWorldSpeed_ = 0;
    bool isHitStop_ = false;
};