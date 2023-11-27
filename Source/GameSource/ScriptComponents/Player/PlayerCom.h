#pragma once

#include "Components\System\Component.h"
#include "AttackPlayer.h"
#include "MovePlayer.h"
#include "JustAvoidPlayer.h"

#include "Graphics/Sprite/Sprite.h"

class PostEffect;
class CameraCom;

//アニメーションリスト
enum ANIMATION_PLAYER
{
    WALK_RUNRUN_1,
    IDEL_1,
    IDEL_2,
    JUMP_1,
    JUMP_2,
    RUN_HARD_1,
    RUN_HARD_2,
    RUN_SOFT_1,
    RUN_SOFT_2,
    WALK_RUNRUN_2,
    PUNCH,
    BIGSWORD_UP,
    BIGSWORD_LEFT,
    BIGSWORD_RIGHT,
    BIGSWORD_DOWN,
    DASH_ANIM,
    BIGSWORD_COM1_01,
    BIGSWORD_COM1_02,
    BIGSWORD_COM1_03,
    BIGSWORD_COM2_01,
    BIGSWORD_COM2_02,
    BIGSWORD_COM2_03,
    BIGSWORD_DASH,
    JUMP_IN,
    DODGE_BACK,
    DODGE_FRONT,
    DODGE_LEFT,
    DODGE_RIGHT,
    DASH_BACK,
    RUN_TURN,
    RUN_STOP,
    RUN_TURN_FORWARD,
    JUMP_ATTACK_UPPER,
    JUMP_ATTACK_DOWN_START,
    JUMP_ATTACK_DOWN_END,
    JUMP_ATTACK_DOWN_DO,
    JUMP_FALL,
    BIGSWORD_DASH_3,
    JUMP_ATTACK_06,
    JUMP_ATTACK_05,
    JUMP_ATTACK_04,
    JUMP_ATTACK_03,
    JUMP_ATTACK_02,
    JUMP_ATTACK_01,
    DAMAGE_RIGHT,
    DAMAGE_LEFT,
    DAMAGE_FRONT,
    DAMAGE_BACK,
    SIT_TITLE,
    STANDUP_TITLE,
    TRIANGLE_ATTACK_02,
    TRIANGLE_ATTACK_01,
    TRIANGLE_ATTACK_03,
    TRIANGLE_ATTACK_PUSH,
};

class PlayerCom : public Component
{
    //コンポーネントオーバーライド
public:
    PlayerCom() {}
    ~PlayerCom() {}

    // 名前取得
    const char* GetName() const override { return "Player"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

    // sprite描画
    void Render2D(float elapsedTime) override;

    //PlayerComクラス
public:
    //mask
    void MaskRender(PostEffect* postEff, std::shared_ptr<CameraCom> maskCamera);

private:
    //アニメーション初期化設定
    void AnimationInitialize();

public:
    //今のプレイヤーの遷移状態
    enum class PLAYER_STATUS
    {
        IDLE,
        MOVE,
        DASH,
        BACK_DASH,
        JUMP,
        JUST,
        JUMP_DASH,
        JUMP_BACK_DASH,
        JUMP_JUST,

        ATTACK,
        ATTACK_DASH,
        ATTACK_JUMP,
        ATTACK_JUMP_FALL_BEFORE,
        ATTACK_JUMP_FALL,
    };
    PLAYER_STATUS GetPlayerStatus() { return playerStatus_; }
    PLAYER_STATUS GetPlayerStatusOld() { return playerStatusOld_; }
    void SetPlayerStatus(PLAYER_STATUS status) {
        if (status != playerStatus_)
            playerStatusOld_ = playerStatus_;
        playerStatus_ = status;
    }

    std::shared_ptr<MovePlayer> GetMovePlayer() { return movePlayer_; }
    std::shared_ptr<AttackPlayer> GetAttackPlayer() { return attackPlayer_; }
    std::shared_ptr<JustAvoidPlayer> GetJustAvoidPlayer() { return justAvoidPlayer_; }

    //放射線ブラーをプレイヤー中心に
    void BlurStartPlayer(float power, float time
        , std::string boneName = "", DirectX::XMFLOAT2 pos = { (1920.0f * 0.8f) / 2.0f ,(1080.0f * 0.8f) / 2.0f }
        , std::shared_ptr<GameObject> posObj = nullptr
    );
    //ブラー中か
    bool IsBlurPlay() {
        if (blurTimer_ < blurTime_)
            return true;

        return false;
    }

private:
    PLAYER_STATUS playerStatus_ = PLAYER_STATUS::IDLE;
    PLAYER_STATUS playerStatusOld_ = PLAYER_STATUS::IDLE;

    //プレイヤーの攻撃の動きのみ管理する
    std::shared_ptr<AttackPlayer> attackPlayer_;
    std::shared_ptr<MovePlayer> movePlayer_;
    std::shared_ptr<JustAvoidPlayer> justAvoidPlayer_;

    //ブラー
    float blurTimer_;
    float blurTime_;
    float blurPower_;
    std::string blurBoneName_;
    std::weak_ptr<GameObject> blurPosObj_;

    //UI
    //ワイプ背景
    bool startUI_ = false;
    std::unique_ptr<Sprite> faceFrameUI_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceFrame.png");
    //HP
    std::unique_ptr<Sprite> hpSprite_[8];
    DirectX::XMFLOAT2 hpDonutsPos_[8];
    bool isHpDirection_ = false;	//演出フラグ
    DirectX::XMFLOAT3 hpDir_;	//posと透明値
    DirectX::XMFLOAT2 dirVelo_;
    float hpGravity_ = 10;
    //皿
    std::unique_ptr<Sprite> saraSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/saraUI.png");

    //ロックオン
    std::unique_ptr<Sprite> lockSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/lookOn.png");
};