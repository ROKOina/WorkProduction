#include "CharacterStatusCom.h"

#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "GameSource/ScriptComponents/Player/PlayerCom.h"

#include <imgui.h>

// �J�n����
void CharacterStatusCom::Start()
{

}

// �X�V����
void CharacterStatusCom::Update(float elapsedTime)
{
    //1�t���[������isFrameDamage_��true�ɂ���
    if (isFrameDamage_)
    {
        //�P�t���[����̏���
        if (oneFrameJudge_)
        {
            isFrameDamage_ = false;
            oneFrameJudge_ = false;
            isAnimDamage_ = true;   //�A�j���[�V������
            damageType_ = ATTACK_SPECIAL_TYPE::NORMAL;
        }
        else
            oneFrameJudge_ = true;
    }

    //�_���[�W
    if (isInvincible_)
    {
        //�v���C���[�̔�e
        if (GetGameObject()->GetComponent<PlayerCom>())
        {
            if (!isAnimDamage_) //�_���[�W�A�j���[�V������
            {
                damageTimer_ += elapsedTime;
                if (damageTimer_ > damageInvincibleTime_)
                {
                    if (!invincibleSetFlag) //���G���Ԃ�ʂɐݒ肵�Ă���ꍇ�͂Ƃ΂�
                        isInvincible_ = false;
                    damageTimer_ = 0;
                }
            }
        }
        else //�G�̔�e����
        {
            //�v���C���[����擾
            std::shared_ptr<GameObject> playerWeapon = GameObjectManager::Instance().Find("Candy");
            if (!playerWeapon->GetComponent<WeaponCom>()->GetIsAttackAnim()) //�U�����I�������t���O��؂�
            {
                if (!invincibleSetFlag) //���G���Ԃ�ʂɐݒ肵�Ă���ꍇ�͂Ƃ΂�
                    isInvincible_ = false;
            }
        }
    }

    //���G���Ԃ�ݒ肵�Ă���΁i�_���[�W�Ƃ͕ʁj
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

    bool damage = isAnimDamage_;
    ImGui::Checkbox("isDamageAnimation", &damage);
    damage = isInvincible_;
    ImGui::Checkbox("isInvincible", &damage);
}


//�_���[�W
void CharacterStatusCom::OnDamage(int minasHP, DirectX::XMFLOAT3& power, ATTACK_SPECIAL_TYPE attackType)
{
    isInvincible_ = true;
    isFrameDamage_ = true;
    isAnimDamage_ = true;
    damageType_ = attackType;
    hp_ -= minasHP;

    if (!isAttackNonMove_)  //�U���ɂ��ړ����邩����
        GetGameObject()->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);   //�ړ�
}
