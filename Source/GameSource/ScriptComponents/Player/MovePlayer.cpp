#include "MovePlayer.h"

#include "PlayerCom.h"
#include "Components/MovementCom.h"
#include "Components/AnimatorCom.h"
#include "Components/AnimationCom.h"
#include "Components/CameraCom.h"
#include "Components/TransformCom.h"
#include "Components/ColliderCom.h"
#include "Input/Input.h"
#include "SystemStruct/QuaternionStruct.h"
#include "Graphics/Graphics.h"
#include <imgui.h>

MovePlayer::MovePlayer(std::shared_ptr<PlayerCom> player) : player_(player)
{
    //�ړ��p�����[�^�[������
    moveParam_[MOVE_PARAM::WALK].moveMaxSpeed = 5.0f;
    moveParam_[MOVE_PARAM::WALK].moveAcceleration = 1.0f;
    moveParam_[MOVE_PARAM::WALK].turnSpeed = 4.0f;

    moveParam_[MOVE_PARAM::RUN].moveMaxSpeed = 8.0f;
    moveParam_[MOVE_PARAM::RUN].moveAcceleration = 2.0f;
    moveParam_[MOVE_PARAM::RUN].turnSpeed = 8.0f;

    moveParam_[MOVE_PARAM::JUSTDASH].moveMaxSpeed = 3.0f;
    moveParam_[MOVE_PARAM::JUSTDASH].moveAcceleration = 1.0f;
    moveParam_[MOVE_PARAM::JUSTDASH].turnSpeed = 4.0f;

    moveParam_[MOVE_PARAM::DASH].moveMaxSpeed = 15.0f;
    moveParam_[MOVE_PARAM::DASH].moveAcceleration = 6.0f;
    moveParam_[MOVE_PARAM::DASH].turnSpeed = 8.0f;

    moveParamType_ = MOVE_PARAM::WALK;
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
    move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);

    dashCoolTimer_ = 1;
}

void MovePlayer::Update(float elapsedTime)
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    //�ړ������͂���Ă�����
    if (IsMove(elapsedTime))
    {
        //�ړ�����

        //����؂�ւ�������
        RunTurnJudge();

        if (!isRunTurn_)    //�^�[��������Ȃ���Γ���
        {
            //��]
            if (isInputTurn_)
                Trun(elapsedTime);

            //�W���X�g��𒆂͒ʂ�Ȃ�
            if (isInputMove_)
            {
                //�c�����ړ�
                VerticalMove();
                //�������ړ�
                HorizonMove();
            }
        }
    }

    //�؂�ւ�������
    RunTurnMove();


    //�_�b�V������
    DashMove(elapsedTime);


    //�ړ��A�j���p�ɑ��͒������擾
    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetFloatValue("moveSpeed", length);
}

void MovePlayer::OnGui()
{
    ImGui::InputInt("paramType", &moveParamType_);
    ImGui::InputInt("dashState", &dashState_);
    ImGui::InputInt("jumpCount", &jumpCount_);
    ImGui::Checkbox("isInputMove", &isInputMove_);
    ImGui::Checkbox("isInputTurn", &isInputTurn_);
}

//�X�e�B�b�N���͒l����ړ��x�N�g�����擾
DirectX::XMFLOAT3 MovePlayer::GetMoveVec()
{
    //���͏����擾
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();

    //�J���������ƃX�e�B�b�N�̓��͒l�ɂ���Đi�s�������v�Z����
    std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
    const DirectX::XMFLOAT3& cameraRight = camera->GetRight();
    const DirectX::XMFLOAT3& cameraFront = camera->GetFront();

    //�ړ��x�N�g����XZ���ʂɐ����ȃx�N�g���ɂȂ�悤�ɂ���

    //�J�����E�����x�N�g����XZ�P�ʃx�N�g���ɕϊ�
    float cameraRightX = cameraRight.x;
    float cameraRightZ = cameraRight.z;
    float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
    if (cameraRightLength > 0.0f)
    {
        //�P�ʃx�N�g����
        cameraRightX /= cameraRightLength;
        cameraRightZ /= cameraRightLength;
    }

    //�J�����O�����x�N�g����XZ�P�ʃx�N�g���ɕϊ�
    float cameraFrontX = cameraFront.x;
    float cameraFrontZ = cameraFront.z;
    float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
    if (cameraFrontLength > 0.0f)
    {
        //�P�ʃx�N�g����
        cameraFrontX /= cameraFrontLength;
        cameraFrontZ /= cameraFrontLength;
    }

    //�X�e�B�b�N�̐������͒l���J�����E�����ɔ��f���A
    //�X�e�B�b�N�̐������͒l���J�����O�����ɔ��f���A
    //�i�s�x�N�g�����v�Z����
    DirectX::XMFLOAT3 vec;
    //XZ�������Ɉړ�
    vec.x = cameraFrontX * ay + cameraRightX * ax;
    vec.z = cameraFrontZ * ay + cameraRightZ * ax;
    //Y�������ɂ͈ړ����Ȃ�
    vec.y = 0.0f;

    return vec;
}

