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
    //ダメージ
    void OnDamage(DirectX::XMFLOAT3& power = DirectX::XMFLOAT3(0, 0, 0), ATTACK_SPECIAL_TYPE attackType = ATTACK_SPECIAL_TYPE::NORMAL);

    //無敵時間
    void SetInvincibleTime(float time) { damageInvincibleTime_ = time; }
    float GetInvincibleTime() { return damageInvincibleTime_; }

    bool GetIsInvincible() {
        return isDamage_;
    }

    //ダメージを受けた時だけtrueを返す
    bool GetFrameDamage() { return isFrameDamage_; }

    //ダメージアニメーション中ならtrue
    bool GetDamageAnimation() { return isAnimDamage_; }

    //攻撃による移動を制限
    void SetAttackNonMove(bool flag) { isAttackNonMove_ = flag; }
    bool GetAttackNonMove() { return isAttackNonMove_; }

    //ダメージの種類を取得
    ATTACK_SPECIAL_TYPE GetDamageType() { return damageType_; }

private:
    //状態系
    bool isDamage_ = false; //ダメージを受けている時にtrue
    bool isFrameDamage_ = false; //ダメージを受けた時だけtrue
    bool isAnimDamage_ = false; //ダメージアニメーション中はtrue
    bool oneFrameJudge = false;
    ATTACK_SPECIAL_TYPE damageType_ = ATTACK_SPECIAL_TYPE::NORMAL; //特殊な攻撃を１フレーム保存
    float damageInvincibleTime_ = 0.5f; //ダメージ時の無敵時間
    float damageTimer_ = 0; //ダメージ時のタイマー

    int hp_ = 100;

    //攻撃による移動を制限
    bool isAttackNonMove_ = false;
};