#pragma once

#include "Components\System\Component.h"
#include "AttackPlayer.h"
#include "MovePlayer.h"
#include "JustAvoidPlayer.h"

class PlayerCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    PlayerCom() {}
    ~PlayerCom() {}

    // ���O�擾
    const char* GetName() const override { return "Player"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //PlayerCom�N���X

//#pragma region �W���X�g���
//private:
//    //�W���X�g���������
//    void JustInisialize();
//    //�W���X�g��𔽌��X�V����
//    void JustAvoidanceAttackUpdate(float elapsedTime);
//    //�W���X�g��𒆈ړ�
//    void JustAvoidanceMove(float elapsedTime);
//    //�W���X�g��𔽌��̓��͂�����
//    void JustAvoidanceAttackInput();
//
//    //������
//    void JustAvoidanceSquare(float elapsedTime);
//
//public:
//    //�W���X�g��𔻒�
//    bool isJustJudge_ = false;  //�W���X�g��𔻒�
//    int justAvoidState_ = -1;   //�W���X�g����̑J��
//    float justAvoidTime_ = 1;   //�W���X�g�������
//    float justAvoidTimer_ = 0;
//    std::shared_ptr<GameObject> justHitEnemy_;   //�W���X�g������̓G�ۑ�
//
//    //�W���X�g��𔽌�
//    enum class JUST_AVOID_KEY   //���͂𔻒�
//    {
//        SQUARE,     //��
//        TRIANGLE,   //��
//
//        NULL_KEY,
//    };
//    JUST_AVOID_KEY justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
//
//
//#pragma endregion


private:
    //�A�j���[�V�����������ݒ�
    void AnimationInitialize();

public:
    //���̃v���C���[�̑J�ڏ��
    enum class PLAYER_STATUS
    {
        IDLE,
        MOVE,
        DASH,
        BACK_DASH,
        JUMP,
        JUST,
        JUMP_DASH,
        JUMP_BACK_DASH,
        JUMP_JUST,

        ATTACK,
        ATTACK_DASH,
    };
    PLAYER_STATUS GetPlayerStatus() { return playerStatus_; }
    void SetPlayerStatus(PLAYER_STATUS status) { playerStatus_ = status; }

    std::shared_ptr<MovePlayer> GetMovePlayer() { return movePlayer_; }
    std::shared_ptr<AttackPlayer> GetAttackPlayer() { return attackPlayer_; }
    std::shared_ptr<JustAvoidPlayer> GetJustAvoidPlayer() { return justAvoidPlayer_; }

private:
    PLAYER_STATUS playerStatus_ = PLAYER_STATUS::IDLE;

    //�v���C���[�̍U���̓����̂݊Ǘ�����
    std::shared_ptr<AttackPlayer> attackPlayer_;
    std::shared_ptr<MovePlayer> movePlayer_;
    std::shared_ptr<JustAvoidPlayer> justAvoidPlayer_;
};