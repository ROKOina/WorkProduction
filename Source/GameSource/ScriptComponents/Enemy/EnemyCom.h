#pragma once

#include "Components\System\Component.h"

class EnemyCom : public Component
{
    //コンポーネントオーバーライド
public:
    EnemyCom() {}
    ~EnemyCom() {}

    // 名前取得
    const char* GetName() const override { return "Enemy"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

    //EnemyComクラス
private:
    //アニメーション初期化設定
    void AnimationInitialize();




};
