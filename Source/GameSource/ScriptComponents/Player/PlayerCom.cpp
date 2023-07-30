#include "PlayerCom.h"
#include "Input/Input.h"
#include <imgui.h>
#include "Components\CameraCom.h"
#include "Components\TransformCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"

#include "../Enemy/EnemyCom.h"
#include "../Weapon\WeaponCom.h"
#include "../CharacterStatusCom.h"

//�A�j���[�V�������X�g
enum AnimationPlayer
{
    WALK_RUNRUN_1,
    IDEL_1,
    IDEL_2,
    JUMP_1,
    JUMP_2,
    RUN_HARD_1,
    RUN_HARD_2,
    RUN_SOFT_1,
    RUN_SOFT_2,
    WALK_RUNRUN_2,
    PUNCH,
    BIGSWORD_UP,
    BIGSWORD_LEFT,
    BIGSWORD_RIGHT,
    BIGSWORD_DOWN,
    DASH_ANIM,

};

// �J�n����
void PlayerCom::Start()
{
    //�J�������v���C���[�Ƀt�H�[�J�X����
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");

    //�v���C���[�̃��[���h�|�W�V�������擾
    DirectX::XMFLOAT3 wp = GetGameObject()->transform_->GetWorldPosition();
    wp.z -= 10;
    wp.y += 6;
    cameraObj->transform_->SetLocalPosition(wp);

    //������傫��
    GetGameObject()->GetComponent<CapsuleColliderCom>()->SetRadius(0.17f);

    //�ړ��p�����[�^�[������
    moveParam_[MOVE_PARAM::WALK].moveMaxSpeed = 5.0f;
    moveParam_[MOVE_PARAM::WALK].moveAcceleration = 1.2f;
    moveParam_[MOVE_PARAM::WALK].turnSpeed = 4.0f;

    moveParam_[MOVE_PARAM::RUN].moveMaxSpeed = 8.0f;
    moveParam_[MOVE_PARAM::RUN].moveAcceleration = 1.2f;
    moveParam_[MOVE_PARAM::RUN].turnSpeed = 8.0f;

    moveParam_[MOVE_PARAM::JUSTDASH].moveMaxSpeed = 3.0f;
    moveParam_[MOVE_PARAM::JUSTDASH].moveAcceleration = 1.0f;
    moveParam_[MOVE_PARAM::JUSTDASH].turnSpeed = 4.0f;

    moveParam_[MOVE_PARAM::DASH].moveMaxSpeed = 20.0f;
    moveParam_[MOVE_PARAM::DASH].moveAcceleration = 1.0f;
    moveParam_[MOVE_PARAM::DASH].turnSpeed = 8.0f;

    moveParamType_ = MOVE_PARAM::WALK;
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
    move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
    move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);

    //����X�e�[�^�X������
    std::shared_ptr<WeaponCom> weapon = GetGameObject()->GetChildFind("greatSword")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_RIGHT, 1, 30,0.7f,0.3f);
    weapon->SetAttackStatus(BIGSWORD_UP, 1, 30, 0.2f, 0.8f);

    //�A�j���[�V����������
    AnimationInitialize();

}

