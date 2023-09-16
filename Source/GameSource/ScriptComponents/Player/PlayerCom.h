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

//#pragma region ジャスト回避
//private:
//    //ジャスト回避初期化
//    void JustInisialize();
//    //ジャスト回避反撃更新処理
//    void JustAvoidanceAttackUpdate(float elapsedTime);
//    //ジャスト回避中移動
//    void JustAvoidanceMove(float elapsedTime);
//    //ジャスト回避反撃の入力を見る
//    void JustAvoidanceAttackInput();
//
//    //□反撃
//    void JustAvoidanceSquare(float elapsedTime);
//
//public:
//    //ジャスト回避判定
//    bool isJustJudge_ = false;  //ジャスト回避判定
//    int justAvoidState_ = -1;   //ジャスト回避の遷移
//    float justAvoidTime_ = 1;   //ジャスト回避時間
//    float justAvoidTimer_ = 0;
//    std::shared_ptr<GameObject> justHitEnemy_;   //ジャスト回避時の敵保存
//
//    //ジャスト回避反撃
//    enum class JUST_AVOID_KEY   //入力を判定
//    {
//        SQUARE,     //□
//        TRIANGLE,   //△
//
//        NULL_KEY,
//    };
//    JUST_AVOID_KEY justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
//
//
//#pragma endregion


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