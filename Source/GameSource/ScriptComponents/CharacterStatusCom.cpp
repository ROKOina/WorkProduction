#include "CharacterStatusCom.h"

#include "Components\MovementCom.h"

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
        if (oneFrameJudge)
        {
            isFrameDamage_ = false;
            oneFrameJudge = false;
        }
        else
            oneFrameJudge = true;
    }

    //ダメージ
    if (isDamage_)
    {
        damageTimer_ += elapsedTime;
        if (damageTimer_ > damageInvincibleTime_)
        {
            isDamage_ = false;
            damageTimer_ = 0;
        }
    }
}

// GUI描画
void CharacterStatusCom::OnGUI()
{

}


//ダメージ
void CharacterStatusCom::OnDamage(DirectX::XMFLOAT3& power)
{
    isDamage_ = true;
    isFrameDamage_ = true;

    GetGameObject()->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);
}
