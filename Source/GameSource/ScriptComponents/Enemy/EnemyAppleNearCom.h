#pragma once

#include "EnemyNearCom.h"

//近接敵
class EnemyAppleNearCom :public EnemyNearCom
{
public:
    EnemyAppleNearCom() {}
    ~EnemyAppleNearCom() {  }

    // 名前取得
    const char* GetName() const override { return "EnemyAppleNear"; }

    // 開始処理
    void Start()override;

    // 更新処理
    void Update(float elapsedTime)override;

    // GUI描画
    void OnGUI()override;

    // メッセージ受信したときの処理
    bool OnMessage(const Telegram& msg)override;

private:
    //アニメーション初期化設定
    void AnimationInitialize()override;

private:
};