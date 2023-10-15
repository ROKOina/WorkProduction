#pragma once

#include "Components\System\Component.h"


#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorData.h"
#include "BehaviorTree/NodeBase.h"

#include "TelegramEnemy.h"

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
    RUN_SWORD,
    IDLE_SWORD,
    WALK_SWORD,
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
    ~EnemyCom() { }

    // 名前取得
    virtual const char* GetName() const = 0;

    // 開始処理
    virtual void Start();

    // 更新処理
    virtual void Update(float elapsedTime);

    // GUI描画
    virtual void OnGUI();

    //EnemyComクラス
private:
    struct MoveDataEnemy
    {
        float walkSpeed = 1.0f;
        float walkMaxSpeed = 2.0f;
        float runSpeed = 1.0f;
        float runMaxSpeed = 5.0f;
    };

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

    //攻撃フラグ
    void SetIsAttackFlag(bool falg) { isAttackFlag_ = falg; }
    bool GetIsAttackFlag() { return isAttackFlag_; }

    //被弾ー＞立ち上がりモーション
    void SetStandUpMotion();
    void StandUpUpdate();

    const MoveDataEnemy& GetMoveDataEnemy() const { return moveDataEnemy; }

    //AI関係

    // メッセージ受信したときの処理
    virtual bool OnMessage(const Telegram& msg);

    int GetID() { return enemyId_; }
    void SetID(int id) { enemyId_ = id; }

private:    //これだけ何故か派生クラスで使えないので、派生クラスでも作成する
    //被弾時にアニメーションする時のAITREEを決める
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);

protected:
    //アニメーション初期化設定
    virtual void AnimationInitialize(){}

    //ダメージ処理
    void DamageProcess(float elapsedTime);

    //ジャスト回避用判定出す
    void justColliderProcess();

    //重力設定
    void GravityProcess(float elapsedTime);

    //AI
    std::shared_ptr<BehaviorTree> aiTree_;
    std::shared_ptr<BehaviorData> behaviorData_;
    std::shared_ptr<NodeBase> activeNode_;

    //ダメージAITREE番号
    std::vector<int> damageAnimAiTreeId_;
    bool isAnimDamage_ = false; //ダメージアニメーションするときはtrue
    bool oldAnimDamage_ = false;

    DirectX::XMFLOAT3 targetPosition_;
    //攻撃範囲
    float attackRange_ = 2;
    //索敵範囲
    float searchRange_ = 7;

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

    //攻撃フラグ
    bool isAttackFlag_ = false;

    //移動
    MoveDataEnemy moveDataEnemy;

    //識別番号
    int enemyId_ = -1;
};
