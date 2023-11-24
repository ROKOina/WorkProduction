#include "AttackPlayer.h"

#include "PlayerCom.h"
#include "PlayerCameraCom.h"
#include "Components\ColliderCom.h"
#include "Components\RendererCom.h"
#include "Components\TransformCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "Components\CameraCom.h"
#include "SystemStruct\QuaternionStruct.h"
#include "Components/ParticleComManager.h"
#include "Components/ParticleSystemCom.h"
#include "../Weapon\WeaponCom.h"

#include "GameSource/Math/Mathf.h"
#include "Graphics/Graphics.h"
#include "Input/Input.h"
#include <imgui.h>

void AttackPlayer::Update(float elapsedTime)
{
    //�R���{�p���m�F����
    ComboJudge();
    
    //��s���͎���true
    bool isLeadInput = false;
    if (attackLeadInputKey_ != ATTACK_KEY::NULL_KEY)
    {
        if (DoComboAttack())
            isLeadInput = true;
    }


    //���͂����ď���������
    if (IsAttackInput(elapsedTime) || isLeadInput)
    {
        if (isLeadInput)
        {
            attackLeadInputKey_ = ATTACK_KEY::NULL_KEY;
        }

        switch (attackKey_)
        {
        case AttackPlayer::ATTACK_KEY::SQUARE:
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP_FALL)break;
            if (!isSquareAttack_)break;
            SquareInput();
            break;
        case AttackPlayer::ATTACK_KEY::TRIANGLE:
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP_FALL)break;
            TriangleInput();
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
    ImGui::Checkbox("isNormalAttack_", &isNormalAttack_);
    ImGui::Checkbox("isComboJudge_", &isComboJudge_);
    ImGui::InputInt("SquareCount", &comboSquareCount_, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputInt("TriangleCount", &comboTriangleCount_, ImGuiInputTextFlags_ReadOnly);
    ImGui::Checkbox("isSquareAttack", &isSquareAttack_);
    ImGui::Checkbox("isJumpSquareInput_", &isJumpSquareInput_);
    ImGui::DragFloat("jumpAttackComboWaitTimer_", &jumpAttackComboWaitTimer_);
}

//�R���{�p���m�F����
void AttackPlayer::ComboJudge()
{
    //���݂̃A�j���C���f�b�N�X
    int currentAnimIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    //�R���{�p�����菈��
    if (isComboJudge_)
    {
        static int oldSquareCount = 0;
        static int oldTriangleCount = 0;
        static int oldAnim = 0;
        if (comboSquareCount_ > 0 || comboTriangleCount_ > 0) {
            //�O�t���[���̃R���{�ƃA�j���[�V����������
            //�R���{�I��������
            if (oldSquareCount == comboSquareCount_ && oldTriangleCount == comboTriangleCount_ || comboJudgeEnd_)
            {
                if (oldAnim != currentAnimIndex || comboJudgeEnd_)
                {
                    //�R���{�I��
                    AttackFlagEnd();

                    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

                    //�󒆂��̔���
                    if (playerObj->GetComponent<MovementCom>()->OnGround())
                    {
                        //���n���ɋ󒆍U���������̉��
                        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP)
                            playerObj->GetComponent<AnimatorCom>()->SetTriggerOn("idle");

                        //�v���C���[�X�e�[�^�X�̍X�V
                        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
                    }
                    else
                    {
                        //�v���C���[�X�e�[�^�X�̍X�V
                        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP);
                    }

                    //���U���ł��Ȃ�����
                    if (isJumpSquareInput_)
                    {
                        isSquareAttack_ = false;
                        isJumpSquareInput_ = false;
                    }
                    comboJudgeEnd_ = false;
                }
            }
        }
        oldSquareCount = comboSquareCount_;
        oldTriangleCount = comboTriangleCount_;
        oldAnim = currentAnimIndex;
    }

    //�R���{�I��������ON�̏ꍇ�͐؂�
    if (comboJudgeEnd_)
    {
        comboJudgeEnd_ = false;
    }

    //�R���{������
    if (comboSquareCount_ > 0 || comboTriangleCount_ > 0)
    {
        player_.lock()->GetMovePlayer()->SetIsInputMove(false);
    }
}

