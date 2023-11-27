#pragma once

#include "EnemyCom.h"

//遠隔敵
class EnemyFarCom :public EnemyCom
{
public:
    EnemyFarCom() {}
    ~EnemyFarCom() { }

    // 名前取得
    virtual const char* GetName() const override { return "EnemyFar"; }

    // 開始処理
    virtual void Start()override;

    // 更新処理
    virtual void Update(float elapsedTime)override;

    // GUI描画
    virtual void OnGUI()override;

    // メッセージ受信したときの処理
    virtual bool OnMessage(const Telegram& msg)override;

public:
    //逃走距離
    float GetBackMoveRange() { return backMoveRange_; }
    void SetBackMoveRange(float range) { backMoveRange_ = range; }

private:
    //Near敵ではprotectedで使えるが何故かこれは使えない？？？？？？
    //被弾時にアニメーションする時のAITREEを決める
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);

protected:
    //アニメーション初期化設定
    virtual void AnimationInitialize()override;

protected:
    //逃走距離
    float backMoveRange_ = 5.0f;
};