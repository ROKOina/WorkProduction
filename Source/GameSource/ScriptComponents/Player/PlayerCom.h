#pragma once

#include "Components\System\Component.h"
#include "AttackPlayer.h"
#include "MovePlayer.h"
#include "JustAvoidPlayer.h"

//�A�j���[�V�������X�g
enum ANIMATION_PLAYER
{
    WALK_RUNRUN_1,
    IDEL_1,
    IDEL_2,
    JUMP_1,
    JUMP_2,
    RUN_HARD_1,
    RUN_HARD_2,
    RUN_SOFT_1,
    RUN_SOFT_2,
    WALK_RUNRUN_2,
    PUNCH,
    BIGSWORD_UP,
    BIGSWORD_LEFT,
    BIGSWORD_RIGHT,
    BIGSWORD_DOWN,
    DASH_ANIM,
    BIGSWORD_COM1_01,
    BIGSWORD_COM1_02,
    BIGSWORD_COM1_03,
    BIGSWORD_COM2_01,
    BIGSWORD_COM2_02,
    BIGSWORD_COM2_03,
    BIGSWORD_DASH,
    JUMP_IN,
    DODGE_BACK,
    DODGE_FRONT,
    DODGE_LEFT,
    DODGE_RIGHT,
    DASH_BACK,
    RUN_TURN,
    RUN_STOP,
    RUN_TURN_FORWARD,
};

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