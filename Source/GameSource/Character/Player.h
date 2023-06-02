#pragma once

#include "Graphics/Shader.h"
#include "Graphics/Model.h"
#include "GameSource\Character\Character.h"
#include "GameSource\Render\Effect\Effect.h"
#include "GameSource\Camera\CameraController.h"
#include "GameSource\Weapon\Weapon.h"

//プレイヤー
class Player : public Character
{
public: //関数
    Player(CameraController* camera);
    ~Player() override;

    //インスタンス取得
    static Player& Instance();

    //更新処理
    void Update(float elapsedTime);

    //描画処理
    void Render(ID3D11DeviceContext* dc, Shader* shader);

    //デバッグ用GUI描画
    void DrawDebugGUI();

    //デバッグプリミティブ描画
    void DrawDebugPrimitive();

    //ジャンプ入力処理
    bool InputJump();

private:    //関数
    //スティック入力値から移動ベクトルを取得
    DirectX::XMFLOAT3 GetMoveVec() const;

    //移動入力処理
    bool InputMove(float elapsedTime);

    //攻撃入力処理
    bool InputAttack();

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

    int JumpCount = 0;
    int JumpLimit = 2;

    //ボーン
    float leftHandRadius = 0.4f;
    bool attackCollisionFlag = false;

    //ImGui
    bool VisibleDebugPrimitive = true;

    //エフェクト
    Effect* hitEffect = nullptr;

    std::unique_ptr<CameraController> cameraController;

    std::unique_ptr<Weapon> w;

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
        Anim_spin,
        Anim_happyWalk,
        Anim_ninjaWalk,
    };
};