#pragma once

#include "Components\System\Component.h"
#include "AttackPlayer.h"
#include "MovePlayer.h"
#include "JustAvoidPlayer.h"

#include "Graphics/Sprite/Sprite.h"

class PostEffect;
class CameraCom;

//�A�j���[�V�������X�g
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
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    PlayerCom() {}
    ~PlayerCom() {}

    // ���O�擾
    const char* GetName() const override { return "Player"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    // sprite�`��
    void Render2D(float elapsedTime) override;

    //PlayerCom�N���X
public:
    //mask
    void MaskRender(PostEffect* postEff, std::shared_ptr<CameraCom> maskCamera);

private:
    //�A�j���[�V�����������ݒ�
    void AnimationInitialize();

public:
    //���̃v���C���[�̑J�ڏ��
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

    //���ː��u���[���v���C���[���S��
    void BlurStartPlayer(float power, float time
        , std::string boneName = "", DirectX::XMFLOAT2 pos = { (1920.0f * 0.8f) / 2.0f ,(1080.0f * 0.8f) / 2.0f }
        , std::shared_ptr<GameObject> posObj = nullptr
    );
    //�u���[����
    bool IsBlurPlay() {
        if (blurTimer_ < blurTime_)
            return true;

        return false;
    }

    //�r�l�b�g�X�^�[�g�i�_���[�W���j
    void VignetteStart(float power, float time);

    //���b�N�I���摜�Q�b�g
    Sprite* GetLockOnSprite() { return lockSprite_.get(); }
    enum LOCK_TARGET    //���b�N�I���ύX�p
    {
        NORMAL_LOCK,
        JUST_LOCK,
    };
    void SetLockOn(LOCK_TARGET target) { lockOnTarget_ = target; }
    LOCK_TARGET GetLockOn() { return lockOnTarget_; }

private:
    PLAYER_STATUS playerStatus_ = PLAYER_STATUS::IDLE;
    PLAYER_STATUS playerStatusOld_ = PLAYER_STATUS::IDLE;

    //�v���C���[�̍U���̓����̂݊Ǘ�����
    std::shared_ptr<AttackPlayer> attackPlayer_;
    std::shared_ptr<MovePlayer> movePlayer_;
    std::shared_ptr<JustAvoidPlayer> justAvoidPlayer_;

    //�u���[
    float blurTimer_;
    float blurTime_;
    float blurPower_;
    std::string blurBoneName_;
    std::weak_ptr<GameObject> blurPosObj_;

    //�_���[�W�r�l�b�g
    float vignetteDamageTime_;
    float vignetteDamageTimer_;
    float vignetteDamagePower_;
    //�m�����r�l�b�g
    struct VignetteLowHP
    {
        float power;
        float speed;
        float powerOffset;
    } vignetteHPPower_[3];
    float vignetteLowHP_;

    //UI
    //���C�v�w�i
    bool startUI_ = false;
    std::unique_ptr<Sprite> faceFrameUI_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceFrame.png");
    //HP
    std::unique_ptr<Sprite> hpSprite_[8];
    DirectX::XMFLOAT2 hpDonutsPos_[8];
    bool isHpDirection_ = false;	//���o�t���O
    DirectX::XMFLOAT3 hpDir_;	//pos�Ɠ����l
    DirectX::XMFLOAT2 dirVelo_;
    float hpGravity_ = 10;
    //�M
    std::unique_ptr<Sprite> saraSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/saraUI.png");

    //���b�N�I��
    LOCK_TARGET lockOnTarget_ = LOCK_TARGET::NORMAL_LOCK;
    std::unique_ptr<Sprite> lockSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/lookOn.png");

    //�{�^��
    std::unique_ptr<Sprite> XSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/XButton.png");
    std::unique_ptr<Sprite> YSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/YButton.png");
    std::unique_ptr<Sprite> YTuyoSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/YTuyoButton.png");
    std::unique_ptr<Sprite> ASprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/AButton.png");
    std::unique_ptr<Sprite> BSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/BButton.png");
    std::unique_ptr<Sprite> RTSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/RTButton.png");
    DirectX::XMFLOAT2 buttonPos_ = {};
    DirectX::XMFLOAT2 offsetButtonPos_[5] = {};
    float buttonSize_[5] = {};  //���o�p�T�C�Y
    //����
    std::unique_ptr<Sprite> attackStringSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/attackString.png");
    std::unique_ptr<Sprite> attackJumpStringSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/attackJumpString.png");
    std::unique_ptr<Sprite> attackRangeStringSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/attackRangeString.png");
    std::unique_ptr<Sprite> jumpStringSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/jumpString.png");
    std::unique_ptr<Sprite> dashStringSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/dashString.png");
    std::unique_ptr<Sprite> sniperStringSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/sniperAttackString.png");
    std::unique_ptr<Sprite> slowStringSprite_ = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/button/slowAttackString.png");
    bool spriteRightPost_[5];    //�{�^���̉E���ɒu����
};