//���͏���ۑ�
bool AttackPlayer::IsAttackInput(float elapsedTime)
{
    //��s���͎��͏㏑������
    if (attackLeadInputKey_ == ATTACK_KEY::NULL_KEY)
        attackKey_ = ATTACK_KEY::NULL_KEY;
    else
        attackKey_ = attackLeadInputKey_;

    if (!isNormalAttack_)return false;

    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    if (animFlagName_.size() > 0)
    {
        animator->ResetParameterList();
        animator->SetTriggerOn(animFlagName_);   

        comboSquareCount_++;
        animFlagName_ = "";
    }
    else
    {
        GamePad& gamePad = Input::Instance().GetGamePad();
        if (gamePad.GetButtonDown() & GamePad::BTN_X)   //��
        {
            attackKey_ = ATTACK_KEY::SQUARE;
            return true;
        }

        if (gamePad.GetButtonDown() & GamePad::BTN_Y)   //��
        {
            attackKey_ = ATTACK_KEY::TRIANGLE;
            return true;
        }
    }

    return false;
}

//�����͎�����
void AttackPlayer::SquareInput()
{
    //�U�����͐�s���͂ɕۑ�
    if (InAttackJudgeNow())
    {
        attackLeadInputKey_ = ATTACK_KEY::SQUARE;
        return;
    }

    //�A�j���C���f�b�N�X�ŁA�R���{�J�E���g���Z�b�g
    AttackComboCountReset();

    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //���̃R���{��ԂőJ�ڂ�ς���
    if (comboSquareCount_ == 0) //�R���{0
    {
        attackLeadInputKey_ = ATTACK_KEY::NULL_KEY;

        std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
        //�󒆍U��
        if (!playerObj->GetComponent<MovementCom>()->OnGround())
        {
            animator->ResetParameterList();
            animator->SetTriggerOn("squareJump");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);

            isJumpSquareInput_ = true;
        }
        //�n��U��
        else
        {
            isJumpSquareInput_ = false;
            //�_�b�V���U��������
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
            {
                animator->ResetParameterList();
                animator->SetTriggerOn("triangleDash");
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_DASH);
                NormalAttack(true);
            }
            else
            {
                animator->ResetParameterList();
                animator->SetTriggerOn("squareIdle");
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);
                NormalAttack(true);

                //�ŏ��Ɂ����͂�0�ɂ���
                comboTriangleCount_ = 0;
            }
        }

        comboSquareCount_++;

        //�_�b�V���I���t���O
        player_.lock()->GetMovePlayer()->DashEndFlag();

        return;
    }

    //�R���{1�`
    if (comboSquareCount_ >= 3)return;

    if (DoComboAttack())    //�R���{��t����
    {
        std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
        //�󒆍U��
        if (!playerObj->GetComponent<MovementCom>()->OnGround())
        {
            animator->ResetParameterList();
            animator->SetTriggerOn("square");
            NormalAttack();
            comboSquareCount_++;
            jumpAttackComboWaitTimer_ = 0;

            isJumpSquareInput_ = true;


            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);
            return;
        }
        //�n��U��
        else
        {
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK)  //�ʏ�U�����̃R���{
            {
                animator->SetTriggerOn("square");
                NormalAttack();
                comboSquareCount_++;

                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);
                return;
            }
            //�_�b�V���U��
            else if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH)
            {
                animator->SetTriggerOn("square");
                DashAttack(2);
                comboSquareCount_++;

                return;
            }
        }
    }
}

