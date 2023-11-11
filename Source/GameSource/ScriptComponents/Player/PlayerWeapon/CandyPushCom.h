#pragma once

#include "Components\System\Component.h"

class PushWeaponCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    PushWeaponCom() {}
    ~PushWeaponCom() {}

    // ���O�擾
    const char* GetName() const override { return "PushWeapon"; }

    // �J�n����
    void Start() override {};

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

public:
    void MoveStart(DirectX::XMFLOAT3 movePos,DirectX::XMFLOAT3 startPos);
    bool IsMove() { return isMove_; }

private:
    bool isMove_ = false;
    DirectX::XMFLOAT3 startPos_;
    DirectX::XMFLOAT3 movePos_;
    float speed_ = 60;

    DirectX::XMFLOAT3 saveEuler_;
};