//�ړ����͏���
bool MovePlayer::IsMove(float elapsedTime)
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    //�i�s�x�N�g���擾
    inputMoveVec_ = GetMoveVec();

    //���n����
    if (move->OnGround() && jumpCount_ < 2
        && player_.lock()->GetPlayerStatus() != PlayerCom::PLAYER_STATUS::ATTACK_JUMP)
    {
        jumpCount_ = 2;
        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
        isInputMove_ = true;
        isInputTurn_ = true;

        //�A�j���[�^�[
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("idle");

        //�U�����n����
        player_.lock()->GetAttackPlayer()->AttackOnGround();
    }

    //�W�����v
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_A)
    {
        if (jumpCount_ > 0)
        {
            //�W���X�g��𒆂̂݃W�����v�ł��Ȃ�
            if (!player_.lock()->GetJustAvoidPlayer()->GetIsJustJudge())
            {
                //�I���t���O
                player_.lock()->GetAttackPlayer()->AttackFlagEnd();
                player_.lock()->GetAttackPlayer()->AttackJump();
                DashEndFlag(false);
                EndRunTurn(); 

                //�A�j���[�^�[
                std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
                animator->ResetParameterList();
                animator->SetTriggerOn("jump");
                isStopRunStop_ = false;

                //���x
                inputMoveVec_.y = jumpSpeed_;
                move->ZeroVelocityY();

                //�W�����v�񐔌��炷
                --jumpCount_;

                //�󒆃_�b�V�����Z�b�g
                jumpDashCount_ = 1;

                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP);
            }
        }
    }

    //�W�����v�~���A�j���[�V����
    if (!move->OnGround())
    {
        if (move->GetVelocity().y < 0)
        {
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP)
            {
                std::shared_ptr<AnimationCom> animation = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
                if (animation->GetCurrentAnimationIndex() != ANIMATION_PLAYER::JUMP_FALL)   //�؂艺�낵�U������Ȃ���
                {
                    //�A�j���[�^�[
                    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
                    animator->SetTriggerOn("jumpFall");
                }
            }
        }
    }

    //�X�g�b�v���[�V�����t���O����
    if (isStopRunStop_)
    {
        std::shared_ptr<AnimationCom> animation = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
        if (animation->IsPlayAnimation())
        {
            return false;
        }
        else
        {
            isStopRunStop_ = false;
        }
    }

    static float nonInputTime = 0.1f;   //���͂���Ă��Ȃ����Ԃ𑪂�
    //�i�s�x�N�g�����[���x�N�g���łȂ��ꍇ�͓��͂��ꂽ
    if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.y * inputMoveVec_.y + inputMoveVec_.z * inputMoveVec_.z <= 0.1f)
    {
        DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
        float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

        if (isInputMove_)
        {
            //���͂��I���ƕ�����
            if (player_.lock()->GetPlayerStatus() != PlayerCom::PLAYER_STATUS::JUMP)
                if (player_.lock()->GetAttackPlayer()->GetComboSquareCount() <= 0&&player_.lock()->GetAttackPlayer()->GetComboTriangleCount() <= 0)
                    if (!isDashJudge_)
                    {
                        nonInputTime -= elapsedTime;
                        if (nonInputTime < 0)   //���͂���Ă��Ȃ��P�\���Ԃ��߂���Ε�����
                        {
                            if (moveParamType_ != MOVE_PARAM::WALK)
                            {                
                                if (player_.lock()->GetPlayerStatusOld() == PlayerCom::PLAYER_STATUS::MOVE
                                    || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::BACK_DASH)
                                {
                                    //�A�j���[�^�[
                                    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
                                    animator->SetTriggerOn("runStop");
                                    isStopRunStop_ = true;
                                    move->ZeroVelocity();
                                }

                                moveParamType_ = MOVE_PARAM::WALK;
                                move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);
                                move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
                            }
                        }

                        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
                    }
        }
        return false;
    }
    else
    {
        //���͂���Ă��Ȃ��P�\����
        nonInputTime = 0.05f;
    }

    //��ԍX�V
    if (player_.lock()->GetPlayerStatus() != PlayerCom::PLAYER_STATUS::JUMP && isInputMove_)
    {
        if (!isDashJudge_ && !player_.lock()->GetJustAvoidPlayer()->GetIsJustJudge()
            && player_.lock()->GetAttackPlayer()->GetComboSquareCount() <= 0
            && player_.lock()->GetAttackPlayer()->GetComboTriangleCount() <= 0
            )
        {
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::MOVE);
        }
    }


    return true;
}

