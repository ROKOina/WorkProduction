#pragma once

#include "EnemyCom.h"

//�ߐړG
class EnemyNearCom :public EnemyCom
{
public:
    EnemyNearCom() {}
    ~EnemyNearCom() { activeNode_.release(); }

    // ���O�擾
    const char* GetName() const override { return "EnemyNear"; }

    // �J�n����
    void Start()override;

    // �X�V����
    void Update(float elapsedTime)override;

    // GUI�`��
    void OnGUI()override;

    // ���b�Z�[�W��M�����Ƃ��̏���
    bool OnMessage(const Telegram& msg)override;

public:
    //�ڋ߃t���O�擾
    bool GetIsNearFlag() { return isNearFlag_; }

private:
    //��e���ɃA�j���[�V�������鎞��AITREE�����߂�
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);

    //�A�j���[�V�����������ݒ�
    void AnimationInitialize()override;

    //�ڋ߃t���O�Ǘ�
    void NearFlagProcess();

private:
    //�ڋ߃t���O
    bool isNearFlag_ = false;
};