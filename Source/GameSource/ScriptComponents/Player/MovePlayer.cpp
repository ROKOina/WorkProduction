#include "MovePlayer.h"

#include "PlayerCom.h"
#include "Components/MovementCom.h"
#include "Components/AnimatorCom.h"
#include "Components/CameraCom.h"
#include "Components/TransformCom.h"
#include "Components/ColliderCom.h"
#include "Input/Input.h"
#include "SystemStruct/QuaternionStruct.h"
#include "Graphics/Graphics.h"

void MovePlayer::Update(float elapsedTime)
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    //�ړ������͂���Ă�����
    if (IsMove(elapsedTime))
    {
        //�ړ�����
        //��]
        if (isInputTrun_)
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

    //�_�b�V������
    DashMove(elapsedTime);

    //�ړ��A�j���p�ɑ��͒������擾
    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetFloatValue("moveSpeed", length);
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

    if (move->OnGround() && jumpCount_ < 2)
    {
        jumpCount_ = 2;
        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);

        //�A�j���[�^�[
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("idle");

    }

    //�W�����v
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_B)
    {
        if (jumpCount_ > 0)
        {
            //�W���X�g��𒆂̂݃W�����v�ł��Ȃ�
            if (!player_.lock()->isJustJudge_)
            {
                //�U���Ɖ���I���t���O
                player_.lock()->AttackFlagEnd();
                DashEndFlag(false);

                //�A�j���[�^�[
                std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
                animator->ResetParameterList();
                animator->SetTriggerOn("jump");
                inputMoveVec_.y = jumpSpeed_;

                move->ZeroVelocityY();
                --jumpCount_;

                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP);
            }
        }
    }

    //�i�s�x�N�g�����[���x�N�g���łȂ��ꍇ�͓��͂��ꂽ
    if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.y * inputMoveVec_.y + inputMoveVec_.z * inputMoveVec_.z <= 0.1f) {
        DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
        float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

        if (isInputMove_)
            //if (length < 0.1f)
            //if (!isDashJudge_)
        {
            //���͂��I���ƕ�����
            if (player_.lock()->GetPlayerStatus() != PlayerCom::PLAYER_STATUS::JUMP)
                if (player_.lock()->comboAttackCount_ <= 0)
                    if (!isDashJudge_)
                    {
                        moveParamType_ = MOVE_PARAM::WALK;
                        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);
                        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);

                        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
                    }
        }
        return false;
    }

    //��ԍX�V
    if (player_.lock()->GetPlayerStatus() != PlayerCom::PLAYER_STATUS::JUMP)
        if (!isDashJudge_ && !player_.lock()->isJustJudge_ && player_.lock()->comboAttackCount_ <= 0)
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::MOVE);


    return true;
}

//��]
void MovePlayer::Trun(float elapsedTime)
{
    //���͕����̃N�H�[�^�j�I������
    QuaternionStruct inputQuaternion = QuaternionStruct::LookRotation(inputMoveVec_, player_.lock()->up_);
    QuaternionStruct playerQuaternion = player_.lock()->GetGameObject()->transform_->GetRotation();

    float worldSpeed = Graphics::Instance().GetWorldSpeed();

    //��������]
    DirectX::XMFLOAT4 rota;
    DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&playerQuaternion.dxFloat4),
        DirectX::XMLoadFloat4(&inputQuaternion.dxFloat4), moveParam_[moveParamType_].turnSpeed * (elapsedTime * worldSpeed)));

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

    if (!isDash_)return;

    //�_�b�V��
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER &&
        dashCoolTimer_ < 0)
    {
        isDashJudge_ = true;        //�_�b�V���t���OON

        dashCoolTimer_ = dashCoolTime_; //�N�[���^�C��

        //�A�j���[�^�[
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

        //�_�b�V����R���{
        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH)
        {
            if (player_.lock()->attackPlayer_->OnHitEnemy() && player_.lock()->attackPlayer_->ComboReadyEnemy())
            {
                Graphics::Instance().SetWorldSpeed(1.0f);
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::DASH);


                player_.lock()->animFlagName_ = "jump";
                animator->ResetParameterList();
                player_.lock()->attackPlayer_->DashAttack(2);
                animator->SetIsStop(true);
                isDash_ = false;
                return;
            }
        }

        //�A�^�b�N���Z�b�g
        player_.lock()->AttackFlagEnd();

        //���͂������state=0
        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
        {
            dashState_ = 0;
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP)
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP_DASH);
            else
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::DASH);
        }
        else
        {
            dashState_ = 10;
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP)
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP_BACK_DASH);
            else
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::BACK_DASH);
        }

        animator->SetTriggerOn("dash");


        ////�W���X�g���������
        //JustInisialize();
        //isNormalAttack_ = true;

    }

    //�_�b�V�����̍X�V
    DashStateUpdate(elapsedTime);

}

