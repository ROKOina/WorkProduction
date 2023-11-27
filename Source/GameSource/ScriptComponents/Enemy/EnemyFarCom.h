#pragma once

#include "EnemyCom.h"

//���u�G
class EnemyFarCom :public EnemyCom
{
public:
    EnemyFarCom() {}
    ~EnemyFarCom() { }

    // ���O�擾
    virtual const char* GetName() const override { return "EnemyFar"; }

    // �J�n����
    virtual void Start()override;

    // �X�V����
    virtual void Update(float elapsedTime)override;

    // GUI�`��
    virtual void OnGUI()override;

    // ���b�Z�[�W��M�����Ƃ��̏���
    virtual bool OnMessage(const Telegram& msg)override;

public:
    //��������
    float GetBackMoveRange() { return backMoveRange_; }
    void SetBackMoveRange(float range) { backMoveRange_ = range; }

private:
    //Near�G�ł�protected�Ŏg���邪���̂�����͎g���Ȃ��H�H�H�H�H�H
    //��e���ɃA�j���[�V�������鎞��AITREE�����߂�
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);

protected:
    //�A�j���[�V�����������ݒ�
    virtual void AnimationInitialize()override;

protected:
    //��������
    float backMoveRange_ = 5.0f;
};