static bool aaa = true;
// �X�V����
void PlayerCom::Update(float elapsedTime)
{
    //�J�����ݒ�
    if(aaa)
    {
        //�J�������v���C���[�Ƀt�H�[�J�X����
        std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");
        cameraObj->transform_->LookAtTransform(GetGameObject()->transform_->GetLocalPosition());

        //�v���C���[�̃��[���h�|�W�V�������擾
        DirectX::XMFLOAT3 wp = GetGameObject()->transform_->GetWorldPosition();
        wp.z -= 10;
        wp.y += 6;
        cameraObj->transform_->SetLocalPosition(wp);

    }

    //�ړ�
    {
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        //�ړ������͂���Ă�����
        if (IsMove(elapsedTime))
        {
            //�ړ�����
            //��]
            Trun(elapsedTime);

            //�W���X�g��𒆂͒ʂ�Ȃ�
            if (isInputMove_)
            //if (!isJustJudge_)
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
        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetFloatValue("moveSpeed", length);
    }

    //�W���X�g���
    {
        //�X�V
        JustAvoidanceAttackUpdate(elapsedTime);
    }

    //�U��
    {
        //�U���X�V
        AttackUpdate();
        //�����蔻��
        AttackJudgeCollision();
    }

    //�J�v�Z�������蔻��ݒ�
    {
        std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
        //�������瓪�܂ŃJ�v�Z����ݒ�
        capsule->SetPosition1({ 0,0,0 });
        Model::Node* headNode = GetGameObject()->GetComponent<RendererCom>()->GetModel()->FindNode("Head");
        DirectX::XMMATRIX PWorld = DirectX::XMLoadFloat4x4(&headNode->parent->worldTransform);
        DirectX::XMFLOAT3 pos;
        DirectX::XMStoreFloat3(&pos, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&headNode->translate), PWorld));
        DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetWorldPosition();
        capsule->SetPosition2({ pos.x - playerPos.x,pos.y - playerPos.y,pos.z - playerPos.z });
    }

    //�����Ƃ̓����蔻��
    std::vector<HitObj> hitGameObj = GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
    for (auto& hitObj : hitGameObj)
    {
        //���O����
        //if (std::strcmp(hitObj.gameObject->GetName(), "picolabo") != 0)continue;
        //�^�O����
        //�G�Ƃ̓�����i���j
        if (COLLIDER_TAG::Enemy == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
        {
            //�����Ԃ�
            DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetWorldPosition();
            DirectX::XMFLOAT3 hitPos = hitObj.gameObject->transform_->GetWorldPosition();

            DirectX::XMVECTOR PlayerPos = { playerPos.x,0,playerPos.z };
            DirectX::XMVECTOR HitPos = { hitPos.x,0, hitPos.z };

            DirectX::XMVECTOR ForceNormal = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(HitPos, PlayerPos));
            ForceNormal = DirectX::XMVectorScale(ForceNormal, 5);

            DirectX::XMFLOAT3 power;
            DirectX::XMStoreFloat3(&power, ForceNormal);

            hitObj.gameObject->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);
        }
    }

}

int oo = 0;
// GUI�`��
void PlayerCom::OnGUI()
{

    ImGui::InputInt("nowMoveParamType", &moveParamType_);
    ImGui::SliderInt("moveParamType", &oo, 0, MOVE_PARAM::MAX-1);
    ImGui::DragFloat("moveMaxSpeed", &moveParam_[oo].moveMaxSpeed, 0.1f);
    ImGui::DragFloat("moveSpeed", &moveParam_[oo].moveSpeed, 0.1f);
    ImGui::DragFloat("moveAcceleration", &moveParam_[oo].moveAcceleration, 0.1f);

    ImGui::InputInt("state", &dashState_);

    ImGui::DragFloat("jumpSpeed", &jumpSpeed_, 0.1f);

    ImGui::DragFloat3("up", &up_.x);

    ImGui::Checkbox("dash", &isDashJudge_);
    ImGui::Checkbox("aaa", &aaa);

    //���͏����擾
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();
    ImGui::DragFloat("x", &ax);
    ImGui::DragFloat("y", &ay);
}


#pragma region �ړ��n����

//�X�e�B�b�N���͒l����ړ��x�N�g�����擾
DirectX::XMFLOAT3 PlayerCom::GetMoveVec()
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
bool PlayerCom::IsMove(float elapsedTime)
{
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

    //�i�s�x�N�g���擾
    inputMoveVec_ = GetMoveVec();

    //�W�����v
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_B)
    {
        //�A�j���[�^�[
        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("jump");
        inputMoveVec_.y = jumpSpeed_;
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
            moveParamType_ = MOVE_PARAM::WALK;
            move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);
            move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
        }
        return false;
    }
    return true;
}

//��]
void PlayerCom::Trun(float elapsedTime)
{
    //���͕����̃N�H�[�^�j�I������
    QuaternionStruct inputQuaternion = QuaternionStruct::LookRotation(inputMoveVec_, up_);
    QuaternionStruct playerQuaternion = GetGameObject()->transform_->GetRotation();

    //��������]
    DirectX::XMFLOAT4 rota;
    DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&playerQuaternion.dxFloat4),
        DirectX::XMLoadFloat4(&inputQuaternion.dxFloat4), moveParam_[moveParamType_].turnSpeed * elapsedTime));

    GetGameObject()->transform_->SetRotation(rota);
}

//�c�����ړ�
void PlayerCom::VerticalMove()
{
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

    move->AddForce({ 0,inputMoveVec_.y,0 });
}

//�������ړ�
void PlayerCom::HorizonMove()
{
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

    //�O�����Ɉړ�����͂𐶐�
    DirectX::XMFLOAT3 forward = GetGameObject()->transform_->GetWorldFront();
    DirectX::XMVECTOR Forward = DirectX::XMLoadFloat3(&forward);
    DirectX::XMFLOAT3 moveVec;
    DirectX::XMStoreFloat3(&moveVec, DirectX::XMVectorScale(Forward, moveParam_[moveParamType_].moveSpeed));

    //�͂ɉ�����
    move->AddForce(moveVec);
}

