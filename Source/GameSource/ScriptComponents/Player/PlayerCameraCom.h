#pragma once

#include "Components\System\Component.h"

#define DefaultRange 4.0f
#define NearRange 2.0f
#define FarRange 7.0f

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
    void SetChangeRange(float time, float range) {
        rangeTimer_ = time;
        rangeDir_ = range;
    }

    void SetForcusPos(DirectX::XMFLOAT3 focus) { lerpFocusPos_ = focus; }
    DirectX::XMFLOAT3 GetForcusPos() { return lerpFocusPos_; }

    void SetIsJust(bool flag) { isJust = flag; }
    void SetJustPos(DirectX::XMFLOAT3 pos) { justPos_ = pos; }

private:
    float angleX_ = 0, angleY_ = 0;
    float range_ = 4;   //�J��������
    float rangeTimer_;  //�J�����̋�����ς��鎞��
    float rangeDir_;    //�J�����̋������w��
    float angleLimit_ = 22;   //�J�����p�x����
    DirectX::XMFLOAT3 oldCameraPos_;

    DirectX::XMFLOAT3 lerpFocusPos_;
    float lerpSpeed_ = 1;
    DirectX::XMFLOAT3 Eye_;

    //�W���X�g��𒆉��o�悤
    bool isJust = false;
    DirectX::XMFLOAT3 justPos_;
};