#pragma once

#include "Components\System\Component.h"

class PlayerCameraCom :public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    PlayerCameraCom() {}
    ~PlayerCameraCom() {}

    // ���O�擾
    const char* GetName() const override { return "PlayerCamera"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

private:
    float angleX_ = 0, angleY_ = 0;
    float range_ = 4;   //�J��������
    DirectX::XMFLOAT3 oldCameraPos_;
    DirectX::XMFLOAT3 Eye_;
};