//�����͎�����
void AttackPlayer::TriangleInput()
{
    //�U�����͐�s���͂ɕۑ�
    if (InAttackJudgeNow())
    {
        attackLeadInputKey_ = ATTACK_KEY::TRIANGLE;
        return;
    }

    //�A�j���C���f�b�N�X�ŁA�R���{�J�E���g���Z�b�g
    AttackComboCountReset();

    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    //�v���C���[�X�e�[�^�X�擾
    PlayerCom::PLAYER_STATUS playerStatus = player_.lock()->GetPlayerStatus();

    //�W�����v������
    {
        //�W�����v���؂肨�낵
        if ((playerStatus == PlayerCom::PLAYER_STATUS::JUMP
            || playerStatus == PlayerCom::PLAYER_STATUS::JUMP_BACK_DASH
            || playerStatus == PlayerCom::PLAYER_STATUS::JUMP_DASH
            || playerStatus == PlayerCom::PLAYER_STATUS::ATTACK_JUMP)
            && comboSquareCount_ <= 3
            )
        {
            //�R���{���Ȃ�return
            if (playerStatus == PlayerCom::PLAYER_STATUS::ATTACK_JUMP)
                if (!DoComboAttack())//�R���{��t����
                    return;

            animator->ResetParameterList();
            animator->SetTriggerOn("triangleJumpDown");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);
            comboSquareCount_ = 10;
            isJumpSquareInput_ = false;
            isComboJudge_ = false;
            jumpAttackComboWaitTimer_ = 0;

            player_.lock()->GetGameObject()->GetComponent<MovementCom>()->ZeroVelocity();

            //�؂肨�낵���͈ړ��s��
            player_.lock()->GetMovePlayer()->SetIsInputMove(false);
            player_.lock()->GetMovePlayer()->SetIsInputTurn(false);
            player_.lock()->GetMovePlayer()->DashEndFlag();
            isJumpFall_ = true;

            //�J����������
            player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>()->SetChangeRange(0.4f, FarRange);
        }
    }

    //�R���{�J�E���g�����čU����ς���
    if (comboSquareCount_ == 0) //�R���{0
    {
        attackLeadInputKey_ = ATTACK_KEY::NULL_KEY;

        //�؂�グ�U��
        if(player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::IDLE
            || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::MOVE
            || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH)
        {
            animator->ResetParameterList();
            animator->SetTriggerOn("triangleJump");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);
            NormalAttack();

            player_.lock()->GetMovePlayer()->DashEndFlag();

            //�W�����v�J�E���g���炷
            player_.lock()->GetMovePlayer()->SetJumpCount(player_.lock()->GetMovePlayer()->GetJumpCount() - 1);

            //�J����������
            player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>()->SetChangeRange(0.4f, FarRange);

            comboTriangleCount_++;
        }

        return;
    }
    else if (comboSquareCount_ <= 3&& comboTriangleCount_<3)
    {
        if (DoComboAttack() && player_.lock()->GetPlayerStatus() != PlayerCom::PLAYER_STATUS::ATTACK_DASH)
        {
            animator->ResetParameterList();
            animator->SetTriggerOn("triangle");

            NormalAttack();
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);

            //�p�[�e�B�N��
            GameObjectManager::Instance().Find("playerHandParticle")
                ->GetComponent<ParticleSystemCom>()->Restart();

            comboTriangleCount_++;
        }
    }


}


