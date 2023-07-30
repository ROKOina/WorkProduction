#pragma once

#include "Components\System\Component.h"

class EnemyCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    EnemyCom() {}
    ~EnemyCom() {}

    // ���O�擾
    const char* GetName() const override { return "Enemy"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //EnemyCom�N���X
private:
    //�A�j���[�V�����������ݒ�
    void AnimationInitialize();




};
