#pragma once

#include <memory>

//�O���錾
class PlayerCom;
class GameObject;

//�v���C���[�̃W���X�g������Ǘ�
class JustAvoidPlayer
{
public:
    JustAvoidPlayer(std::shared_ptr<PlayerCom> player) : player_(player) {}
    ~JustAvoidPlayer() {}

    void Update(float elapsedTime);

    void OnGui();

private:
    //�W���X�g���������
    void JustInisialize();
    //�W���X�g��𒆈ړ�
    void JustAvoidanceMove(float elapsedTime);
    //�W���X�g��𔽌��̓��͂�����
    void JustAvoidanceAttackInput();

    //������
    void JustAvoidanceSquare(float elapsedTime);

public:
    //�W���X�g��𔻒�
    bool isJustJudge_ = false;  //�W���X�g��𔻒�
    int justAvoidState_ = -1;   //�W���X�g����̑J��
    float justAvoidTime_ = 1;   //�W���X�g�������
    float justAvoidTimer_ = 0;
    std::shared_ptr<GameObject> justHitEnemy_;   //�W���X�g������̓G�ۑ�

    //�W���X�g��𔽌�
    enum class JUST_AVOID_KEY   //���͂𔻒�
    {
        SQUARE,     //��
        TRIANGLE,   //��

        NULL_KEY,
    };
    JUST_AVOID_KEY justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;

private:
    std::weak_ptr<PlayerCom> player_;
};