//�_�b�V�����̍X�V
void MovePlayer::DashStateUpdate(float elapsedTime)
{
    if (!isDashJudge_)return;

    //�W���X�g����̓����蔻��
    std::vector<HitObj> hitGameObj = player_.lock()->GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
    //�W���X�g��������G�l�~�[��ۑ�
    std::shared_ptr<GameObject> enemy;
    for (auto& hitObj : hitGameObj)
    {
        if (COLLIDER_TAG::JustAvoid != hitObj.gameObject->GetComponent<Collider>()->GetMyTag())continue;

        if (!enemy) {
            enemy = hitObj.gameObject->GetParent();
            continue;
        }
        //��ԋ߂��G��ۑ�
        DirectX::XMFLOAT3 pPos = player_.lock()->GetGameObject()->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 ePos = hitObj.gameObject->GetParent()->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 eOldPos = enemy->transform_->GetWorldPosition();

        DirectX::XMVECTOR PPos = DirectX::XMLoadFloat3(&pPos);
        DirectX::XMVECTOR EPos = DirectX::XMLoadFloat3(&ePos);
        DirectX::XMVECTOR EOldPos = DirectX::XMLoadFloat3(&eOldPos);

        float currentLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(PPos, EPos)));
        float oldLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(PPos, EOldPos)));

        if (currentLength < oldLength)
            enemy = hitObj.gameObject->GetParent();
    }


    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

    //�_�b�V���X�V
    switch (dashState_)
    {
    case 0:
    {
        //�p�x��ς���
        QuaternionStruct dashDirection = QuaternionStruct::LookRotation(inputMoveVec_, player_.lock()->up_);
        player_.lock()->GetGameObject()->transform_->SetRotation(dashDirection);
    }
    [[fallthrough]];    //�t�H�[�X���[(1�ɂ��̂܂܂���)
    //���͕����_�b�V��
    case 1:
    {
        isInputMove_ = true;
        player_.lock()->isNormalAttack_ = false;

        //�_�b�V���ɕύX
        move->ZeroVelocity();
        moveParamType_ = MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);
        dashStopTimer_ = dashStopTime_;
        dashState_++;
        break;
    }
    case 2:
        //�W���X�g��𔻒�
        if (enemy)
        {
            player_.lock()->isJustJudge_ = true;
            player_.lock()->justAvoidState_ = 0;
            dashState_ = -1;
            isDashJudge_ = false;
            player_.lock()->justHitEnemy_ = enemy;
            break;
        }

        //�ő呬�x�ɒB�����玟�̃X�e�[�g
        dashStopTimer_ -= elapsedTime;
        if (length > dashMaxSpeed_ || dashStopTimer_ < 0)
        {
            dashState_++;
            player_.lock()->isNormalAttack_ = true;
        }
        break;
    case 3:
    {
        //�����x�������Ă���
        float acce = move->GetMoveAcceleration();
        acce -= 10 * elapsedTime;
        move->SetMoveAcceleration(acce);

        if (moveParam_[MOVE_PARAM::RUN].moveMaxSpeed >= length)
            dashState_++;
        break;
    }
    case 4:
        //����ɕύX(�_�b�V���I��)
        moveParamType_ = MOVE_PARAM::RUN;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
        dashState_ = -1;
        isDashJudge_ = false;
        isInputMove_ = true;

        break;

        //�������_�b�V��
    case 10:
        //�_�b�V���ɕύX
        move->ZeroVelocity();
        moveParamType_ = MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed * 2);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);
        dashStopTimer_ = dashStopTime_;
        dashState_++;

        isInputMove_ = false;

        break;
    case 11:
    {
        //�W���X�g��𔻒�
        if (enemy)
        {
            player_.lock()->isJustJudge_ = true;
            player_.lock()->justAvoidState_ = 0;
            dashState_ = -1;
            isDashJudge_ = false;
            player_.lock()->justHitEnemy_ = enemy;
            break;
        }

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
        acce -= 10 * elapsedTime;
        move->SetMoveAcceleration(acce);

        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
            dashState_ = 1;

        if (moveParam_[MOVE_PARAM::RUN].moveMaxSpeed >= length)
            dashState_ = 3;
        break;
    }
    };

    //���͂��Ȃ��Ȃ邩�A�_�b�V���X�e�[�g��10�ȉ��̏ꍇ�i���_�b�V�����͑��͂�0�Ȃ̂Łj
    //�_�b�V���I��
    if (length < 0.1f && dashState_ < 10 && dashState_ > 1)
    {
        isDashJudge_ = false;
        dashState_ = -1;
    }
}

//�����I�Ƀ_�b�V�����I��点��i�U�������j
void MovePlayer::DashEndFlag(bool isWalk)
{
    isDashJudge_ = false;
    dashState_ = -1;

    if (isWalk)
    {
        std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
        moveParamType_ = MOVE_PARAM::WALK;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
    }
}
