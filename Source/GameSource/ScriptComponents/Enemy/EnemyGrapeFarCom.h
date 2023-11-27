#pragma once

#include "EnemyFarCom.h"

//�ߐړG
class EnemyGrapeFarCom :public EnemyFarCom
{
public:
    EnemyGrapeFarCom() {}
    ~EnemyGrapeFarCom() {  }

    // ���O�擾
    const char* GetName() const override { return "EnemyGrapeFar"; }

    // �J�n����
    void Start()override;

    // �X�V����
    void Update(float elapsedTime)override;

    // GUI�`��
    void OnGUI()override;

private:
    //�A�j���[�V�����������ݒ�
    void AnimationInitialize()override;

    //��e���ɃA�j���[�V�������鎞��AITREE�����߂�
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);


    //�e��������
    void SpawnGrapeBall();

private:
    //��񂾂��e�o�������邽��
    bool isSpawnBall_ = false;
};

//�e�p
class GrapeBallCom :public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    GrapeBallCom() {}
    ~GrapeBallCom() {}

    // ���O�擾
    const char* GetName() const override { return "GrapeBall"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

public:

private:
    float speed_ = 4.5f;
    float rotaAsistPower_ = 1.0f;   //�v���C���[�ւ̒e�̋z���t���p���[

    bool endFlag_ = false;
    float removeTimer = 5.0f;
};