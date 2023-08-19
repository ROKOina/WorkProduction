#pragma once

#include "Components\System\Component.h"
#include "AttackPlayer.h"

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

#pragma region 移動
private:
    //スティック入力値から移動ベクトルを取得
    DirectX::XMFLOAT3 GetMoveVec();

    //移動入力処理
    bool IsMove(float elapsedTime);
    //回転
    void Trun(float elapsedTime);
    //縦方向移動
    void VerticalMove();
    //横方向移動
    void HorizonMove();

    //ダッシュ
    void DashMove(float elapsedTime);
    //ダッシュ時の更新
    void DashStateUpdate(float elapsedTime, std::shared_ptr<GameObject> enemy);
    //強制的にダッシュを終わらせる（攻撃時等）
    void DashEndFlag();

private:
    //移動系
    //入力値保存
    DirectX::XMFLOAT3 inputMoveVec_;

    //走りと歩きを切り替える
    enum MOVE_PARAM
    {
        WALK,
        RUN,
        JUSTDASH,
        DASH,
        MAX,
    };
    struct
    {
        float moveMaxSpeed = 10.0f;
        float moveSpeed = 1.0f;
        float moveAcceleration = 0.2f;
        float turnSpeed = 8.0f;
    }moveParam_[MOVE_PARAM::MAX];
    int moveParamType_ = MOVE_PARAM::WALK;

    //入力で移動できるか
    bool isInputMove_ = true;
    //回転できるか
    bool isInputTrun_ = true;

    float jumpSpeed_ = 20.0f;
    int jumpCount_ = 2;

    bool isDash_ = true;        //ダッシュできるか
    bool isDashJudge_ = false;  //ダッシュ中か
    int dashState_ = -1;        //ダッシュの遷移
    float dashMaxSpeed_ = 20;   //ダッシュの最大スピード
    float dashStopTime_ = 1;    //ダッシュ無理やり止めるため
    float dashStopTimer_;

#pragma endregion

#pragma region ジャスト回避
private:
    //ジャスト回避初期化
    void JustInisialize();
    //ジャスト回避反撃更新処理
    void JustAvoidanceAttackUpdate(float elapsedTime);
    //ジャスト回避中移動
    void JustAvoidanceMove(float elapsedTime);
    //ジャスト回避反撃の入力を見る
    void JustAvoidanceAttackInput();

    //□反撃
    void JustAvoidanceSquare(float elapsedTime);

private:
    //ジャスト回避判定
    bool isJustJudge_ = false;  //ジャスト回避判定
    int justAvoidState_ = -1;   //ジャスト回避の遷移
    float justAvoidTime_ = 1;   //ジャスト回避時間
    float justAvoidTimer_ = 0;
    std::shared_ptr<GameObject> justHitEnemy_;   //ジャスト回避時の敵保存

    //ジャスト回避反撃
    enum class JUST_AVOID_KEY   //入力を判定
    {
        SQUARE,     //□
        TRIANGLE,   //△

        NULL_KEY,
    };
    JUST_AVOID_KEY justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;

    //ジャスト回避後の攻撃を攻撃に引き継ぐ用
    std::string justAnimFlagName_ = "";

#pragma endregion

#pragma region 攻撃
private:
    //攻撃更新
    void AttackUpdate();

    //攻撃当たり判定
    void AttackJudgeCollision();

    //強制的に攻撃を終わらせる（ジャンプ時等）
    void AttackFlagEnd();

private:
    bool isNormalAttack_ = true;     //攻撃できるか
    int comboAttackCount_ = 0;

#pragma endregion

private:
    //アニメーション初期化設定
    void AnimationInitialize();

private:

    DirectX::XMFLOAT3 up_ = {0,1,0};

    //プレイヤーの攻撃の動きのみ管理する
    std::shared_ptr<AttackPlayer> attackPlayer_;
};