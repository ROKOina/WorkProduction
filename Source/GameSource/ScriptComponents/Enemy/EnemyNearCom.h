#pragma once

#include "EnemyCom.h"

//近接敵
class EnemyNearCom :public EnemyCom
{
public:
    EnemyNearCom() {}
    ~EnemyNearCom() {  }

    // 名前取得
    virtual const char* GetName() const override { return "EnemyNear"; }

    // 開始処理
    virtual void Start()override;

    // 更新処理
    virtual void Update(float elapsedTime)override;

    // GUI描画
    virtual void OnGUI()override;

    // メッセージ受信したときの処理
    virtual bool OnMessage(const Telegram& msg)override;

public:
    //接近フラグ取得
    bool GetIsNearFlag() { return isNearFlag_; }
    void SetIsNearFlag(bool flag) { isNearFlag_ = flag; }
    //経路探索フラグ
    bool GetIsPathFlag() { return isPathFlag_; }
    void SetIsPathFlag(bool flag) { isPathFlag_ = flag; }

protected:
    //被弾時にアニメーションする時のAITREEを決める
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);

    //アニメーション初期化設定
    virtual void AnimationInitialize()override;

    //接近フラグ管理
    void NearFlagProcess();

protected:
    //接近フラグ
    bool isNearFlag_ = false;

    //経路探索フラグ
    bool isPathFlag_ = false;
};