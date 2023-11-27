#pragma once

#include "EnemyCom.h"

//�ߐړG
class EnemyNearCom :public EnemyCom
{
public:
    EnemyNearCom() {}
    ~EnemyNearCom() {  }

    // ���O�擾
    virtual const char* GetName() const override { return "EnemyNear"; }

    // �J�n����
    virtual void Start()override;

    // �X�V����
    virtual void Update(float elapsedTime)override;

    // GUI�`��
    virtual void OnGUI()override;

    // ���b�Z�[�W��M�����Ƃ��̏���
    virtual bool OnMessage(const Telegram& msg)override;

public:
    //�ڋ߃t���O�擾
    bool GetIsNearFlag() { return isNearFlag_; }
    void SetIsNearFlag(bool flag) { isNearFlag_ = flag; }
    //�o�H�T���t���O
    bool GetIsPathFlag() { return isPathFlag_; }
    void SetIsPathFlag(bool flag) { isPathFlag_ = flag; }

protected:
    //��e���ɃA�j���[�V�������鎞��AITREE�����߂�
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);

    //�A�j���[�V�����������ݒ�
    virtual void AnimationInitialize()override;

    //�ڋ߃t���O�Ǘ�
    void NearFlagProcess();

protected:
    //�ڋ߃t���O
    bool isNearFlag_ = false;

    //�o�H�T���t���O
    bool isPathFlag_ = false;
};