#pragma once

#include "EnemyNearCom.h"

//�ߐړG
class EnemyAppleNearCom :public EnemyNearCom
{
public:
    EnemyAppleNearCom() {}
    ~EnemyAppleNearCom() {  }

    // ���O�擾
    const char* GetName() const override { return "EnemyAppleNear"; }

    // �J�n����
    void Start()override;

    // �X�V����
    void Update(float elapsedTime)override;

    // GUI�`��
    void OnGUI()override;

    // ���b�Z�[�W��M�����Ƃ��̏���
    bool OnMessage(const Telegram& msg)override;

private:
    //�A�j���[�V�����������ݒ�
    void AnimationInitialize()override;

private:
};