#pragma once

#include <memory>

//前方宣言
class PlayerCom;
class GameObject;

//プレイヤーの攻撃の動きのみ管理する
class AttackPlayer
{
public:
    AttackPlayer(std::shared_ptr<PlayerCom> player) : player_(player) {}
    ~AttackPlayer(){}

    void Update(float elapsedTime);

public://intでステートを返す、ステートをコードにする
    enum ATTACK_CODE
    {
        EnterAttack = 99,   //攻撃アニメーション許可
    };

public: 
    void NormalAttack();

private:
    int NormalAttackUpdate(float elapsedTime);


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

    //直前の攻撃が当たっているか
    bool OnHitEnemy() { return onHitEnemy_; }

    void ResetState() { state_ = -1; }

    //敵を捉えているか
    bool ComboReadyEnemy() { 
        if (enemyCopy_)return true;
        return false;
    }

private:
    //アシスト範囲を索敵して近い敵を返す
    std::shared_ptr<GameObject> AssistGetNearEnemy();

    //敵に接近する( true:接近完了　false:接近中 )
    bool ApproachEnemy(std::shared_ptr<GameObject> enemy, float dist, float speed = 1);

    //敵の方向へ回転する ( true:完了 )
    bool ForcusEnemy(float elapsedTime, std::shared_ptr<GameObject> enemy, float rollSpeed);

private:
    std::weak_ptr<PlayerCom> player_;

    std::shared_ptr<GameObject> enemyCopy_;  //敵保存

    //現在の攻撃の種類
    enum class ATTACK_FLAG
    {
        Normal,
        Dash,

        Null,
    };
    ATTACK_FLAG attackFlagState_ = ATTACK_FLAG::Null;

    bool onHitEnemy_ = false;

    //攻撃の動きを管理
    int state_ = -1;
};

/*
コンボ図（＞:次の行動、＾:どちらでも繋がる）

ダッシュ
DASH＞△＞DASH＞□＾△

*/