//��]
void MovePlayer::Trun(float elapsedTime)
{
    //���͕����̃N�H�[�^�j�I������
    QuaternionStruct inputQuaternion = QuaternionStruct::LookRotation({ inputMoveVec_.x,0,inputMoveVec_.z });
    QuaternionStruct playerQuaternion = player_.lock()->GetGameObject()->transform_->GetRotation();

    float worldSpeed = Graphics::Instance().GetWorldSpeed();

    //��������]
    DirectX::XMFLOAT4 rota;

    //�^�[���X�s�[�h
    float turnSpeed = moveParam_[moveParamType_].turnSpeed;
    if (player_.lock()->GetAttackPlayer()->GetComboSquareCount() > 0
        || player_.lock()->GetAttackPlayer()->GetComboTriangleCount() > 0
        )   //�U�����Ȃ�x������
        turnSpeed = 5.5f;

    DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&playerQuaternion.dxFloat4),
        DirectX::XMLoadFloat4(&inputQuaternion.dxFloat4), turnSpeed * (elapsedTime * worldSpeed)));

    player_.lock()->GetGameObject()->transform_->SetRotation(rota);
}

//�c�����ړ�
void MovePlayer::VerticalMove()
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    move->AddForce({ 0,inputMoveVec_.y,0 });
}

//�������ړ�
void MovePlayer::HorizonMove()
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    //�O�����Ɉړ�����͂𐶐�
    DirectX::XMFLOAT3 forward = player_.lock()->GetGameObject()->transform_->GetWorldFront();
    forward.y = 0;
    DirectX::XMVECTOR Forward = DirectX::XMLoadFloat3(&forward);
    DirectX::XMFLOAT3 moveVec;
    DirectX::XMStoreFloat3(&moveVec, DirectX::XMVectorScale(Forward, moveParam_[moveParamType_].moveSpeed));

    //�͂ɉ�����
    move->AddForce(moveVec);
}

//�_�b�V��
void MovePlayer::DashMove(float elapsedTime)
{
    //�_�b�V���N�[���^�C���X�V
    if (dashCoolTimer_ >= 0)
    {
        dashCoolTimer_ -= elapsedTime;
    }

    //�󒆃_�b�V�����Z�b�g
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    if (move->OnGround())
    {
        jumpDashCount_ = 1;
    }

    if (!isDash_)return;

    //�_�b�V��
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER &&
        dashCoolTimer_ < 0 && jumpDashCount_ > 0)
    {
        //�^�[�������I��
        EndRunTurn();

        isStopRunStop_ = false;

        isDashJudge_ = true;        //�_�b�V���t���OON

        dashCoolTimer_ = dashCoolTime_; //�N�[���^�C��

        jumpDashCount_--;   //�󒆃_�b�V���J�E���g������

        //�A�j���[�^�[
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

        animator->ResetParameterList();

        //�A�^�b�N���Z�b�g
        player_.lock()->GetAttackPlayer()->AttackFlagEnd();

        //���͂������state=0
        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
        {
            dashState_ = 0;
            if (!move->OnGround())
                //if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP)
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP_DASH);
            else
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::DASH);

            animator->SetTriggerOn("dash");
        }
        else
        {
            dashState_ = 10;
            if (!move->OnGround())
                //if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP)
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP_BACK_DASH);
            else
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::BACK_DASH);

            animator->SetTriggerOn("dashBack");
        }

    }

    //�_�b�V�����̍X�V
    DashStateUpdate(elapsedTime);

}

