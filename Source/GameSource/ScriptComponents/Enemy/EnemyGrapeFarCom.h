#pragma once

#include "EnemyFarCom.h"

//近接敵
class EnemyGrapeFarCom :public EnemyFarCom
{
public:
    EnemyGrapeFarCom() {}
    ~EnemyGrapeFarCom() {  }

    // 名前取得
    const char* GetName() const override { return "EnemyGrapeFar"; }

    // 開始処理
    void Start()override;

    // 更新処理
    void Update(float elapsedTime)override;

    // GUI描画
    void OnGUI()override;

private:
    //アニメーション初期化設定
    void AnimationInitialize()override;

    //被弾時にアニメーションする時のAITREEを決める
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);


    //弾生成処理
    void SpawnGrapeBall();

private:
    //一回だけ弾出現させるため
    bool isSpawnBall_ = false;
};

//弾用
class GrapeBallCom :public Component
{
    //コンポーネントオーバーライド
public:
    GrapeBallCom() {}
    ~GrapeBallCom() {}

    // 名前取得
    const char* GetName() const override { return "GrapeBall"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

public:

private:
    float speed_ = 4.5f;
    float rotaAsistPower_ = 1.0f;   //プレイヤーへの弾の吸い付きパワー

    bool endFlag_ = false;
    float removeTimer = 5.0f;
};