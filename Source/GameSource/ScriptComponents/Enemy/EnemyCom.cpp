#include "EnemyCom.h"
#include "Components\AnimationCom.h"
#include "Components\TransformCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimatorCom.h"

#include "../Player/PlayerCom.h"

//�A�j���[�V�������X�g
enum AnimationEnemy
{
    WALK,
    RUN,
    RUN_BACK,
    JUMP,
    IDEL,
    KICK,
};


// �J�n����
void EnemyCom::Start()
{
    //�A�j���[�V����������
    AnimationInitialize();
}

// �X�V����
void EnemyCom::Update(float elapsedTime)
{
    //���
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

    //���ŃW���X�g��𓖂��蔻���؂�A�A�^�b�N�����蔻������Ă���
    DirectX::XMFLOAT3 pos;
    //�W���X�g
    std::shared_ptr<GameObject> justChild = GetGameObject()->GetChildFind("picolaboAttackJust");
    if (GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("kick_left_just", pos)) {
        justChild->GetComponent<Collider>()->SetEnabled(true);
        //justChild->transform_->SetWorldPosition(pos);
    }
    else
    {
        justChild->GetComponent<Collider>()->SetEnabled(false);
    }

    //�W���X�g�����蔻��
    std::vector<HitObj> hitObj = justChild->GetComponent<Collider>()->OnHitGameObject();
    for (auto& h : hitObj)
    {
        if (h.gameObject->GetComponent<Collider>()->GetMyTag() != COLLIDER_TAG::Player)continue;
        int i = 0;
    }

    //�A�^�b�N�A�j���[�V�����C�x���g�擾
    //�C�x���g���͎q�̃A�^�b�N�I�u�W�F�N�g���I����
    std::shared_ptr<GameObject> attackChild = GetGameObject()->GetChildFind("picolaboAttack");
    if (GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("kick_left", pos)) {
        attackChild->GetComponent<Collider>()->SetEnabled(true);
        attackChild->transform_->SetWorldPosition(pos);
    }
    else
    {
        attackChild->GetComponent<Collider>()->SetEnabled(false);
    }

    //�Ƃ肠�����߂��ɂ�����U��(��)
    DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 myPos = GetGameObject()->transform_->GetWorldPosition();
    if (playerPos.x * playerPos.x - myPos.x * myPos.x < 5 * 5)
    {
        if (playerPos.z * playerPos.z - myPos.z * myPos.z < 5 * 5)
        {
            //�A�j���[�^�[
            std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
            animator->SetTriggerOn("kick");
        }
    }

    ////�A�^�b�N�����蔻��
    //hitObj = attackChild->GetComponent<Collider>()->OnHitGameObject();
    //for (auto& h : hitObj)
    //{
    //    if (h.gameObject->GetComponent<Collider>()->GetMyTag() != COLLIDER_TAG::Player)continue;
    //    //�v���C���[�Ƀ_���[�W
    //    std::shared_ptr<GameObject> player = h.gameObject;
    //    if (player->GetComponent<PlayerCom>()->GetIsInvincible())continue;
    //    player->GetComponent<MovementCom>()->AddNonMaxSpeedForce({ 0, 0, -30 });
    //    player->GetComponent<PlayerCom>()->OnDamage();
    //}
}

// GUI�`��
void EnemyCom::OnGUI()
{

}

//�_���[�W
void EnemyCom::OnDamage(DirectX::XMFLOAT3& power)
{
    isDamage_ = true;

    GetGameObject()->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);
}

//�A�j���[�V�����������ݒ�
void EnemyCom::AnimationInitialize()
{
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //�����̃A�j���[�V����
    animator->SetFirstTransition(AnimationEnemy::IDEL);
    animator->SetLoopAnimation(AnimationEnemy::IDEL, true);

    //�A�j���[�V�����p�����[�^�[�ǉ�
    animator->AddTriggerParameter("kick");

    animator->AddAnimatorTransition(IDEL, KICK);
    animator->SetTriggerTransition(IDEL, KICK, "kick");

    animator->AddAnimatorTransition(KICK, IDEL, true);
}
