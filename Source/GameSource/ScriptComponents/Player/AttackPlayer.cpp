#include "AttackPlayer.h"

#include "PlayerCom.h"
#include "Components\ColliderCom.h"
#include "Components\TransformCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "SystemStruct\QuaternionStruct.h"
#include "../Weapon\WeaponCom.h"
#include "Graphics/Graphics.h"
#include "Input/Input.h"


void AttackPlayer::Update(float elapsedTime)
{
    AttackInputUpdate(elapsedTime);

    AttackMoveUpdate(elapsedTime);
}

//�U�����͏���
void AttackPlayer::AttackInputUpdate(float elapsedTime)
{
    if (!isNormalAttack_)return;

    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //�_�b�V���R���{���͂ɂ���
    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH && comboAttackCount_ == 2)
    {
        if (EndAttackState())
        {
            animator->SetIsStop(false);
            Graphics::Instance().SetWorldSpeed(0.3f);
            player_.lock()->GetMovePlayer()->isDash_ = true;
        }
    }

    int currentAnimIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    //�R���{�㏈��
    {
        static int oldCount = 0;
        static int oldAnim = 0;
        if (comboAttackCount_ > 0) {
            //�O�t���[���̃R���{�ƃA�j���[�V����������
            //�R���{�I��������
            if (oldCount == comboAttackCount_)
            {
                if (oldAnim != currentAnimIndex)
                {
                    //�R���{�I��
                    AttackFlagEnd();

                    player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
                }
            }
        }
        oldCount = comboAttackCount_;
        oldAnim = currentAnimIndex;
    }

    //�_�b�V���A�^�b�N���̏���
    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH)
    {
        if (DoComboAttack())
        {
            //�_�b�V���L����
            player_.lock()->GetMovePlayer()->isDash_ = true;
            //�������Ă�����
            if (OnHitEnemy() && ComboReadyEnemy())
            {
                Graphics::Instance().SetWorldSpeed(0.3f);
                player_.lock()->GetMovePlayer()->dashCoolTimer_ = 0;
            }
        }
    }


    //�����p���U���A�j���[�V����
    if (animFlagName_.size() > 0)
    {
        animator->SetTriggerOn(animFlagName_);
        comboAttackCount_++;
        animFlagName_ = "";
    }
    else
    {
        GamePad& gamePad = Input::Instance().GetGamePad();
        if (gamePad.GetButtonDown() & GamePad::BTN_X)   //��
        {

            if (comboAttackCount_ == 0)
            {
                //�_�b�V���U��������
                if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
                {
                    //�_�b�V����R���{�̂��߃_�b�V�����o���Ȃ�����
                    player_.lock()->GetMovePlayer()->isDash_ = false;
                    animator->SetTriggerOn("triangleDash");
                    player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_DASH);
                    DashAttack(1);

                    comboAttackCount_++;

                    //�_�b�V���I���t���O
                    player_.lock()->GetMovePlayer()->DashEndFlag();
                }


                return;
            }
        }

        if (gamePad.GetButtonDown() & GamePad::BTN_Y)   //��
        {
            //���̏�ԂőJ�ڂ�ς���
            if (comboAttackCount_ == 0)
            {
                //�_�b�V���U��������
                if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
                {
                    return;
                }
                else
                {
                    animator->SetTriggerOn("squareIdle");
                    player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);
                    NormalAttack();
                }

                comboAttackCount_++;

                //�_�b�V���I���t���O
                player_.lock()->GetMovePlayer()->DashEndFlag();

                return;
            }

            if (comboAttackCount_ >= 3)return;

            if (DoComboAttack())
            {
                if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK)
                {
                    animator->SetTriggerOn("square");
                    NormalAttack();
                    comboAttackCount_++;

                    player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);
                    return;
                }
            }

            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
            {
                if (animator->GetIsStop())return;
                animator->SetTriggerOn("squareIdle");

                //����
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);
                Graphics::Instance().SetWorldSpeed(1.0f);
                comboAttackCount_ = 1;

                return;
            }

            ////�A�j���[�V�����Ǝ��̍U���̎�ނ�I��
            //nextAnimName_ = "squareIdle";
        }
    }

    //�R���{������
    if (comboAttackCount_ > 0)
    {
        player_.lock()->GetMovePlayer()->isInputMove_ = false;
        player_.lock()->GetMovePlayer()->isInputTrun_ = false;
    }

}