//�R���{����
void AttackPlayer::ComboProcess(float elapsedTime)
{
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //�W�����v�U���X�V����
    {
        std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

        //�R���{�ݒ�
        {
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP)
            {
                //�R���{��t���Ԑݒ�
                if (isJumpSquareInput_)
                {
                    std::shared_ptr<AnimationCom> animCom = playerObj->GetComponent<AnimationCom>();
                    if (jumpAttackComboWaitTimer_ <= 0
                        && attackKey_ == ATTACK_KEY::NULL_KEY)
                    {
                        for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
                        {
                            //����AutoCollision�Ȃ�
                            if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;
                            //�A�j���[�V�����C�x���g�̃G���h�t���[����Ȃ�
                            if (!animCom->GetCurrentAnimationEventIsEnd(animEvent.name.c_str()))continue;

                            jumpAttackComboWaitTimer_ = jumpAttackComboWaitTime_;
                            isComboJudge_ = false;
                        }
                    }
                }
            }
            else
            {
                isJumpSquareInput_ = false;
            }

            //�R���{��t���ԏ���
            if (jumpAttackComboWaitTimer_ > 0)
            {
                jumpAttackComboWaitTimer_ -= elapsedTime;
                if (jumpAttackComboWaitTimer_ <= 0)
                {
                    isComboJudge_ = true;
                    comboJudgeEnd_ = true;
                }
            }
        }

        //�d�͐ݒ�
        {
            static bool isSetGravity = false;
            //�A�j���[�V�����C�x���g�����ďd�͂�ݒ肷��
            if (player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("ZeroGravity", DirectX::XMFLOAT3())
                && player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP
                || jumpAttackComboWaitTimer_ > 0
                )
            {
                playerObj->GetComponent<MovementCom>()->ZeroVelocityY();
                playerObj->GetComponent<MovementCom>()->SetGravity(GRAVITY_ZERO);
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
        }

        int index = playerObj->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();
        //�؂艺�낵����
        {
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
                isComboJudge_ = true;
                comboJudgeEnd_ = true;
                player_.lock()->GetMovePlayer()->SetIsInputMove(true);
                isJumpAttackEnd = false;
            }
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


    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    //����擾
    std::shared_ptr<WeaponCom> weapon1 = playerObj->GetChildFind("Candy")->GetComponent<WeaponCom>();
    std::shared_ptr<WeaponCom> weapon2 = playerObj->GetChildFind("CandyCircle")->GetComponent<WeaponCom>();
    std::shared_ptr<WeaponCom> weapon3 = playerObj->GetChildFind("CandyStick")->GetComponent<WeaponCom>();

    //���ł��U�����������Ă���Ȃ玟�̍U��������܂ŁAtrue�ɂ���
    if (weapon1->GetOnHit()||weapon2->GetOnHit()||weapon3->GetOnHit())
    {
        if (!onHitEnemy_)
        {
            onHitEnemy_ = true;

            //�q�b�g���J�����X�g�b�v
            GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()->HitStop(0.1f);
        }
    }


    //�U�����̎�
    if (comboSquareCount_ > 0)
    {
        //��]�s�\
        player_.lock()->GetMovePlayer()->SetIsInputTurn(false);

        //�U���t���[���O������]�\��
        std::shared_ptr<AnimationCom> animCom = playerObj->GetComponent<AnimationCom>();
        for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
        {
            //����AutoCollision�Ȃ�
            if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;

            //�A�j���[�V�����C�x���g�̃X�^�[�g�t���[���O�Ȃ��]�\
            if (!animCom->GetCurrentAnimationEventIsStart(animEvent.name.c_str()))continue;

            //��]�\
            player_.lock()->GetMovePlayer()->SetIsInputTurn(true);
        }
        
        if (!isSquareDirection_)    //���U�����o���Ă��Ȃ��ꍇ
        {
            //���U���P
            if (animCom->GetCurrentAnimationEvent("AutoCollisionTriangleAttack01", DirectX::XMFLOAT3()))
            {
                SpawnCombo1();
            }
            //���U���Q
            if (animCom->GetCurrentAnimationEvent("AutoCollisionTriangleAttack02", DirectX::XMFLOAT3()))
            {
                SpawnCombo2();
            }
            //���U���R
            if (animCom->GetCurrentAnimationEvent("AutoCollisionTriangleAttack03", DirectX::XMFLOAT3()))
            {
                //�J����������
                player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>()->SetChangeRange(1.0f, FarRange);

                SpawnCombo3();
            }
        }
    }

    //���U�����o�X�V
    SquareAttackDirection(elapsedTime);

}

//�A�j���C���f�b�N�X�ŁA�R���{�J�E���g���Z�b�g
void AttackPlayer::AttackComboCountReset()
{
    //���݂̃A�j���C���f�b�N�X
    int currentAnimIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    //�W�����v�U�����i��
    if (currentAnimIndex == ANIMATION_PLAYER::JUMP_ATTACK_01)
    {
        comboSquareCount_ = 1;
    }

    //�_�b�V���U���I����
    if (currentAnimIndex == ANIMATION_PLAYER::BIGSWORD_COM2_02)
    {
        //�����Ƀ_�b�V���؂�I����Ă���ɂ���
        if (DoComboAttack())
            comboSquareCount_ = 0;
    }

    //���R�i�ڏI����
    if (currentAnimIndex == ANIMATION_PLAYER::TRIANGLE_ATTACK_03)
    {
        if (DoComboAttack())
        comboSquareCount_ = 0;
    }
}


void AttackPlayer::NormalAttack(bool middleAssist)
{
    state_ = 0;
    onHitEnemy_ = false;    //�U�������͂��ꂽ��false��
    isMiddleAssist_ = middleAssist; //�������A�V�X�g
    attackFlagState_ = ATTACK_FLAG::Normal;

    isSquareDirection_ = false;
}

int AttackPlayer::NormalAttackUpdate(float elapsedTime)
{
    if (enemyCopy_.expired() && state_ > 0)
    {
        state_ = ATTACK_CODE::EnterAttack;
        return state_;
    }

    switch (state_)
    {
    case 0:
        //�͈͓��ɓG�͂��邩
        enemyCopy_ = AssistGetNearEnemy();
        if (!enemyCopy_.lock()) //�G�����Ȃ��ꍇ
        {
            //�m�[�}���U���̎�
            if (isMiddleAssist_)
            {
                //�������ł��G��T��
                enemyCopy_ = AssistGetMediumEnemy();
                if (enemyCopy_.lock()) //�G�����Ȃ��ꍇ
                {
                    state_ = 5;
                    break;
                }
            }

            state_ = ATTACK_CODE::EnterAttack;
            break;
        }
        state_++;
        break;

        //�ߋ����A�V�X�g
    case 1:
        //��]
        if (ForcusEnemy(elapsedTime, enemyCopy_.lock(), 50))
            state_++;
        break;

    case 2:
        //�ڋ�
        if (ApproachEnemy(enemyCopy_.lock(), 1.5f, 3) || onHitEnemy_)
            state_ = ATTACK_CODE::EnterAttack;

        break;

        //�������A�V�X�g
    case 5:
    {
        //�A�j���[�^�[
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

        //�_�b�V���A�j���[�V�����Đ�
        animator->ResetParameterList();
        animator->SetTriggerOn("dash");
        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_DASH);
        isComboJudge_ = false;

        //�J����������
        player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>()->SetChangeRange(1.0f, FarRange);

        state_++;
    }
    break;
    case 6:
        //��]
        if (ForcusEnemy(elapsedTime, enemyCopy_.lock(), 50))
            state_++;
        break;

    case 7:
        //�ڋ�
        if (ApproachEnemy(enemyCopy_.lock(), 1.5f, 10))
        {
            isComboJudge_ = true;
            comboSquareCount_++;

            //�A�j���[�^�[
            std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

            //�_�b�V���A�j���[�V�����Đ�
            animator->ResetParameterList();
            animator->SetTriggerOn("triangleDash");

            state_ = ATTACK_CODE::EnterAttack;
        }
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
    if (enemyCopy_.expired() && state_ > 0)
    {
        state_ = ATTACK_CODE::EnterAttack;
        return state_;
    }

    switch (state_)
    {
    //    //�R���{�P
    //case 0:
    //    //�͈͓��ɓG�͂��邩
    //    enemyCopy_ = AssistGetNearEnemy();
    //    if (!enemyCopy_.lock())
    //    {
    //        state_ = ATTACK_CODE::EnterAttack;
    //        break;
    //    }
    //    state_++;
    //    break;

    //case 1:
    //    //��]
    //    if (ForcusEnemy(elapsedTime, enemyCopy_.lock(), 10))
    //        state_ = ATTACK_CODE::EnterAttack;
    //    break;

        //�R���{2
    case 10:
        //�ڋ�
        if (!enemyCopy_.lock())
        {
            state_ = ATTACK_CODE::EnterAttack;
            break;
        }

        if (ApproachEnemy(enemyCopy_.lock(), 1.5f, 10))
            state_ = ATTACK_CODE::EnterAttack;
        break;
    }

    return state_;
}


//�R���{�o���邩����
bool AttackPlayer::DoComboAttack()
{
    //�W�����v�U�����R���{��t���Ԃ�true��
    if (jumpAttackComboWaitTimer_ > 0)
        return true;

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

//�U�����蒆������
bool AttackPlayer::InAttackJudgeNow()
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    std::shared_ptr<AnimationCom> animCom = playerObj->GetComponent<AnimationCom>();
    for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
    {
        //����AutoCollision�Ȃ�
        if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;
        //�A�j���[�V�����C�x���g�O�Ȃ�false
        if (!animCom->GetCurrentAnimationEvent(animEvent.name.c_str(), DirectX::XMFLOAT3()))continue;

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
    std::shared_ptr<Collider> assistColl = playerObj->GetChildFind("attackAssistNear")->GetComponent<Collider>();
    std::vector<HitObj> hitGameObj = assistColl->OnHitGameObject();
    for (auto& hitObj : hitGameObj)
    {
        if (COLLIDER_TAG::Enemy == hitObj.gameObject.lock()->GetComponent<Collider>()->GetMyTag())
        {
            //�ŏ��͂��̂܂ܓ����
            if (!enemyNearObj)enemyNearObj = hitObj.gameObject.lock();
            //��ԋ߂��G��������
            else
            {
                DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemyNearObj->transform_->GetWorldPosition());
                DirectX::XMVECTOR EN = DirectX::XMLoadFloat3(&hitObj.gameObject.lock()->transform_->GetWorldPosition());
                DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());

                DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(E, P);
                DirectX::XMVECTOR PEN = DirectX::XMVectorSubtract(EN, P);

                float lenE = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
                float lenEN = DirectX::XMVectorGetX(DirectX::XMVector3Length(PEN));
                if (lenE > lenEN)enemyNearObj = hitObj.gameObject.lock();
            }
        }
    }

    //�G�����Ȃ��ꍇreturn
    return enemyNearObj;
}

