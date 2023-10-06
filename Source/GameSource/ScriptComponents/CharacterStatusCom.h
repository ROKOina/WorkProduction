#pragma once

#include "Components\System\Component.h"

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

    //更新前
    void UpdateStart() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

    //CharacterStatusComクラス
public:
    //ダメージ
    void OnDamage(DirectX::XMFLOAT3& power = DirectX::XMFLOAT3(0, 0, 0));

    bool GetIsInvincible() {
        return isDamage_;
    }

    //ダメージを受けた時だけtrueを返す
    bool GetFrameDamage() { return isFrameDamage_; }



private:
    //状態系
    bool isDamage_ = false; //ダメージを受けている時にtrue
    bool isFrameDamage_ = false; //ダメージを受けた時だけtrue
    bool oneFrameJudge = false;
    float damageInvincibleTime_ = 0.5f; //ダメージ時の無敵時間
    float damageTimer_ = 0; //ダメージ時のタイマー

};