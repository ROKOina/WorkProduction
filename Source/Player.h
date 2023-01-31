#pragma once

#include "Graphics/Shader.h"
#include "Graphics/Model.h"
#include "Character.h"
#include "ProjectileManager.h"
#include "Effect.h"

//プレイヤー
class Player : public Character
{
public: //関数
    Player();
    ~Player() override;

    //インスタンス取得
    static Player& Instance();

    //更新処理
    void Update(float elapsedTime);

    //描画処理
    void Render(ID3D11DeviceContext* dc, Shader* shader);

    //プレイヤーとエネミーとの衝突処理
    void CollisionPlayerVsEnemies();

    //弾丸と敵の衝突処理
    void CollisionProjectileVsEnemies();

    //デバッグ用GUI描画
    void DrawDebugGUI();

    //デバッグプリミティブ描画
    void DrawDebugPrimitive();

    ////ジャンプ処理
    //void Jump(float speed);

    ////速力処理更新
    //void UpdateVelocity(float elapsedTime);

    //ジャンプ入力処理
    bool InputJump();

private:    //関数
    //スティック入力値から移動ベクトルを取得
    DirectX::XMFLOAT3 GetMoveVec() const;

    //移動入力処理
    bool InputMove(float elapsedTime);

    //弾丸入力処理
    void InputProjectile();

    //攻撃入力処理
    bool InputAttack();

    ////旋回処理
    //void Turn(float elapsedTime, float vx, float vz, float speed);

    //ノードとエネミーの衝突判定
    void CollisionNodeVsEnemies(const char* nodeName, float nodeRadius);

protected:
    //着地した時に呼ばれる
    void OnLanding()override;

    //ダメージを受けた時に呼ばれる
    void OnDamage()override;

    //死亡した時に呼ばれる
    void OnDead()override;

private:    //変数
    Model* model = nullptr;
    float moveSpeed = 5.0f;
    float turnSpeed = DirectX::XMConvertToRadians(720);

    //ジャンプ
    float jumpSpeed = 20.0f;
    //float gravity = -1.0f;
    //DirectX::XMFLOAT3 velocity = { 0,0,0 }; //速力

    int JumpCount = 0;
    int JumpLimit = 2;

    ProjectileManager projectileManager;

    //ボーン
    float leftHandRadius = 0.4f;
    bool attackCollisionFlag = false;

    //ImGui
    bool VisibleDebugPrimitive = true;

    //エフェクト
    Effect* hitEffect = nullptr;

private:   //アニメーション 
    //遷移
    //待機ステートへ遷移
    void TransitionIdleState();
    //移動ステートへ遷移
    void TransitionMoveState();
    //ジャンプステートへ遷移
    void TransitionJumpState();
    //着地ステートへ遷移
    void TransitionLandState();
    //攻撃ステートへ遷移
    void TransitionAttackState();
    //ダメージステートへ遷移
    void TransitionDamageState();
    //死亡ステートへ遷移
    void TransitionDeathState();
    //復活ステートへ遷移
    void TransitionReviveState();
    
    


    //更新処理
    //待機ステート更新処理
    void UpdateIdleState(float elapsedTime);
    //移動ステート更新処理
    void UpdateMoveState(float elapsedTime);
    //ジャンプステート更新処理
    void UpdateJumpState(float elapsedTime);
    //着地ステート更新処理
    void UpdateLandState(float elapsedTime);
    //攻撃ステート更新処理
    void UpdateAttackState(float elapsedTime);
    //ダメージステート更新処理
    void UpdateDamageState(float elapsedTime);
    //死亡ステート更新処理
    void UpdateDeathState(float elapsedTime);
    //復活ステート更新処理
    void UpdateReviveState(float elapsedTime);

    enum class State {
        Idle,
        Move,
        Jump,
        Land,
        Attack,
        Damage,
        Death,
        Revive
    };

    State state = State::Idle;

    //アニメーション
    enum Animation
    {
        Anim_Attack,
        Anim_Death,
        Anim_Falling,
        Anim_GetHit1,
        Anim_GetHit2,
        Anim_Idle,
        Anim_Jump,
        Anim_Jump_Flip,
        Anim_Landing,
        Anim_Revive,
        Anim_Running,
        Anim_Walking 
    };
};