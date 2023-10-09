#include "CharacterStatusCom.h"

#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "GameSource/ScriptComponents/Player/PlayerCom.h"

#include <imgui.h>

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
        //�P�t���[����̏���
        if (oneFrameJudge)
        {
            isFrameDamage_ = false;
            oneFrameJudge = false;
            isAnimDamage_ = true;   //�A�j���[�V������
            damageType_ = ATTACK_SPECIAL_TYPE::NORMAL;
        }
        else
            oneFrameJudge = true;
    }

    //�_���[�W
    if (isDamage_)
    {
        //�v���C���[�̔�e����
        if (GetGameObject()->GetComponent<PlayerCom>())
        {
            damageTimer_ += elapsedTime;
            if (damageTimer_ > damageInvincibleTime_)
            {
                isDamage_ = false;
                damageTimer_ = 0;
            }
        }
        else //�G�̔�e����
        {
            //�v���C���[����擾
            std::shared_ptr<GameObject> playerWeapon = GameObjectManager::Instance().Find("Candy");
            if(!playerWeapon->GetComponent<WeaponCom>()->GetIsAttackAnim()) //�U�����I�������t���O��؂�
                isDamage_ = false;
        }
    }

    //�_���[�W�A�j���[�V��������
    if (!GetGameObject()->GetComponent<AnimationCom>()->IsPlayAnimation() && isAnimDamage_)
    {
        isAnimDamage_ = false;
    }

}

// GUI�`��
void CharacterStatusCom::OnGUI()
{
    ImGui::InputInt("hp", &hp_);
}


//�_���[�W
void CharacterStatusCom::OnDamage(DirectX::XMFLOAT3& power, ATTACK_SPECIAL_TYPE attackType)
{
    isDamage_ = true;
    isFrameDamage_ = true;
    damageType_ = attackType;
    hp_ -= 1;

    if (!isAttackNonMove_)
        GetGameObject()->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);
}