//�_�b�V�����̍X�V
void MovePlayer::DashStateUpdate(float elapsedTime)
{
    if (!isDashJudge_)return;

    //�W���X�g��𔻒�
    player_.lock()->GetJustAvoidPlayer()->JustAvoidJudge();

    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

    //�_�b�V���X�V
    switch (dashState_)
    {
    case 0:
    {
        //�p�x��ς���
        QuaternionStruct dashDirection = QuaternionStruct::LookRotation(inputMoveVec_);
        player_.lock()->GetGameObject()->transform_->SetRotation(dashDirection);

    }
    [[fallthrough]];    //�t�H�[�X���[(1�ɂ��̂܂܂���)
    //���͕����_�b�V��
    case 1:
    {
        isInputMove_ = true;
        player_.lock()->GetAttackPlayer()->SetIsNormalAttack(false);

        //�_�b�V���ɕύX
        move->ZeroVelocity();
        moveParamType_ = MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);
        dashStopTimer_ = dashStopTime_;

        //�A�j���[�^�[
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("idle");

        dashState_ = 2;
        break;
    }
    case 2:
        //�W���X�g��𔻒�
        if (player_.lock()->GetJustAvoidPlayer()->GetJustHitEnemy().lock())
        {
            player_.lock()->GetJustAvoidPlayer()->StartJustAvoid();
            dashState_ = -1;
            isDashJudge_ = false;
            break;
        }

        //�󒆂̎��͏d�͂�0��
        if (!move->OnGround() && player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP_DASH)
            move->SetGravity(GRAVITY_ZERO);

        //�ő呬�x�ɒB�����玟�̃X�e�[�g
        dashStopTimer_ -= elapsedTime;
        if (length > dashMaxSpeed_ || dashStopTimer_ < 0)
        {
            dashState_++;
            player_.lock()->GetAttackPlayer()->SetIsNormalAttack(true);
        }

        break;
    case 3:
    {
        //�����x�������Ă���
        float acce = move->GetMoveAcceleration();
        acce -= 10 * elapsedTime;
        if (acce < 0)
            acce = 0;
        move->SetMoveAcceleration(acce);

        if (moveParam_[MOVE_PARAM::RUN].moveMaxSpeed >= length)
            dashState_++;
        break;
    }
    case 4:
    {
        //����ɕύX(�_�b�V���I��)
        moveParamType_ = MOVE_PARAM::RUN;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
        dashState_ = -1;
        isDashJudge_ = false;
        isInputMove_ = true;

        //�A�j���[�^�[
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("idle");

        //�󒆂��̔���

        if (move->OnGround())
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
        else
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP);


        if (move->GetGravity() < 10.0f)
            move->SetGravity(GRAVITY_NORMAL);

        break;
    }

        //�������_�b�V��
    case 10:
        //�_�b�V���ɕύX
        move->ZeroVelocity();
        moveParamType_ = MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);
        dashStopTimer_ = dashStopTime_;
        dashState_++;

        isInputMove_ = false;

        break;
    case 11:
    {
        //�W���X�g��𔻒�
        if (player_.lock()->GetJustAvoidPlayer()->GetJustHitEnemy().lock())
        {
            player_.lock()->GetJustAvoidPlayer()->StartJustAvoid();
            dashState_ = -1;
            isDashJudge_ = false;
            break;
        }

        //�󒆂̎��͏d�͂�0��
        if (!move->OnGround())
            move->SetGravity(GRAVITY_ZERO);

        DirectX::XMFLOAT3 front = player_.lock()->GetGameObject()->transform_->GetWorldFront();
        DirectX::XMFLOAT3 back = { -front.x,0,-front.z };
        DirectX::XMStoreFloat3(&back, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&back), moveParam_[moveParamType_].moveSpeed));
        move->AddForce(back);

        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
            dashState_ = 1;

        //�ő呬�x�ɒB�����玟�̃X�e�[�g
        dashStopTimer_ -= elapsedTime;
        if (length > dashMaxSpeed_ * 0.5f || dashStopTimer_ < 0)
            dashState_++;
        break;
    }
    case 12:
    {
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);

        DirectX::XMFLOAT3 front = player_.lock()->GetGameObject()->transform_->GetWorldFront();
        DirectX::XMFLOAT3 back = { -front.x,0,-front.z };
        DirectX::XMStoreFloat3(&back, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&back), moveParam_[moveParamType_].moveSpeed));
        move->AddForce(back);

        //�����x�������Ă���
        float acce = move->GetMoveAcceleration();
        acce -= 30 * elapsedTime;
        if (acce < 0)
            acce = 0;
        move->SetMoveAcceleration(acce);

        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
        {
            dashState_ = 1;
            move->SetGravity(GRAVITY_NORMAL);

            //�󒆂��̔���
            if (move->OnGround())
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
            else
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP);

        }

        if (moveParam_[MOVE_PARAM::RUN].moveMaxSpeed >= length)
            dashState_ = 3;
        break;
    }
    };

    ////���͂��Ȃ��Ȃ邩�A�_�b�V���X�e�[�g��10�ȉ��̏ꍇ�i���_�b�V�����͑��͂�0�Ȃ̂Łj
    ////�_�b�V���I��
    //if (length < 0.1f && dashState_ < 10 && dashState_ > 1)
    //{
    //    move->SetGravity(GRAVITY_NORMAL);
    //    isDashJudge_ = false;
    //    dashState_ = -1;
    //    player_.lock()->GetAttackPlayer()->isNormalAttack_ = true;

    //    moveParamType_ = MOVE_PARAM::RUN;
    //    move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
    //    move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
    //}
}

