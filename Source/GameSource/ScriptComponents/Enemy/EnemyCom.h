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
public:
    //ダメージ
    void OnDamage(DirectX::XMFLOAT3& power = DirectX::XMFLOAT3(0, 0, 0));

    bool GetIsInvincible() {
        return isDamage_;
    }

private:
    //アニメーション初期化設定
    void AnimationInitialize();



private:

    //状態系
    bool isDamage_ = false; //ダメージを受けている時にtrue
    float damageInvincibleTime_ = 1; //ダメージ時の無敵時間
    float damageTimer_ = 0; //ダメージ時のタイマー

};
