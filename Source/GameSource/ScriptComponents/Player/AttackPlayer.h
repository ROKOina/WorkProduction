#pragma once

#include <memory>
#include <string>
#include <vector>
#include <DirectXMath.h>

#include "Audio\AudioSource.h"
#include "Audio\Audio.h"

//前方宣言
class PlayerCom;
class GameObject;

//プレイヤーの攻撃の動きのみ管理する
class AttackPlayer
{
public:
    AttackPlayer(std::shared_ptr<PlayerCom> player);
    ~AttackPlayer();

    void Update(float elapsedTime);

    void OnGui();

    void Render2D(float elapsedTime);

    //音解放
    void AudioRelease();
private:

    //コンボ継続確認処理
    void ComboJudge();
    //攻撃入力処理
    bool IsAttackInput(float elapsedTime);
    void SquareInput();
    void TriangleInput();
    //コンボ処理
    void ComboProcess(float elapsedTime);

    //攻撃動き処理
    void AttackMoveUpdate(float elapsedTime);

    //アニメインデックスで、コンボカウントリセット
    void AttackComboCountReset();

public://intでステートを返す、ステートをコードにする
    enum ATTACK_CODE
    {
        EnterAttack = 99,   //攻撃アニメーション許可
    };

    //ノーマル攻撃
public: 
    void NormalAttack(bool middleAssist = false);

private:
    int NormalAttackUpdate(float elapsedTime);

    //ダッシュ攻撃
public: 
    //ダッシュコンボ番号を引数で指定（１から）
    void DashAttack(int comboNum);

private:
    int DashAttackUpdate(float elapsedTime);

public:
    //攻撃アニメーション判定
    bool EndAttackState() { return state_ == ATTACK_CODE::EnterAttack; }

    //コンボ出来るか判定
    bool DoComboAttack();

    //攻撃判定中か判定
    bool InAttackJudgeNow();

    //直前の攻撃が当たっているか
    bool OnHitEnemy() { return onHitEnemy_; }

    void ResetState() { state_ = -1; }

    //敵を捉えているか
    bool ComboReadyEnemy() { 
        if (enemyCopy_.expired())return false;
        if (enemyCopy_.lock())return true;
        return false;
    }

    //着地時攻撃処理
    void AttackOnGround();

    //ジャンプ時攻撃処理
    void AttackJump();

    //強制的に攻撃を終わらせる（ジャンプ時等）
    void AttackFlagEnd();

private:
    //アシスト範囲を索敵して近い敵を返す
    std::shared_ptr<GameObject> AssistGetNearEnemy();

    //アシスト範囲を索敵して中距離敵を返す
    std::shared_ptr<GameObject> AssistGetMediumEnemy();

    //敵に接近する( true:接近完了　false:接近中 )
    bool ApproachEnemy(std::shared_ptr<GameObject> enemy, float dist, float speed = 1);

    //敵の方向へ回転する ( true:完了 )
    bool ForcusEnemy(float elapsedTime, std::shared_ptr<GameObject> enemy, float rollSpeed);

    //強攻撃動きと演出更新
    void SquareAttackDirection(float elapsedTime);
    //強攻撃出現
    void SpawnCombo1();
    void SpawnCombo2();
    void SpawnCombo3();

public: 
    //ゲッター＆セッター
    const bool& GetIsNormalAttack()const { return isNormalAttack_; }
    void SetIsNormalAttack(bool flag) { isNormalAttack_ = flag; }

    const int& GetComboSquareCount()const { return comboSquareCount_; }
    const int& GetComboTriangleCount()const { return comboTriangleCount_; }

    void SetAnimFlagName(std::string str) { animFlagName_ = str; }


private:
    std::weak_ptr<GameObject> enemyCopy_;  //敵保存

    //現在の攻撃の種類
    enum class ATTACK_FLAG
    {
        Normal,
        Dash,

        Null,
    };
    ATTACK_FLAG attackFlagState_ = ATTACK_FLAG::Null;

    //入力の種類を保存
    enum class ATTACK_KEY   //入力を判定
    {
        SQUARE,     //□
        TRIANGLE,   //△

        NULL_KEY,
    };
    ATTACK_KEY attackKey_ = ATTACK_KEY::NULL_KEY;
    ATTACK_KEY attackLeadInputKey_ = ATTACK_KEY::NULL_KEY;  //先行入力キー保存

    bool isNormalAttack_ = true;     //攻撃できるか

    bool isSquareAttack_ = true;

    //強攻撃管理
    struct SquareAttackMove
    {
        bool enable = false;    //起動中か
        float directionTime;    //演出タイム
        float directionTimer;
        float colliderScale;
        float colliderScaleEnd; //終了時の大きさ
        struct ObjData
        {
            std::weak_ptr<GameObject> obj;
            bool isMove;
            float speed;    //移動スピード
            DirectX::XMFLOAT3 velocity; //移動方向
        };
        std::vector<ObjData> objData;
    };
    bool isSquareDirection_ = false;    //強攻撃を一度打つため、演出していたらtrue
    int squareAttackKindCount_ = 3; //強攻撃の種類
    std::vector<SquareAttackMove> squareAttackMove_;

    bool isJumpSquareInput_ = false;    //ジャンプ中□コンボ一回までにするため

    //ジャンプ中攻撃のコンボ受付時間
    float jumpAttackComboWaitTime_ = 0.5f;
    float jumpAttackComboWaitTimer_ = 0;

    //コンボ継続処理判定をするか
    bool isComboJudge_ = true;

    //強制的にコンボを終わらせるフラグ
    bool comboJudgeEnd_ = false;

    //攻撃の段階（コンボ回数ではない）
    int comboSquareCount_ = 0;
    int comboTriangleCount_ = 0;

    //中距離アシストを有効にするか
    bool isMiddleAssist_ = false;

    //降下攻撃中か
    bool isJumpFall_ = false;

    //他から攻撃に引き継ぐ用
    std::string animFlagName_ = "";

    //敵に攻撃が当たっているかアニメーション中だけ保存
    bool onHitEnemy_ = false;

    //攻撃の動きを管理
    int state_ = -1;

    //ヒットエフェクト用
    float delayTimer_;   //少し遅らせる
    int weaponNum_ = -1;
    bool startHitEff_ = false; //ヒット確認

    //ロックオンUI用
    std::weak_ptr<GameObject> lockOnUIEnemy_;

    //SE
    std::unique_ptr<AudioSource> triangleChargeSE_ = Audio::Instance().LoadAudioSource("Data/Audio/Player/attackCharge2.wav");
    std::unique_ptr<AudioSource> triangle12SE_ = Audio::Instance().LoadAudioSource("Data/Audio/Player/triangle12.wav");
    std::unique_ptr<AudioSource> triangle3BombSE_ = Audio::Instance().LoadAudioSource("Data/Audio/Player/triangle3Bomb.wav");
    std::unique_ptr<AudioSource> triangle3SE_ = Audio::Instance().LoadAudioSource("Data/Audio/Player/triangle3_1.wav");
    std::unique_ptr<AudioSource> dashSE_ = Audio::Instance().LoadAudioSource("Data/Audio/Player/dash.wav");
    std::unique_ptr<AudioSource> haSE_ = Audio::Instance().LoadAudioSource("Data/Audio/Player/haVoice.wav");

private:
    std::weak_ptr<PlayerCom> player_;
};