//����؂�ւ�������
void MovePlayer::RunTurnJudge()
{
    if (isRunTurn_)return;

    //�󒆂������ꍇ�͕Ԃ�
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    if (!move->OnGround())return;

    //�A�j���[�V����
    std::shared_ptr<AnimationCom> animation = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
    if (animation->GetCurrentAnimationIndex() != ANIMATION_PLAYER::RUN_HARD_2)return;

    //�v���C���[�̌���
    DirectX::XMFLOAT3 front = player_.lock()->GetGameObject()->transform_->GetWorldFront();

    //�p�x�����߂�
    float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&front), DirectX::XMLoadFloat3(&inputMoveVec_)));

    //�������������ꍇ
    if (dot < -0.7)
    {
        moveParamType_ = MOVE_PARAM::RUN;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);

        saveTurnVec_ = { inputMoveVec_.x,0,inputMoveVec_.z };
        isRunTurn_ = true;
        runTurnState_ = 0;
    }
}


//����؂�ւ�������
void MovePlayer::RunTurnMove()
{
    switch (runTurnState_)
    {
    case 0:
    {
        //�A�j���[�^�[
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("runTurn");
        animator->SetAnimationSpeedOffset(1.5f);
        runTurnState_++;
        break;
    }
    case 1:
    {
        std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
        move->ZeroVelocity();
        runTurnState_++;
        break;
    }
    case 2:
    {
        //�A�j���[�V����
        std::shared_ptr<AnimationCom> animation = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
        if (!animation->IsPlayAnimation())
        {
            runTurnState_++;
        }
        else if (animation->GetCurrentAnimationIndex() != ANIMATION_PLAYER::RUN_TURN_FORWARD)
        {
            runTurnState_++;
        }

        QuaternionStruct inputRotato = QuaternionStruct::LookRotation(saveTurnVec_);
        QuaternionStruct playerRotato = player_.lock()->GetGameObject()->transform_->GetRotation();

        DirectX::XMStoreFloat4(&playerRotato.dxFloat4, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&playerRotato.dxFloat4), DirectX::XMLoadFloat4(&inputRotato.dxFloat4), 0.2f));

        player_.lock()->GetGameObject()->transform_->SetRotation(playerRotato);

        break;
    }
    case 3:
    {
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetAnimationSpeedOffset(1);

        runTurnState_ = -1;
        isRunTurn_ = false;

        break;
    }

    }

}

//�����I�Ƀ^�[�����I��点��
void MovePlayer::EndRunTurn()
{
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetAnimationSpeedOffset(1);

    runTurnState_ = -1;
    isRunTurn_ = false;
}

//�����I�Ƀ_�b�V�����I��点��i�U�������j
void MovePlayer::DashEndFlag(bool isWalk)   //�����ɐ؂�ւ��邩
{
    isDashJudge_ = false;
    dashState_ = -1;
    player_.lock()->GetAttackPlayer()->SetIsNormalAttack(true);

    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    //�d�͏�����
    move->SetGravity(GRAVITY_NORMAL);

    if (moveParamType_ == MOVE_PARAM::DASH)
    {
        moveParamType_ = MOVE_PARAM::RUN;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
    }

    if (isWalk)
    {
        moveParamType_ = MOVE_PARAM::WALK;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
    }
}

//�ړ��̑����Z�b�g
void MovePlayer::SetMoveParamType(MOVE_PARAM moveParam)
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    moveParamType_ = moveParam;
    move->SetMoveMaxSpeed(moveParam_[moveParam].moveMaxSpeed);
    move->SetMoveAcceleration(moveParam_[moveParam].moveAcceleration);
}
