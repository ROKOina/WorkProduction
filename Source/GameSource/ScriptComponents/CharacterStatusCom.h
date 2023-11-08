#pragma once

#include "Components\System\Component.h"
#include "GameSource/ScriptComponents/Weapon/WeaponCom.h"

//キャラクターの状態を管理
class CharacterStatusCom : public Component
{
    //コンポーネントオーバーライド
public:
    CharacterStatusCom() {}
    ~CharacterStatusCom() {}

    // 名前取得
    const char* GetName() const override { return "CharacterStatus"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

    //CharacterStatusComクラス
public:
    //ダメージ（invincibleNonDamageはプレイヤーアニメーション関係なく無敵時間判定）
    void OnDamage(std::shared_ptr<WeaponCom> weapon, int minasHP, DirectX::XMFLOAT3& power = DirectX::XMFLOAT3(0, 0, 0)
        , ATTACK_SPECIAL_TYPE attackType = ATTACK_SPECIAL_TYPE::NORMAL, float invincibleNonDamage = -1.0f);

    //無敵時間
    void SetInvincibleTime(float time) { damageInvincibleTime_ = time; }
    float GetInvincibleTime() { return damageInvincibleTime_; }

    bool GetIsInvincible() { return isInvincible_; }

    //ダメージとは別の無敵時間
    void SetInvincibleNonDamage(float time){
        invincibleSetTimer = time;
        invincibleSetFlag = true;
        isInvincible_ = true;
    }

    //ダメージを受けた時だけtrueを返す
    bool GetFrameDamage() { return isFrameDamage_; }

    //ダメージアニメーション中ならtrue
    bool GetDamageAnimation() { return isAnimDamage_; }

    //攻撃による移動を制限
    void SetAttackNonMove(bool flag) { isAttackNonMove_ = flag; }
    bool GetAttackNonMove() { return isAttackNonMove_; }

    //hp取得
    void SetMaxHP(int hp) { maxHp_ = hp; }
    int GetMaxHP() { return maxHp_; }
    void SetHP(int hp) { hp_ = hp; }
    int GetHP() { return hp_; }

    //ダメージの種類を取得
    ATTACK_SPECIAL_TYPE GetDamageType() { return damageType_; }

private:
    //状態系
    bool isInvincible_ = false; //無敵時間はtrue
    bool isFrameDamage_ = false; //ダメージを受けた時だけtrue
    bool isAnimDamage_ = false; //ダメージアニメーション中はtrue
    bool oneFrameJudge_ = false;    //一フレームを測る

    float damageInvincibleTime_ = 1.5f; //ダメージ時の無敵時間
    float damageTimer_ = 0; //ダメージ時のタイマー

    //一定時間無敵になる
    float invincibleSetTimer = 0;
    bool invincibleSetFlag = false;

    //ダメージ情報
    ATTACK_SPECIAL_TYPE damageType_ = ATTACK_SPECIAL_TYPE::NORMAL; //特殊な攻撃を１フレーム保存

    //hp
    int hp_ = 100;
    int maxHp_ = 100;

    //被弾時に保存する
    std::weak_ptr<WeaponCom> saveWeapon_;

    //攻撃による移動を制限
    bool isAttackNonMove_ = false;
};