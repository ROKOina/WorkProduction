#pragma once

#include "Components\System\Component.h"

#include "Graphics/Sprite/Sprite.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorData.h"
#include "BehaviorTree/NodeBase.h"

#include "TelegramEnemy.h"

class PostEffect;
class CameraCom;


//AI遷移
enum class AI_TREE
{
    NONE_AI,
    ROOT,

    //2層
    //親：ROOT 
    BATTLE,
    SCOUT,

    //3層
    //親：SCOUT 
    WANDER,
    IDLE,

    //親：BATTLE
    ATTACK,
    ROUTE,
    BACK_MOVE,
    ATTACK_IDLE,
    PURSUIT,

    //4層
    //親：ATTACK
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

    // sprite描画
    virtual void Render2D(float elapsedTime);

    //EnemyComクラス
public:
    //mask
    void MaskRender(PostEffect* postEff, std::shared_ptr<CameraCom> maskCamera);

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
    const DirectX::XMFLOAT3& GetTargetPosition() const { return targetPosition_; }

    //ターゲット位置に移動、回転
    void GoTargetMove(bool isMove = true, bool isTurn = true);

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

    //攻撃前待機フラグ
    bool GetIsAttackIdleFlag() { return isAttackIdle_; }
    void SetIsAttackIdleFlag(bool flag) { isAttackIdle_ = flag; }

    //被弾ー＞立ち上がりモーション
    void SetStandUpMotion();
    void StandUpUpdate();

    const MoveDataEnemy& GetMoveDataEnemy() const { return moveDataEnemy_; }

    //AI関係

    // メッセージ受信したときの処理
    virtual bool OnMessage(const Telegram& msg);

    int GetID() { return enemyId_; }
    void SetID(int id) { enemyId_ = id; }

private:
    //アニメーションの更新
    void AnimationSetting();

private:    //これだけ何故か派生クラスで使えないので、派生クラスでも作成する
    //被弾時にアニメーションする時のAITREEを決める
    template<typename... Args>
    void OnDamageAnimAI_TREE(Args... args);

protected:
    //アニメーション初期化設定
    virtual void AnimationInitialize() {}

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

    float damageEffTimer_;

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
    int getUpAnim_;
    bool isStandUpMotion_ = false;
    bool playStandUp_ = false;

    //攻撃フラグ
    bool isAttackFlag_ = false;

    //攻撃前待機
    bool isAttackIdle_ = false;

    //移動
    MoveDataEnemy moveDataEnemy_;

    //HPBar
    std::unique_ptr<Sprite>     hpSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Enemy/enemyHp.png");
    std::unique_ptr<Sprite>     hpBackSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Enemy/enemyHpGage.png");
    std::unique_ptr<Sprite>     hpMaskSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Enemy/enemyHpMask.png");
    DirectX::XMFLOAT2 sP{0,0};
    DirectX::XMFLOAT3 saP{0,0,0};
    //識別番号
    int enemyId_ = -1;
};