std::shared_ptr<GameObject> AttackPlayer::AssistGetMediumEnemy()
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
    std::shared_ptr<GameObject> enemyNearObj;   //��ԋ߂��G������

    //�A�V�X�g����擾
    std::shared_ptr<Collider> assistColl = playerObj->GetChildFind("attackAssistMedium")->GetComponent<Collider>();
    std::vector<HitObj> hitGameObj = assistColl->OnHitGameObject();
    for (auto& hitObj : hitGameObj)
    {
        if (COLLIDER_TAG::Enemy == hitObj.gameObject.lock()->GetComponent<Collider>()->GetMyTag())
        {
            //�ŏ��͂��̂܂ܓ����
            if (!enemyNearObj)enemyNearObj = hitObj.gameObject.lock();
            //��ԋ߂��G��������
            else
            {
                DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemyNearObj->transform_->GetWorldPosition());
                DirectX::XMVECTOR EN = DirectX::XMLoadFloat3(&hitObj.gameObject.lock()->transform_->GetWorldPosition());
                DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());

                DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(E, P);
                DirectX::XMVECTOR PEN = DirectX::XMVectorSubtract(EN, P);

                float lenE = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
                float lenEN = DirectX::XMVectorGetX(DirectX::XMVector3Length(PEN));
                if (lenE > lenEN)enemyNearObj = hitObj.gameObject.lock();
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
    
    //����������
    DirectX::XMFLOAT3 pPos = playerObj->transform_->GetWorldPosition();
    pPos.y = 0;
    DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&pPos);
    DirectX::XMFLOAT3 ePos = enemy->transform_->GetWorldPosition();
    ePos.y = 0;
    DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&ePos);

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
    peQ = QuaternionStruct::LookRotation({ peR.x,0,peR.z });

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

