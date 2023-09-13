#include "CharacterStatusCom.h"

#include "Components\MovementCom.h"

// �J�n����
void CharacterStatusCom::Start()
{

}

//�X�V�O
void CharacterStatusCom::UpdateStart()
{
}

// �X�V����
void CharacterStatusCom::Update(float elapsedTime)
{
    //1�t���[������isFrameDamage_��true�ɂ���
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
    isFrameDamage_ = true;

    GetGameObject()->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);
}
