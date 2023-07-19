#pragma once

#include "Components\System\Component.h"
#include "GameSource\Character\Character.h"

class PlayerCom : public Component/*, public Character*/
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
public:
    void OnDamage() {
        isDamage_ = true;
    }

    bool GetIsInvincible() {
        return isDamage_;
    }

private:
    //移動
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


    
    float jumpSpeed_ = 20.0f;

    bool isDash_ = false;       //ダッシュ中か
    int dashState_ = -1;        //ダッシュの遷移
    float dashMaxSpeed_ = 20;   //ダッシュの最大スピード
    float dashStopTime_ = 1;    //後ろダッシュを止めるため
    float dashStopTimer_;

    //ジャスト回避
    float justSpeed_ = 3.0f;    //回避前に少し移動（ジャスト回避適用ダッシュ）
    float justSpeedTime_ = 0.5f;    //ダッシュに変わる時間
    float justSpeedTimer_;
    bool isJustDash_ = false;   //ジャスト回避判定中のダッシュ中か

    //状態系
    bool isDamage_ = false; //ダメージを受けている時にtrue
    float damageInvincibleTime_ = 1; //ダメージ時の無敵時間
    float damageTimer_ = 0; //ダメージ時のタイマー


    DirectX::XMFLOAT3 up_ = {0,1,0};
};