void AttackPlayer::SquareAttackDirection(float elapsedTime)
{
    //�U������ނŉ�
    for (int attackIndex = 0; attackIndex < squareAttackMove_.size(); ++attackIndex)
    {
        auto& squDir = squareAttackMove_[attackIndex];
        if (!squDir.enable)continue;

        squDir.directionTimer -= elapsedTime;
        if (squDir.directionTimer < 0)
        {
            squDir.enable = false;
        }

        for (int index = 0; index < squDir.objData.size();)
        {
            //�������
            if (squDir.objData[index].obj.expired())
            {
                squDir.objData.erase(squDir.objData.begin() + index);
                continue;
            }

            //�G�t�F�N�g�I������ & �������
            std::shared_ptr<ParticleSystemCom> particle = squDir.objData[index].obj.lock()->GetComponent<ParticleSystemCom>();
            if (!squDir.enable)
            {
                particle->SetRoop(false);

                std::shared_ptr<GameObject> colObj = squDir.objData[index].obj.lock()->GetChildFind("attack");
                if (colObj)
                {
                    std::shared_ptr<Collider> col = colObj->GetComponent<Collider>();
                    col->SetEnabled(false);
                }
            }

            //�G�t�F�N�g���[�v���̂�
            if (particle->GetRoop()&& squDir.objData[index].isMove)
            {
                //������
                DirectX::XMFLOAT3 pos = squDir.objData[index].obj.lock()->transform_->GetWorldPosition();
                pos.x += squDir.objData[index].velocity.x * squDir.objData[index].speed * elapsedTime;
                pos.z += squDir.objData[index].velocity.z * squDir.objData[index].speed * elapsedTime;
                squDir.objData[index].obj.lock()->transform_->SetWorldPosition(pos);

                if (attackIndex > 0)
                {
                    //�T�C�Y�傫��
                    float timeRatio = (squDir.directionTime-squDir.directionTimer) / squDir.directionTime;
                    float scale = Mathf::Lerp(squDir.colliderScale, squDir.colliderScaleEnd, timeRatio);

                    squDir.objData[index].obj.lock()
                        ->GetChildren()[0].lock()
                        ->GetComponent<SphereColliderCom>()->SetRadius(scale);

                    particle->GetSaveParticleData().particleData.shape.radius = scale;
                }
            }

            index++;
        }
    }
}

