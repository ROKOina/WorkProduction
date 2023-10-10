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
    DAMAGE_FALL,
    DAMAGE_IN_AIR,
    DAMAGE_GO_FLY,
    FALL_STAND_UP,
    DAMAGE_FALL_END,
    ATTACK01_SWORD,
    WALK_SWORD,
    IDLE_SWORD,
    RUN_SWORD,
};

//AI遷移
enum class AI_TREE
{
    NONE,
    ROOT,

    //2層
    //ROOT 
    BATTLE,
    SCOUT,

    //3層
    //SCOUT 
    WANDER,
    IDLE,

    //BATTLE
    ATTACK,
    PURSUIT,

    //4層
    //ATTACK
    NORMAL,
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


    //被弾時にアニメーションする時のAITREEを決める
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);

    //被弾ー＞立ち上がりモーション
    void SetStandUpMotion();
    void StandUpUpdate();

private:
    //アニメーション初期化設定
    void AnimationInitialize();

    //ダメージ処理
    void DamageProcess(float elapsedTime);

    //ジャスト回避用判定出す
    void justColliderProcess();

    //重力設定
    void GravityProcess(float elapsedTime);

    //AI
    std::unique_ptr<BehaviorTree> aiTree_;
    std::unique_ptr<BehaviorData> behaviorData_;
    std::unique_ptr<NodeBase> activeNode_;

    //ダメージAITREE番号
    std::vector<int> damageAnimAiTreeId_;
    bool isAnimDamage_ = false; //ダメージアニメーションするときはtrue
    bool oldAnimDamage_ = false;

    DirectX::XMFLOAT3 targetPosition_;
    //攻撃範囲
    float attackRange_ = 2;
    //索敵範囲
    float searchRange_ = 5;

    //ジャスト回避をActionDerivedと繋ぐため
    bool isJustAvoid_ = false;

    //ジャンプ被弾時
    bool isJumpDamage_ = false;
    //ジャンプ被弾後の重力なくすタイマー
    float skyGravityZeroTimer_ = 0;
    //重力設定用
    bool isSetGravity_ = false;

    //起き上がりモーション中
    bool isStandUpMotion_ = false;
    bool playStandUp_ = false;
};
