#include "CharacterStatusCom.h"

#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "GameSource/ScriptComponents/Player/PlayerCom.h"

#include <imgui.h>

// 開始処理
void CharacterStatusCom::Start()
{

}

//更新前
void CharacterStatusCom::UpdateStart()
{
}

// 更新処理
void CharacterStatusCom::Update(float elapsedTime)
{
    //1フレームだけisFrameDamage_をtrueにする
    if (isFrameDamage_)
    {
        //１フレーム後の処理
        if (oneFrameJudge)
        {
            isFrameDamage_ = false;
            oneFrameJudge = false;
            isAnimDamage_ = true;   //アニメーション中
            damageType_ = ATTACK_SPECIAL_TYPE::NORMAL;
        }
        else
            oneFrameJudge = true;
    }

    //ダメージ
    if (isDamage_)
    {
        //プレイヤーの被弾時間
        if (GetGameObject()->GetComponent<PlayerCom>())
        {
            damageTimer_ += elapsedTime;
            if (damageTimer_ > damageInvincibleTime_)
            {
                isDamage_ = false;
                damageTimer_ = 0;
            }
        }
        else //敵の被弾時間
        {
            //プレイヤー武器取得
            std::shared_ptr<GameObject> playerWeapon = GameObjectManager::Instance().Find("Candy");
            if(!playerWeapon->GetComponent<WeaponCom>()->GetIsAttackAnim()) //攻撃が終わったらフラグを切る
                isDamage_ = false;
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
}


//ダメージ
void CharacterStatusCom::OnDamage(DirectX::XMFLOAT3& power, ATTACK_SPECIAL_TYPE attackType)
{
    isDamage_ = true;
    isFrameDamage_ = true;
    damageType_ = attackType;
    hp_ -= 1;

    if (!isAttackNonMove_)
        GetGameObject()->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);
}
