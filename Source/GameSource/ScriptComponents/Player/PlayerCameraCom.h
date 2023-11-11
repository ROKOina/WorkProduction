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

public:
    void SetAngleY(float angle) { angleY_ = angle; }
    float GetAngleY() { return angleY_; }
    void SetAngleX(float angle) { angleX_ = angle; }
    float GetAngleX() { return angleX_; }

    void SetRange(float range) { range_ = range; }
    float GetRange() { return range_; }

    void SetForcusPos(DirectX::XMFLOAT3 focus) { lerpFocusPos_ = focus; }
    DirectX::XMFLOAT3 GetForcusPos() { return lerpFocusPos_; }

    bool isJust = false;
    DirectX::XMFLOAT3 pos;

private:
    float angleX_ = 0, angleY_ = 0;
    float range_ = 4;   //�J��������
    float angleLimit_ = 55;   //�J�����p�x����
    DirectX::XMFLOAT3 oldCameraPos_;

    DirectX::XMFLOAT3 lerpFocusPos_;
    float lerpSpeed_ = 1;
    DirectX::XMFLOAT3 Eye_;
};