void AttackPlayer::SpawnCombo1()
{
    std::shared_ptr<GameObject> player = player_.lock()->GetGameObject();

    //���U�������t���O
    isSquareDirection_ = true;

    //obj����
    std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect(ParticleComManager::COMBO_1);
    particle->transform_->SetWorldPosition(player->transform_->GetWorldPosition());
    particle->transform_->SetRotation(QuaternionStruct::LookRotation(player->transform_->GetWorldFront()).dxFloat4);

    //�q�ɓ����蔻��
    {
        std::shared_ptr<GameObject> obj = particle->AddChildObject();
        obj->SetName("attack");

        std::shared_ptr<SphereColliderCom> attackCol = obj->AddComponent<SphereColliderCom>();
        attackCol->SetMyTag(COLLIDER_TAG::PlayerAttack);
        attackCol->SetJudgeTag(COLLIDER_TAG::Enemy);
        attackCol->SetRadius(2.3f);

        std::shared_ptr<WeaponCom> weapon = obj->AddComponent<WeaponCom>();
        weapon->SetObject(GameObjectManager::Instance().Find("pico"));
        weapon->SetNodeParent(particle);
        weapon->SetIsForeverUse(true);
        weapon->SetAttackDefaultStatus(5, 0);
    }

    //���p�[�e�B�N��
    std::shared_ptr<GameObject> fireP1 = ParticleComManager::Instance().SetEffect(ParticleComManager::COMBO_1_FIRE, { 0,0,0 }, particle);
    {
        fireP1->SetName("fireParR");
        fireP1->transform_->SetLocalPosition(DirectX::XMFLOAT3(0, 0.2f, 0));
        fireP1->transform_->SetScale(DirectX::XMFLOAT3(2, 1, 2));
    }
    std::shared_ptr<GameObject> fireP2 = ParticleComManager::Instance().SetEffect(ParticleComManager::COMBO_1_FIRE, { 0,0,0 }, particle);
    {
        fireP2->SetName("fireParL");
        fireP2->transform_->SetLocalPosition(DirectX::XMFLOAT3(0, 0.2f, 0));
        fireP2->transform_->SetScale(DirectX::XMFLOAT3(-2, 1, 2));
    }

    //���U������������
    {
        squareAttackMove_[0].enable = true;
        squareAttackMove_[0].directionTime = 1;
        squareAttackMove_[0].directionTimer = squareAttackMove_[0].directionTime;
        squareAttackMove_[0].colliderScale = 2.3f;
        //�������w��
        SquareAttackMove::ObjData oData;
        oData.obj = particle;
        oData.isMove = true;
        oData.velocity = player->transform_->GetWorldFront();
        oData.speed = 20;
        squareAttackMove_[0].objData.emplace_back(oData);
        oData.obj = fireP1;
        oData.isMove = false;
        oData.velocity = { 0,0,0 };
        oData.speed = 0;
        squareAttackMove_[0].objData.emplace_back(oData);
        oData.obj = fireP2;
        oData.isMove = false;
        oData.velocity = { 0,0,0 };
        oData.speed = 0;
        squareAttackMove_[0].objData.emplace_back(oData);
    }

}

