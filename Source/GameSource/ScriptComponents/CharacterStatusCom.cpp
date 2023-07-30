#include "CharacterStatusCom.h"

#include "Components\MovementCom.h"

// �J�n����
void CharacterStatusCom::Start()
{

}

// �X�V����
void CharacterStatusCom::Update(float elapsedTime)
{
    //�_���[�W
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

// GUI�`��
void CharacterStatusCom::OnGUI()
{

}


//�_���[�W
void CharacterStatusCom::OnDamage(DirectX::XMFLOAT3& power)
{
    isDamage_ = true;

    GetGameObject()->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);
}
