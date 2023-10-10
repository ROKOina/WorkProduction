#pragma once

#include "EnemyCom.h"

//���u�G
class EnemyFarCom :public EnemyCom
{
public:
    EnemyFarCom() {}
    ~EnemyFarCom() { activeNode_.release(); }

    // ���O�擾
    const char* GetName() const override { return "EnemyFar"; }

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