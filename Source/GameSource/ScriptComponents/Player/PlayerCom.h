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