#include "CharacterStatusCom.h"

#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "GameSource/ScriptComponents/Player/PlayerCom.h"

#include <imgui.h>

// 開始処理
void CharacterStatusCom::Start()
{

}

// 更新処理
void CharacterStatusCom::Update(float elapsedTime)
{
    //1フレームだけisFrameDamage_をtrueにする
    if (isFrameDamage_)
    {
        //１フレーム後の処理
        if (oneFrameJudge_)
        {
            isFrameDamage_ = false;
            oneFrameJudge_ = false;
            isAnimDamage_ = true;   //アニメーション中
            damageType_ = ATTACK_SPECIAL_TYPE::NORMAL;
        }
        else
            oneFrameJudge_ = true;
    }

    //無敵終了処理
    if (isInvincible_)
    {
        //プレイヤーの被弾
        if (GetGameObject()->GetComponent<PlayerCom>())
        {
            if (!isAnimDamage_) //ダメージアニメーション中ではない場合
            {
                damageTimer_ += elapsedTime;
                if (damageTimer_ > damageInvincibleTime_)
                {
                    if (!invincibleSetFlag) //無敵時間を別に設定している場合はとばす
                        isInvincible_ = false;//無敵終了
                    damageTimer_ = 0;
                }
            }
        }
        else //敵の被弾時間
        {
            //プレイヤー武器取得
            if (!saveWeapon_.expired())
            {
                if (!saveWeapon_.lock()->GetIsAttackAnim()) //攻撃が終わったらフラグを切る
                {
                    if (!invincibleSetFlag) //無敵時間を別に設定している場合はとばす
                        isInvincible_ = false;  //無敵終了
                }
            }
            else
            {
                if (!invincibleSetFlag)
                    isInvincible_ = false;
            }
        }
    }

    //無敵時間を設定していれば（ダメージとは別）
    {
        if (invincibleSetFlag)
        {
            invincibleSetTimer -= elapsedTime;
            if (invincibleSetTimer <= 0)
            {
                isInvincible_ = false;
                invincibleSetFlag = false;
            }
        }
    }

    //ダメージアニメーション判定
    if (!GetGameObject()->GetComponent<AnimationCom>()->IsPlayAnimation() && isAnimDamage_)
    {
        isAnimDamage_ = false;
    }

}

// GUI描画
void CharacterStatusCom::OnGUI()
{
    ImGui::InputInt("hp", &hp_);

    bool damage = isAnimDamage_;
    ImGui::Checkbox("isDamageAnimation", &damage);
    damage = isInvincible_;
    ImGui::Checkbox("isInvincible", &damage);
}


//ダメージ
void CharacterStatusCom::OnDamage(std::shared_ptr<WeaponCom> weapon, int minasHP, DirectX::XMFLOAT3& power, ATTACK_SPECIAL_TYPE attackType, float invincibleNonDamage)
{
    //無敵時間を設定されていれば
    if (invincibleNonDamage > 0)
    {
        SetInvincibleNonDamage(invincibleNonDamage);
    }

    saveWeapon_ = weapon;
    isInvincible_ = true;
    isFrameDamage_ = true;
    isAnimDamage_ = true;
    damageType_ = attackType;
    hp_ -= minasHP;

    if (!isAttackNonMove_|| damageType_== ATTACK_SPECIAL_TYPE::UNSTOP)  //攻撃による移動するか判定
        GetGameObject()->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);   //移動
}
