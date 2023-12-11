#pragma once

#include <memory>
#include <DirectXMath.h>

#include "Audio\AudioSource.h"
#include "Audio\Audio.h"

//前方宣言
class PlayerCom;
class GameObject;
class Sprite;

//プレイヤーのジャスト回避を管理
class JustAvoidPlayer
{
public:
    JustAvoidPlayer(std::shared_ptr<PlayerCom> player);
    ~JustAvoidPlayer();

    void Update(float elapsedTime);

    void OnGui();

    void Render2D(float elapsedTime);

    //音解放
    void AudioRelease();

private:
    //ジャスト回避初期化
    void JustInisialize();
    //ジャスト回避中移動
    void JustAvoidanceMove(float elapsedTime);
    //ジャスト回避反撃の入力を見る
    void JustAvoidanceAttackInput();

    //□反撃
    void JustAvoidanceSquare(float elapsedTime);

    //△反撃
    void JustAvoidanceTriangle(float elapsedTime);

    //ジャスト回避世界色演出
    void JustDirectionUpdate(float elapsedTime);

    //ジャスト回避パーティクルセット
    void SetJustUnderParticle(bool flag);

public:
    //ジャスト回避出来たか判定
    void JustAvoidJudge();

    //ジャスト回避を開始する
    void StartJustAvoid();

    //ゲッター＆セッター
    const std::weak_ptr<GameObject> GetJustHitEnemy() const { return justHitEnemy_; }

    const bool& GetIsJustJudge() const { return isJustJudge_; }

    //ジャスト回避中演出用
    void JustAvoidDirectionEnd(float elapsedTime);

    //ジャスト回避演出描画
    void justDirectionRender2D();

    //ジャスト回避反撃
    enum class JUST_AVOID_KEY   //入力を判定
    {
        SQUARE,     //□
        TRIANGLE,   //△

        NULL_KEY,
    };
    JUST_AVOID_KEY GetJustAvoidKey() { return justAvoidKey_; }

private:
    //ジャスト回避判定
    bool isJustJudge_ = false;  //ジャスト回避判定
    int justAvoidState_ = -1;   //ジャスト回避の遷移
    float justAvoidTime_ = 1;   //ジャスト回避時間
    float justAvoidTimer_ = 0;
    bool onGroundAvoid_ = true; //地上のジャスト回避か
    std::weak_ptr<GameObject> justHitEnemy_;   //ジャスト回避時の敵保存

    bool hitStopEnd_ = false;  //ヒットストップ演出判定
    bool playerDirection_ = false;  //プレイヤー演出

    JUST_AVOID_KEY justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
    JUST_AVOID_KEY justAvoidLeadKey_ = JUST_AVOID_KEY::NULL_KEY;    //先行入力用
    int triangleState_ = 0;
    std::weak_ptr<GameObject> lockTriangleEnemy_;

    //ジャスト回避演出用
    bool isJustSprite_ = false;
    int justSpriteState_ = -1;
    std::unique_ptr<Sprite> justSprite_;

    //SE
    std::unique_ptr<AudioSource> justSE_ = Audio::Instance().LoadAudioSource("Data/Audio/Player/just.wav");
    std::unique_ptr<AudioSource> dashSE_ = Audio::Instance().LoadAudioSource("Data/Audio/Player/dash.wav");
    std::unique_ptr<AudioSource> triangleJustAttackSE_ = Audio::Instance().LoadAudioSource("Data/Audio/Player/triangleJustAttack.wav");
    std::unique_ptr<AudioSource> triangleCursorSE_ = Audio::Instance().LoadAudioSource("Data/Audio/Player/triangleCursor.wav");
    std::unique_ptr<AudioSource> squareSlowSE_ = Audio::Instance().LoadAudioSource("Data/Audio/Player/squareSlow.wav");
    std::unique_ptr<AudioSource> squareSlowInOutSE_ = Audio::Instance().LoadAudioSource("Data/Audio/Player/squareSlowInOut.wav");

private:
    std::weak_ptr<PlayerCom> player_;
};