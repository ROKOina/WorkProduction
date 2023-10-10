#pragma once

#include "EnemyCom.h"

//近接敵
class EnemyNearCom :public EnemyCom
{
public:
    EnemyNearCom() {}
    ~EnemyNearCom() { activeNode_.release(); }

    // 名前取得
    const char* GetName() const override { return "EnemyNear"; }

    // 開始処理
    void Start()override;

    // 更新処理
    void Update(float elapsedTime)override;

    // GUI描画
    void OnGUI()override;

private:
    //被弾時にアニメーションする時のAITREEを決める
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);

    //アニメーション初期化設定
    void AnimationInitialize()override;
};