void AttackPlayer::SpawnCombo2()
{
    std::shared_ptr<GameObject> player = player_.lock()->GetGameObject();

    //���U�������t���O
    isSquareDirection_ = true;

    //obj����
    std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect(ParticleComManager::COMBO_2);
    DirectX::XMFLOAT3 pos = player->transform_->GetWorldPosition();
    pos.y += 1;
    particle->transform_->SetWorldPosition(pos);

    //��]�p�x�Z�o
    DirectX::XMVECTOR PlayerFront = DirectX::XMLoadFloat3(&player->transform_->GetWorldFront());
    float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMVector3Normalize(PlayerFront), { 0,0,1 }));
    float angle = ((dot * -1 + 1) / 2) * 180;
    if (0 < DirectX::XMVectorGetY(DirectX::XMVector3Cross(DirectX::XMVector3Normalize(PlayerFront), { 0,0,1 })))
        angle *= -1;
    particle->transform_->SetEulerRotation(DirectX::XMFLOAT3(0, angle + -54, 0));

    //�q�ɓ����蔻��
    {
        std::shared_ptr<GameObject> obj = particle->AddChildObject();
        obj->SetName("attack");
        obj->transform_->SetEulerRotation(DirectX::XMFLOAT3(0, 54, 0));

        std::shared_ptr<SphereColliderCom> attackCol = obj->AddComponent<SphereColliderCom>();
        attackCol->SetMyTag(COLLIDER_TAG::PlayerAttack);
        attackCol->SetJudgeTag(COLLIDER_TAG::Enemy);
        attackCol->SetRadius(2.3f);

        std::shared_ptr<WeaponCom> weapon = obj->AddComponent<WeaponCom>();
        weapon->SetObject(GameObjectManager::Instance().Find("pico"));
        weapon->SetNodeParent(particle);
        weapon->SetIsForeverUse(true);
        weapon->SetAttackDefaultStatus(5, 0);
        weapon->SetCircleArc(true);

    }

    //���U������������
    {
        squareAttackMove_[1].enable = true;
        squareAttackMove_[1].directionTime = 1;
        squareAttackMove_[1].directionTimer = squareAttackMove_[1].directionTime;
        squareAttackMove_[1].colliderScale = 1;
        squareAttackMove_[1].colliderScaleEnd = 10;
        //�������w��
        SquareAttackMove::ObjData oData;
        oData.obj = particle;
        oData.isMove = true;
        oData.velocity = { 0,0,0 };
        oData.speed = 0;
        squareAttackMove_[1].objData.emplace_back(oData);
    }
}

void AttackPlayer::SpawnCombo3()
{
    std::shared_ptr<GameObject> player = player_.lock()->GetGameObject();

    //���U�������t���O
    isSquareDirection_ = true;

    //obj����
    std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect(ParticleComManager::COMBO_3);
    DirectX::XMFLOAT3 pos = player->transform_->GetWorldPosition();
    pos.y += 0.5;
    particle->transform_->SetWorldPosition(pos);

    //�q�ɓ����蔻��
    {
        std::shared_ptr<GameObject> obj = particle->AddChildObject();
        obj->SetName("attack");

        std::shared_ptr<SphereColliderCom> attackCol = obj->AddComponent<SphereColliderCom>();
        attackCol->SetMyTag(COLLIDER_TAG::PlayerAttack);
        attackCol->SetJudgeTag(COLLIDER_TAG::Enemy);
        attackCol->SetRadius(2.3f);

        std::shared_ptr<WeaponCom> weapon = obj->AddComponent<WeaponCom>();
        weapon->SetObject(GameObjectManager::Instance().Find("pico"));
        weapon->SetNodeParent(particle);
        weapon->SetIsForeverUse(true);
        weapon->SetAttackDefaultStatus(3, 0);

    }

    //���U������������
    {
        squareAttackMove_[2].enable = true;
        squareAttackMove_[2].directionTime = 1;
        squareAttackMove_[2].directionTimer = squareAttackMove_[2].directionTime;
        squareAttackMove_[2].colliderScale = 1;
        squareAttackMove_[2].colliderScaleEnd = 10;
        //�������w��
        SquareAttackMove::ObjData oData;
        oData.obj = particle;
        oData.isMove = true;
        oData.velocity = { 0,0,0 };
        oData.speed = 0;
        squareAttackMove_[2].objData.emplace_back(oData);
    }
}

//���n���U������
void AttackPlayer::AttackOnGround()
{
    isSquareAttack_ = true;
}

//�W�����v���U������
void AttackPlayer::AttackJump()
{
    isSquareAttack_ = true;
}

//�����I�ɍU�����I��点��i�W�����v�����j
void AttackPlayer::AttackFlagEnd()
{
    jumpAttackComboWaitTimer_ = 0;
    comboSquareCount_ = 0;
    comboTriangleCount_ = 0;
    isComboJudge_ = true;
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