//�U����������
void AttackPlayer::AttackMoveUpdate(float elapsedTime)
{
    //state������
    if (EndAttackState())state_ = -1;

    //�U���̎�ނɂ���ē�����ς���
    switch (attackFlagState_)
    {
    case ATTACK_FLAG::Normal:
        if (NormalAttackUpdate(elapsedTime) == ATTACK_CODE::EnterAttack)
            attackFlagState_ = ATTACK_FLAG::Null;
        break;
    case ATTACK_FLAG::Dash:
        if (DashAttackUpdate(elapsedTime) == ATTACK_CODE::EnterAttack)
            attackFlagState_ = ATTACK_FLAG::Null;
        break;
    }


    //�U�����������Ă��邩����
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
    //����X�e�[�^�X������
    std::shared_ptr<WeaponCom> weapon = playerObj->GetChildFind("greatSword")->GetComponent<WeaponCom>();

    //���ł��U�����������Ă���Ȃ玟�̍U��������܂ŁAtrue�ɂ���
    if (weapon->GetOnHit())
    {
        onHitEnemy_ = true;
    }

}


void AttackPlayer::NormalAttack()
{
    state_ = 0;
    onHitEnemy_ = false;    //�U�������͂��ꂽ��false��
    attackFlagState_ = ATTACK_FLAG::Normal;
}

int AttackPlayer::NormalAttackUpdate(float elapsedTime)
{
    switch (state_)
    {
    case 0:
        //�͈͓��ɓG�͂��邩
        enemyCopy_ = AssistGetNearEnemy();
        if (!enemyCopy_)
        {
            state_ = ATTACK_CODE::EnterAttack;
            break;
        }
        state_++;
        break;

    case 1:
        //��]
        if (ForcusEnemy(elapsedTime, enemyCopy_, 10))
            state_++;
       break;

    case 2:
        //�ڋ�
        if (ApproachEnemy(enemyCopy_, 1.5f))
            state_ = ATTACK_CODE::EnterAttack;
        break;
    }

    return state_;
}

void AttackPlayer::DashAttack(int comboNum)
{
    if (comboNum == 1)
        state_ = 0;
    if (comboNum == 2)
        state_ = 10;
    onHitEnemy_ = false;    //�U�������͂��ꂽ��false��
    attackFlagState_ = ATTACK_FLAG::Dash;
}

int AttackPlayer::DashAttackUpdate(float elapsedTime)
{
    switch (state_)
    {
        //�R���{�P
    case 0:
        //�͈͓��ɓG�͂��邩
        enemyCopy_ = AssistGetNearEnemy();
        if (!enemyCopy_)
        {
            state_ = ATTACK_CODE::EnterAttack;
            break;
        }
        state_++;
        break;

    case 1:
        //��]
        if (ForcusEnemy(elapsedTime, enemyCopy_, 10))
            state_ = ATTACK_CODE::EnterAttack;
        break;

        //�R���{�Q
    case 10:
        //�ڋ�
        if (!enemyCopy_)
        {
            state_ = ATTACK_CODE::EnterAttack;
            break;
        }

        if (ApproachEnemy(enemyCopy_, 1.5f, 2))
            state_ = ATTACK_CODE::EnterAttack;
        break;
    }

    return state_;
}


//�R���{�o���邩����
bool AttackPlayer::DoComboAttack()
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    std::shared_ptr<AnimationCom> animCom = playerObj->GetComponent<AnimationCom>();
    for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
    {
        //����AutoCollision�Ȃ�
        if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;
        //�A�j���[�V�����C�x���g�̃G���h�t���[����Ȃ�R���{�\
        if (!animCom->GetCurrentAnimationEventIsEnd(animEvent.name.c_str()))continue;

        return true;
    }
    return false;
}


