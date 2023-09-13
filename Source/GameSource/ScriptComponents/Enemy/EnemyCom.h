#pragma once

#include "Components\System\Component.h"


#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorData.h"
#include "BehaviorTree/NodeBase.h"

//アニメーションリスト
enum ANIMATION_ENEMY
{
    WALK,
    RUN,
    RUN_BACK,
    JUMP,
    IDEL,
    KICK,
    DAMAGE,
    RIGHT_STRAIGHT01,
    LEFT_UPPER01,
};

class EnemyCom : public Component
{
    //コンポーネントオーバーライド
public:
    EnemyCom() {}
    ~EnemyCom() { activeNode_.release(); }

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
    // ターゲット位置をランダム設定
    void SetRandomTargetPosition();

    void SetTargetPosition(DirectX::XMFLOAT3 pos) { targetPosition_ = pos; }
    DirectX::XMFLOAT3 GetTargetPosition() { return targetPosition_; }

    //プレイヤーが近いとtrue
    bool SearchPlayer();

    float GetAttackRange() { return attackRange_; }

    //ダメージ確認
    bool OnDamageEnemy();

    //ジャスト回避フラグオン
    void SetIsJustAvoid(bool flag) { isJustAvoid_ = flag; }
    bool GetIsJustAvoid() { return isJustAvoid_; }

private:
    //アニメーション初期化設定
    void AnimationInitialize();

    //AI
    std::unique_ptr<BehaviorTree> aiTree_;
    std::unique_ptr<BehaviorData> behaviorData_;
    std::unique_ptr<NodeBase> activeNode_;

    DirectX::XMFLOAT3 targetPosition_;
    //攻撃範囲
    float attackRange_ = 2;
    //索敵範囲
    float searchRange_ = 5;

    //ジャスト回避をActionDerivedと繋ぐため
    bool isJustAvoid_ = false;

};
