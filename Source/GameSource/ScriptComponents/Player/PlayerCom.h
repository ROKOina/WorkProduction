#pragma once

#include "Components\System\Component.h"
#include "AttackPlayer.h"
#include "MovePlayer.h"
#include "JustAvoidPlayer.h"

class PlayerCom : public Component
{
    //コンポーネントオーバーライド
public:
    PlayerCom() {}
    ~PlayerCom() {}

    // 名前取得
    const char* GetName() const override { return "Player"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

    //PlayerComクラス


private:
    //アニメーション初期化設定
    void AnimationInitialize();

public:
    //今のプレイヤーの遷移状態
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

    //プレイヤーの攻撃の動きのみ管理する
    std::shared_ptr<AttackPlayer> attackPlayer_;
    std::shared_ptr<MovePlayer> movePlayer_;
    std::shared_ptr<JustAvoidPlayer> justAvoidPlayer_;
};