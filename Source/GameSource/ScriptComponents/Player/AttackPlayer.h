#pragma once

#include <memory>
#include <string>
#include <vector>
#include <DirectXMath.h>

//前方宣言
class PlayerCom;
class GameObject;

//プレイヤーの攻撃の動きのみ管理する
class AttackPlayer
{
public:
    AttackPlayer(std::shared_ptr<PlayerCom> player) : player_(player) 
    {
        squareAttackMove_.resize(squareAttackKindCount_);
        squareAttackMove_[0].directionTime = 2;
        squareAttackMove_[1].directionTime = 2;
        squareAttackMove_[2].directionTime = 2;
    }
    ~AttackPlayer(){}

    void Update(float elapsedTime);

    void OnGui();

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

private:
    std::weak_ptr<PlayerCom> player_;
};

/*
コンボ図（＞:次の行動、＾:どちらでも繋がる）

ダッシュ
DASH＞△＞DASH＞□＾△

*/