//�_�b�V��
void PlayerCom::DashMove(float elapsedTime)
{
    if (!isDash_)return;

    //�W���X�g����̓����蔻��
    std::vector<HitObj> hitGameObj = GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
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
        DirectX::XMFLOAT3 pPos = GetGameObject()->transform_->GetWorldPosition();
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

    //�_�b�V��
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER)
    {
        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
            dashState_ = 0;
        else
            dashState_ = 10;
        isDashJudge_ = true;        //�_�b�V���t���OON

        //�A�j���[�^�[
        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("dash");


        //�W���X�g���������
        JustInisialize();
        isNormalAttack = true;
    }

    //�_�b�V�����̍X�V
    DashStateUpdate(elapsedTime, enemy);

}

//�_�b�V�����̍X�V
void PlayerCom::DashStateUpdate(float elapsedTime, std::shared_ptr<GameObject> enemy)
{
    if (!isDashJudge_)return;

    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

    //�_�b�V���X�V
    switch (dashState_)
    {
        case 0:
        {
            //�p�x��ς���
            QuaternionStruct dashDirection = QuaternionStruct::LookRotation(inputMoveVec_, up_);
            GetGameObject()->transform_->SetRotation(dashDirection);
        }
        [[fallthrough]];    //�t�H�[�X���[(1�ɂ��̂܂܂���)
        //���͕����_�b�V��
        case 1:
        {
            isInputMove_ = true;

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
                isJustJudge_ = true;
                justAvoidState_ = 0;
                dashState_ = -1;
                isDashJudge_ = false;
                justHitEnemy_ = enemy;
                break;
            }

            //�ő呬�x�ɒB�����玟�̃X�e�[�g
            dashStopTimer_ -= elapsedTime;
            if (length > dashMaxSpeed_ || dashStopTimer_ < 0)
                dashState_++;
            break;
        case 3:
        {
            //�����x�������Ă���
            float acce = move->GetMoveAcceleration();
            acce -= 2 * elapsedTime;
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
                isJustJudge_ = true;
                justAvoidState_ = 0;
                dashState_ = -1;
                isDashJudge_ = false;
                justHitEnemy_ = enemy;
                break;
            }

            DirectX::XMFLOAT3 front = GetGameObject()->transform_->GetWorldFront();
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

            DirectX::XMFLOAT3 front = GetGameObject()->transform_->GetWorldFront();
            DirectX::XMFLOAT3 back = { -front.x,0,-front.z };
            DirectX::XMStoreFloat3(&back, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&back), moveParam_[moveParamType_].moveSpeed));
            move->AddForce(back);

            //�����x�������Ă���
            float acce = move->GetMoveAcceleration();
            acce -= 2 * elapsedTime;
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
    if (length < 0.1f && dashState_ < 10)
    {
        isDashJudge_ = false;
        dashState_ = -1;
    }
}

#pragma endregion



#pragma region �W���X�g���

//�W���X�g���������
void PlayerCom::JustInisialize()
{
    justAvoidState_ = -1;
    isJustJudge_ = false;
}

//�W���X�g��𔽌��X�V����
void PlayerCom::JustAvoidanceAttackUpdate(float elapsedTime)
{
    //�W���X�g��𔻒莞
    if (isJustJudge_)
    {
        //�W���X�g����ړ�����
        JustAvoidanceMove(elapsedTime);

        //�W���X�g����㔽�����͊m�F
        JustAvoidanceAttackInput();
    }

    //���������X�V
    switch (justAvoidKey_)
    {
        //������
    case PlayerCom::JUST_AVOID_KEY::SQUARE:
        JustAvoidanceSquare(elapsedTime);
        break;
    case PlayerCom::JUST_AVOID_KEY::TRIANGLE:
        break;
    }

}

