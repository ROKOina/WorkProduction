#pragma once

#include <memory>

//前方宣言
class PlayerCom;
class GameObject;

//プレイヤーのジャスト回避を管理
class JustAvoidPlayer
{
public:
    JustAvoidPlayer(std::shared_ptr<PlayerCom> player) : player_(player) {}
    ~JustAvoidPlayer() {}

    void Update(float elapsedTime);

    void OnGui();

private:
    //ジャスト回避初期化
    void JustInisialize();
    //ジャスト回避中移動
    void JustAvoidanceMove(float elapsedTime);
    //ジャスト回避反撃の入力を見る
    void JustAvoidanceAttackInput();

    //□反撃
    void JustAvoidanceSquare(float elapsedTime);

public:
    //ジャスト回避出来たか判定
    void JustAvoidJudge();

    //ジャスト回避を開始する
    void StartJustAvoid()
    {
        isJustJudge_ = true;
        justAvoidState_ = 0;
    }

    //ゲッター＆セッター
    const std::weak_ptr<GameObject> GetJustHitEnemy() const { return justHitEnemy_; }

    const bool& GetIsJustJudge() const { return isJustJudge_; }


private:
    //ジャスト回避判定
    bool isJustJudge_ = false;  //ジャスト回避判定
    int justAvoidState_ = -1;   //ジャスト回避の遷移
    float justAvoidTime_ = 1;   //ジャスト回避時間
    float justAvoidTimer_ = 0;
    std::weak_ptr<GameObject> justHitEnemy_;   //ジャスト回避時の敵保存
    //std::shared_ptr<GameObject> justHitEnemy_;   //ジャスト回避時の敵保存

    //ジャスト回避反撃
    enum class JUST_AVOID_KEY   //入力を判定
    {
        SQUARE,     //□
        TRIANGLE,   //△

        NULL_KEY,
    };
    JUST_AVOID_KEY justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;

private:
    std::weak_ptr<PlayerCom> player_;
};