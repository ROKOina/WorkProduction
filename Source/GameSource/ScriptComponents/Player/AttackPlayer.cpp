#include "AttackPlayer.h"

#include "PlayerCom.h"
#include "Components\ColliderCom.h"
#include "Components\RendererCom.h"
#include "Components\TransformCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "SystemStruct\QuaternionStruct.h"
#include "../Weapon\WeaponCom.h"
#include "Graphics/Graphics.h"
#include "Input/Input.h"
#include <imgui.h>


void AttackPlayer::Update(float elapsedTime)
{
    //�R���{�p���m�F����
    ComboJudge();

    //���͂����ď���������
    if (IsAttackInput(elapsedTime))
    {
        switch (attackKey_)
        {
        case AttackPlayer::ATTACK_KEY::SQUARE:
            SquareInput();
            break;
        case AttackPlayer::ATTACK_KEY::TRIANGLE:
            TriangleInput();
            break;
        case AttackPlayer::ATTACK_KEY::DASH:
            DashInput();
            break;
        case AttackPlayer::ATTACK_KEY::NULL_KEY:
            break;
        }
    }

    //�R���{����
    ComboProcess(elapsedTime);

    //�U�����̓���
    AttackMoveUpdate(elapsedTime);
}

void AttackPlayer::OnGui()
{
    ImGui::InputInt("comboCount", &comboAttackCount_, ImGuiInputTextFlags_ReadOnly);
}

//�R���{�p���m�F����
void AttackPlayer::ComboJudge()
{
    //���݂̃A�j���C���f�b�N�X
    int currentAnimIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    //�R���{�p�����菈��
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

                    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

                    //�v���C���[�X�e�[�^�X�̍X�V
                    if (playerObj->GetComponent<MovementCom>()->OnGround())
                        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
                    else
                        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP);

                }
            }
        }
        oldCount = comboAttackCount_;
        oldAnim = currentAnimIndex;
    }

    //�R���{������
    if (comboAttackCount_ > 0)
    {
        player_.lock()->GetMovePlayer()->SetIsInputMove(false);
        player_.lock()->GetMovePlayer()->SetIsInputTurn(false);
    }
}

//���͏���ۑ�
bool AttackPlayer::IsAttackInput(float elapsedTime)
{
    attackKey_ = ATTACK_KEY::NULL_KEY;

    if (!isNormalAttack_)return false;

    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //�����p���U���A�j���[�V����
    if (animFlagName_.size() > 0)
    {
        animator->SetTriggerOn(animFlagName_);
        comboAttackCount_++;
        animFlagName_ = "";

        //���̂Ƃ���A�����ŏ����͏I���ɂ��Ă���
        return false;
    }
    else
    {
        GamePad& gamePad = Input::Instance().GetGamePad();
        if (gamePad.GetButtonDown() & GamePad::BTN_Y)   //��
        {
            attackKey_ = ATTACK_KEY::SQUARE;
            return true;
        }

        if (gamePad.GetButtonDown() & GamePad::BTN_X)   //��
        {
            attackKey_ = ATTACK_KEY::TRIANGLE;
            return true;
        }

        if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER)   //DASH
        {
            attackKey_ = ATTACK_KEY::DASH;
            return true;
        }
    }

    return false;
}

//�����͎�����
void AttackPlayer::SquareInput()
{
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //���̏�ԂőJ�ڂ�ς���
    if (comboAttackCount_ == 0)
    {
        std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
        if (!playerObj->GetComponent<MovementCom>()->OnGround())
        {
            ////�W�����v���̎�
            //if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP)
            //{
            animator->SetTriggerOn("squareJump");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);

                //return;
            //}
        }
        else
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
        }

        comboAttackCount_++;

        //�_�b�V���I���t���O
        player_.lock()->GetMovePlayer()->DashEndFlag();

        return;
    }

    if (comboAttackCount_ >= 3)return;

    if (DoComboAttack())
    {
        std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
        if (!playerObj->GetComponent<MovementCom>()->OnGround())
        {
            animator->SetTriggerOn("square");
            comboAttackCount_++;

            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);
            return;
        }
        else
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
}