//�W���X�g����ړ�����
void PlayerCom::JustAvoidanceMove(float elapsedTime)
{
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

    switch (justAvoidState_)
    {
    case 0:
        //�W���X�g����ɕύX
        moveParamType_ = MOVE_PARAM::JUSTDASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::JUSTDASH].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::JUSTDASH].moveAcceleration);
        justAvoidTimer_ = justAvoidTime_;
        justAvoidState_++;

        isInputMove_ = false;
        isNormalAttack = false;

        //�G�̕�������
        GetGameObject()->transform_->LookAtTransform(justHitEnemy_->transform_->GetWorldPosition());
         

        break;
    case 1:
    {
        //�W���X�g����I���^�C�}�[
        justAvoidTimer_ -= elapsedTime;
        if (justAvoidTimer_ < 0)
        {
            JustInisialize();
            moveParamType_ = MOVE_PARAM::RUN;
            move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
            move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
            isInputMove_ = true;
            isNormalAttack = true;
            break;
        }

        //�ړ����������߂�i�W���X�g��𒆂͏���Ɉړ�����j
        DirectX::XMFLOAT3 Direction;
        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
        {
            Direction = { inputMoveVec_.x ,0,inputMoveVec_.z };
        }
        else
        {
            Direction = GetGameObject()->transform_->GetWorldFront();
            //y�������ċt�����ɂ���
            Direction.x *= -1;
            Direction.y = 0;
            Direction.z *= -1;
            //���K��
            DirectX::XMStoreFloat3(&Direction, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&Direction)));
        }
        Direction.x *= moveParam_[MOVE_PARAM::JUSTDASH].moveSpeed;
        Direction.z *= moveParam_[MOVE_PARAM::JUSTDASH].moveSpeed;
        //�͂ɉ�����
        move->AddForce(Direction);

        break;
    }
    }
}

//�W���X�g��𔽌����͊m�F
void PlayerCom::JustAvoidanceAttackInput()
{
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

    //�{�^���Ŕ����ς���
    GamePad& gamePad = Input::Instance().GetGamePad();

    //���̏ꍇ
    if (gamePad.GetButtonDown() & GamePad::BTN_Y)
    {
        JustInisialize();
        moveParamType_ = MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);

        justAvoidKey_ = JUST_AVOID_KEY::SQUARE;

        isDash_ = false;
    }
}

//������
void PlayerCom::JustAvoidanceSquare(float elapsedTime)
{
    //�G�ɐڋ߂���
    {
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();

        DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&GetGameObject()->transform_->GetWorldPosition());
        DirectX::XMVECTOR EnemyPos = DirectX::XMLoadFloat3(&justHitEnemy_->transform_->GetWorldPosition());
        DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(EnemyPos, Pos);
        float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
        //�G�̋߂��܂ňړ�����
        if (length < 1.5f)
        {
            JustInisialize();
            moveParamType_ = MOVE_PARAM::RUN;
            move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
            move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
            justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
            isInputMove_ = true;
            isNormalAttack = true;

            //�A�j���[�^�[
            std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
            animator->SetTriggerOn("bigSwordUp");

            isDash_ = true;
        }

        DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(PE);
        Dir = DirectX::XMVectorScale(Dir, moveParam_[MOVE_PARAM::DASH].moveSpeed);
        DirectX::XMFLOAT3 dir;
        DirectX::XMStoreFloat3(&dir, Dir);

        //�͂ɉ�����
        move->AddForce(dir);

    }
        //isInputMove_ = true;
}

#pragma endregion



#pragma region �U��

//�U���X�V
void PlayerCom::AttackUpdate()
{
    if (!isNormalAttack)return;

    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_Y)
    {
        //�A�j���[�^�[
        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("bigSwordRight");
    }
}

