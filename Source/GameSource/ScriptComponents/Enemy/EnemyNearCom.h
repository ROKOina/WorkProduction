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

private:
    //��e���ɃA�j���[�V�������鎞��AITREE�����߂�
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);

    //�A�j���[�V�����������ݒ�
    void AnimationInitialize()override;
};