//�����͎�����
void AttackPlayer::TriangleInput()
{
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //�W�����v���؂肨�낵
    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP
        || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP_BACK_DASH
        || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP_DASH)
    {
        animator->ResetParameterList();
        animator->SetTriggerOn("triangleJumpDown");
        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);

        player_.lock()->GetGameObject()->GetComponent<MovementCom>()->ZeroVelocity();

        //�؂肨�낵���͈ړ��s��
        player_.lock()->GetMovePlayer()->SetIsInputMove(false);
        player_.lock()->GetMovePlayer()->SetIsInputTurn(false);
        player_.lock()->GetMovePlayer()->DashEndFlag();
        isJumpFall_ = true;
    }


    if (comboAttackCount_ == 0)
    {
        //�_�b�V���U��������
        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
        {
            //�_�b�V����R���{�̂��߃_�b�V�����o���Ȃ�����
            player_.lock()->GetMovePlayer()->SetIsDash(false);
            animator->SetTriggerOn("triangleDash");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_DASH);
            DashAttack(1);

            comboAttackCount_++;

            //�_�b�V���I���t���O
            player_.lock()->GetMovePlayer()->DashEndFlag();
        }
        else if(player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::IDLE
            || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::MOVE)
        {
            animator->SetTriggerOn("triangleJump");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);

            //�W�����v�J�E���g���炷
            player_.lock()->GetMovePlayer()->SetJumpCount(player_.lock()->GetMovePlayer()->GetJumpCount() - 1);

            comboAttackCount_++;
        }

        return;
    }


}

//Dash���͎�����
void AttackPlayer::DashInput()
{
    //�_�b�V����R���{
    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH)
    {
        if (DoComboAttack())
        {
            if (OnHitEnemy() && ComboReadyEnemy())
            {
                Graphics::Instance().SetWorldSpeed(1.0f);
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::DASH);

                //�A�j���[�^�[
                std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

                //���̍U���A�j���[�V�����t���O��n��
                animFlagName_ = "dashComboWait";
                animator->ResetParameterList();

                DashAttack(2);
                animator->SetIsStop(true);

                //��x������
                player_.lock()->GetGameObject()->GetComponent<RendererCom>()->SetEnabled(false);

                return;
            }
        }
    }
}

//�R���{����
void AttackPlayer::ComboProcess(float elapsedTime)
{
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //�_�b�V���R���{����
    {
        //�_�b�V���A�^�b�N���̏���(�_�b�V���ɑ���)
        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH)
        {
            if (DoComboAttack())
            {
                //�������Ă�����
                if (OnHitEnemy() && ComboReadyEnemy())
                {
                    Graphics::Instance().SetWorldSpeed(0.3f);
                }
            }
        }

        //�_�b�V���R���{���͂ɂ���
        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH && comboAttackCount_ == 2)
        {
            if (EndAttackState())
            {
                //��������
                player_.lock()->GetGameObject()->GetComponent<RendererCom>()->SetEnabled(true);

                animator->SetIsStop(false);
                Graphics::Instance().SetWorldSpeed(0.3f);
                player_.lock()->GetMovePlayer()->SetIsDash(true);
            }
        }
    }

    //�W�����v�U���X�V����
    {
        std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

        //�d�͐ݒ�
        static bool isSetGravity = false;
        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP)  //�󒆂̎�
        {
            playerObj->GetComponent<MovementCom>()->ZeroVelocityY();
            playerObj->GetComponent<MovementCom>()->SetGravity(0);
            isSetGravity = true;
        }
        else if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP_FALL)    //�؂艺�낵�U����
        {
            playerObj->GetComponent<MovementCom>()->SetGravity(GRAVITY_FALL);
            isSetGravity = true;
        }
        else
        {
            if (isSetGravity)   //�����ŕς�����������������悤��
            {
                playerObj->GetComponent<MovementCom>()->SetGravity(GRAVITY_NORMAL);
                isSetGravity = false;
            }
        }

        int index = playerObj->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();
        //�؂艺�낵����
        if (index == ANIMATION_PLAYER::JUMP_ATTACK_DOWN_DO)
        {
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP_FALL);
        }

        //�؂艺�낵�㏈��
        static bool isJumpAttackEnd = false;
        if (index == ANIMATION_PLAYER::JUMP_ATTACK_DOWN_END && isJumpFall_)
        {
            isJumpFall_ = false;
            player_.lock()->GetMovePlayer()->SetIsInputMove(false);
            isJumpAttackEnd = true;
        }
        else if (index != ANIMATION_PLAYER::JUMP_ATTACK_DOWN_END && isJumpAttackEnd)
        {
            player_.lock()->GetMovePlayer()->SetIsInputMove(true);
            isJumpAttackEnd = false;
        }
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
    player_.lock()->GetMovePlayer()->SetIsInputMove(true);
    player_.lock()->GetMovePlayer()->SetIsInputTurn(true);
    player_.lock()->GetMovePlayer()->SetIsDash(true);
    ResetState();
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetAnimationSpeedOffset(1);

    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH ||
        player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
    {
        Graphics::Instance().SetWorldSpeed(1.0f);
    }
}
