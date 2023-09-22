#pragma once

#include <memory>
#include <DirectXMath.h>

//前方宣言
class PlayerCom;
class GameObject;

//プレイヤーの移動を管理
class MovePlayer
{
public:
    MovePlayer(std::shared_ptr<PlayerCom> player);
    ~MovePlayer(){}

    void Update(float elapsedTime);

    void OnGui();

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
    void DashStateUpdate(float elapsedTime);

    //走り切り替えし
    void RunTurnMove();
    void RunTurnJudge();
    void EndRunTurn();

public:
    //強制的にダッシュを終わらせる（攻撃時等）
    //引数は歩きにするか
    void DashEndFlag(bool isWalk = true);

public:
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

    //走り切り替えし
    bool isRunTurn_ = false;
    int runTurnState_ = -1;
    DirectX::XMFLOAT3 saveTurnVec_;

    //走りからストップモーションフラグ
    bool isStopRunStop_ = false;

    float jumpSpeed_ = 20.0f;
    int jumpCount_ = 2;

    bool isDash_ = true;        //ダッシュできるか
    bool isDashJudge_ = false;  //ダッシュ中か
    int dashState_ = -1;        //ダッシュの遷移
    float dashMaxSpeed_ = 15;   //ダッシュの最大スピード
    float dashStopTime_ = 1;    //ダッシュ無理やり止めるため
    float dashStopTimer_;
    float dashCoolTime_ = 0.6f;       //ダッシュのクールタイム
    float dashCoolTimer_;

private:
    std::weak_ptr<PlayerCom> player_;
};