//�A�V�X�g�͈͂����G���ċ߂��G��Ԃ�
std::shared_ptr<GameObject> AttackPlayer::AssistGetNearEnemy()
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
    std::shared_ptr<GameObject> enemyNearObj;   //��ԋ߂��G������

    //�A�V�X�g����擾
    std::shared_ptr<Collider> assistColl = playerObj->GetChildFind("attackAssist")->GetComponent<Collider>();
    std::vector<HitObj> hitGameObj = assistColl->OnHitGameObject();
    for (auto& hitObj : hitGameObj)
    {
        if (COLLIDER_TAG::Enemy == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
        {
            //�ŏ��͂��̂܂ܓ����
            if (!enemyNearObj)enemyNearObj = hitObj.gameObject;
            //��ԋ߂��G��������
            else
            {
                DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemyNearObj->transform_->GetWorldPosition());
                DirectX::XMVECTOR EN = DirectX::XMLoadFloat3(&hitObj.gameObject->transform_->GetWorldPosition());
                DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());

                DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(E, P);
                DirectX::XMVECTOR PEN = DirectX::XMVectorSubtract(EN, P);

                float lenE = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
                float lenEN = DirectX::XMVectorGetX(DirectX::XMVector3Length(PEN));
                if (lenE > lenEN)enemyNearObj = hitObj.gameObject;
            }
        }
    }

    //�G�����Ȃ��ꍇreturn
    return enemyNearObj;
}

//�G�ɐڋ߂���( true:�ڋߊ����@false:�ڋߒ� )
bool AttackPlayer::ApproachEnemy(std::shared_ptr<GameObject> enemy, float dist, float speed)
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());
    DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->transform_->GetWorldPosition());

    DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(E, P);
    //�������k�܂��true
    if (dist > DirectX::XMVectorGetX(DirectX::XMVector3Length(PE)))
    {
        playerObj->GetComponent<MovementCom>()->ZeroNonMaxSpeedVelocity();
        return true;
    }

    //�������l�߂�
    PE = DirectX::XMVector3Normalize(PE);
    DirectX::XMFLOAT3 vec;
    DirectX::XMStoreFloat3(&vec, DirectX::XMVectorScale(PE, speed));
    
    //�v���C���[�ړ�
    playerObj->GetComponent<MovementCom>()->AddNonMaxSpeedForce(vec);
    return false;
}


//�G�̕����։�]���� ( true:���� )
bool AttackPlayer::ForcusEnemy(float elapsedTime, std::shared_ptr<GameObject> enemy, float rollSpeed)
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());
    DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->transform_->GetWorldPosition());

    //�v���C���[����G�l�~�[�̕����̃N�H�[�^�j�I�����擾
    DirectX::XMFLOAT3 peR;
    DirectX::XMStoreFloat3(&peR, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(E, P)));
    QuaternionStruct peQ;
    peQ = QuaternionStruct::LookRotation(peR);

    DirectX::XMVECTOR PEQ = DirectX::XMLoadFloat4(&peQ.dxFloat4);
    DirectX::XMVECTOR PQ = DirectX::XMLoadFloat4(&playerObj->transform_->GetRotation());

    float rot = DirectX::XMVectorGetX(DirectX::XMQuaternionDot(PQ, PEQ));

    if (1 - rot * rot < 0.01f)return true;

    //�v���C���[��������A�G�l�~�[�̕����֕�Ԃ���
    PQ = DirectX::XMQuaternionSlerp(PQ, PEQ, rollSpeed * elapsedTime);

    QuaternionStruct playerQuaternion;
    DirectX::XMStoreFloat4(&playerQuaternion.dxFloat4, PQ);

    playerObj->transform_->SetRotation(playerQuaternion);

    return false;
}

//�����I�ɍU�����I��点��i�W�����v�����j
void AttackPlayer::AttackFlagEnd()
{
    comboAttackCount_ = 0;
    player_.lock()->GetMovePlayer()->isInputMove_ = true;
    player_.lock()->GetMovePlayer()->isInputTrun_ = true;
    ResetState();
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetAnimationSpeedOffset(1);

    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH ||
        player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
    {
        Graphics::Instance().SetWorldSpeed(1.0f);
    }

    //playerStatus_ = PLAYER_STATUS::IDLE;
}