//�U�������蔻��
void PlayerCom::AttackJudgeCollision()
{
    //�q���̍U���I�u�W�F�N�g�̓�����
    DirectX::XMFLOAT3 attackPos;
    std::shared_ptr<GameObject> attackChild = GetGameObject()->GetChildFind("picoAttack");
    //AnimationEvent�擾���ē����蔻����Z�b�g
    if (GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("attackPunch", attackPos)) {
        attackChild->GetComponent<Collider>()->SetEnabled(true);
        attackChild->transform_->SetWorldPosition(attackPos);
    }
    else
    {
        attackChild->GetComponent<Collider>()->SetEnabled(false);
    }

    //�q���̍U���I�u�W�F�N�g�̓����蔻��
    std::vector<HitObj> hitAttack = attackChild->GetComponent<Collider>()->OnHitGameObject();
    for (auto& hitObj : hitAttack)
    {
        //�G�ɍU��������������
        if (COLLIDER_TAG::Enemy == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
        {
            std::shared_ptr<EnemyCom> enemy = hitObj.gameObject->GetComponent<EnemyCom>();
            std::shared_ptr<CharacterStatusCom> enemyStatus = hitObj.gameObject->GetComponent<CharacterStatusCom>();
            //���G���͍U���ł��Ȃ�
            if (enemyStatus->GetIsInvincible())continue;

            DirectX::XMFLOAT3 powerForce = { 0,0,0 };
            float power = 30;

            //�v���C���[�̑O�����ɔ�΂�
            DirectX::XMFLOAT3 playerForward = GetGameObject()->transform_->GetWorldFront();
            powerForce.x = playerForward.x * power;
            powerForce.z = playerForward.z * power;


            enemyStatus->OnDamage(powerForce);
        }
    }
}

#pragma endregion



//�A�j���[�V�����������ݒ�
void PlayerCom::AnimationInitialize()
{
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //�����̃A�j���[�V����
    animator->SetFirstTransition(AnimationPlayer::IDEL_2);
    animator->SetLoopAnimation(AnimationPlayer::IDEL_2, true);

    //�A�j���[�V�����p�����[�^�[�ǉ�
    animator->AddTriggerParameter("jump");
    animator->AddTriggerParameter("punch");
    animator->AddTriggerParameter("bigSwordUp");
    animator->AddTriggerParameter("bigSwordDown");
    animator->AddTriggerParameter("bigSwordRight");
    animator->AddTriggerParameter("bigSwordLeft");
    animator->AddTriggerParameter("dash");
    animator->AddFloatParameter("moveSpeed");

    //�A�j���[�V�����J�ڂƃp�����[�^�[�ݒ�����錈�߂�
    {
        //idle
        animator->AddAnimatorTransition(IDEL_2, WALK_RUNRUN_2);
        animator->SetFloatTransition(IDEL_2, WALK_RUNRUN_2,
            "moveSpeed", 0.1f, ParameterJudge::GREATER);

        //walk
        animator->AddAnimatorTransition(WALK_RUNRUN_2, IDEL_2);
        animator->SetFloatTransition(WALK_RUNRUN_2, IDEL_2,
            "moveSpeed", 0.1f, ParameterJudge::LESS);
        animator->SetLoopAnimation(WALK_RUNRUN_2, true);

        animator->AddAnimatorTransition(WALK_RUNRUN_2, RUN_HARD_2);
        animator->SetFloatTransition(WALK_RUNRUN_2, RUN_HARD_2,
            "moveSpeed", moveParam_[MOVE_PARAM::WALK].moveMaxSpeed + 1, ParameterJudge::GREATER);

        //run
        animator->AddAnimatorTransition(RUN_HARD_2, WALK_RUNRUN_2);
        animator->SetFloatTransition(RUN_HARD_2, WALK_RUNRUN_2,
            "moveSpeed", moveParam_[MOVE_PARAM::WALK].moveMaxSpeed + 1, ParameterJudge::LESS);
        animator->SetLoopAnimation(RUN_HARD_2, true);

        //�ǂ�����ł��J�ڂ���
        //jump
        animator->AddAnimatorTransition(JUMP_2);
        animator->SetTriggerTransition(JUMP_2, "jump");
        animator->AddAnimatorTransition(JUMP_2, IDEL_2, true);

        //punch
        animator->AddAnimatorTransition(PUNCH);
        animator->SetTriggerTransition(PUNCH, "punch");
        animator->AddAnimatorTransition(PUNCH, IDEL_2, true);

        //bigSword
        //up
        animator->AddAnimatorTransition(BIGSWORD_UP);
        animator->SetTriggerTransition(BIGSWORD_UP, "bigSwordUp");
        animator->AddAnimatorTransition(BIGSWORD_UP, IDEL_2, true);
        //down
        animator->AddAnimatorTransition(BIGSWORD_DOWN);
        animator->SetTriggerTransition(BIGSWORD_DOWN, "bigSwordDown");
        animator->AddAnimatorTransition(BIGSWORD_DOWN, IDEL_2, true);
        //right
        animator->AddAnimatorTransition(BIGSWORD_RIGHT);
        animator->SetTriggerTransition(BIGSWORD_RIGHT, "bigSwordRight");
        animator->AddAnimatorTransition(BIGSWORD_RIGHT, IDEL_2, true);
        //left
        animator->AddAnimatorTransition(BIGSWORD_LEFT);
        animator->SetTriggerTransition(BIGSWORD_LEFT, "bigSwordLeft");
        animator->AddAnimatorTransition(BIGSWORD_LEFT, IDEL_2, true);

        //dash
        animator->AddAnimatorTransition(DASH_ANIM);
        animator->SetTriggerTransition(DASH_ANIM, "dash");
        animator->AddAnimatorTransition(DASH_ANIM, IDEL_2, true);

    }
}
