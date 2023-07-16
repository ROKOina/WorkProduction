#include "PlayerCom.h"
#include "Input/Input.h"
#include <imgui.h>
#include "Components\CameraCom.h"
#include "Components\TransformCom.h"
#include "Components\AnimationCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"
int state = -1;
float dashMax = 20;

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

    //{   //���A�j���[�V����
    //    std::shared_ptr<AnimationCom> anim = GetGameObject()->GetComponent<AnimationCom>();
    //    anim->ImportFbxAnimation("Data/Model/pico/attack3Combo.fbx");
    //    anim->PlayAnimation(3, true);
    //}

    //�p�����[�^�[������
    moveParam_[MOVE_PARAM::WALK].moveMaxSpeed = 5.0f;
    moveParam_[MOVE_PARAM::WALK].moveAcceleration = 1.2f;
    moveParam_[MOVE_PARAM::WALK].turnSpeed =4.0f;

    moveParam_[MOVE_PARAM::RUN].moveMaxSpeed = 8.0f;
    moveParam_[MOVE_PARAM::RUN].moveAcceleration = 1.2f;
    moveParam_[MOVE_PARAM::RUN].turnSpeed =8.0f;

    moveParam_[MOVE_PARAM::DASH].moveMaxSpeed = 20.0f;
    moveParam_[MOVE_PARAM::DASH].moveAcceleration = 1.0f;
    moveParam_[MOVE_PARAM::DASH].turnSpeed =8.0f;

    moveParamType_ = MOVE_PARAM::WALK;
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
    move->SetMoveAcceleration(moveParam_[MOVE_PARAM::WALK].moveAcceleration);
    move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::WALK].moveMaxSpeed);

    dashSpeed_ = 20.0f;
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

    //�ړ�
    {
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        //�ړ������͂���Ă�����
        if (IsMove(elapsedTime))
        {
            //�ړ�����
            //��]
            Trun(elapsedTime);
            //�c�����ړ�
            VerticalMove();
            //�������ړ�
            HorizonMove();
        }

        //�_�b�V������
        DashMove(elapsedTime);
    }

    //�����蔻��
    std::vector<HitObj> hitGameObj = GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
    for (auto& hitObj : hitGameObj)
    {
        //���O����
        //if (std::strcmp(hitObj.gameObject->GetName(), "picolabo") != 0)continue;
        //�^�O����
        //�G�Ƃ̓�����i���j
        if (COLLIDER_TAG::Enemy == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
        {

            //�폜
            //GameObjectManager::Instance().Remove(hitObj);

            //�����Ԃ�
            DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetWorldPosition();
            DirectX::XMFLOAT3 hitPos = hitObj.gameObject->transform_->GetWorldPosition();

            DirectX::XMVECTOR PlayerPos = { playerPos.x,playerPos.y,playerPos.z };
            DirectX::XMVECTOR HitPos = { hitPos.x, hitPos.y, hitPos.z };

            DirectX::XMVECTOR ForceNormal = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(HitPos, PlayerPos));
            DirectX::XMVectorSetY(ForceNormal, 0);
            ForceNormal = DirectX::XMVectorScale(ForceNormal, 0.01f);
            DirectX::XMFLOAT3 force;
            DirectX::XMStoreFloat3(&force, ForceNormal);
            hitObj.gameObject->transform_->SetWorldPosition(
                { hitPos.x + force.x,0,hitPos.z + force.z });
        }

        ////�W���X�g���
        //if (COLLIDER_TAG::JustAvoid == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
        //{
        //    GamePad& gamePad = Input::Instance().GetGamePad();
        //    if (gamePad.GetButtonDown() & GamePad::BTN_LSHIFT)
        //    {
        //        //���͂����Z�b�g����
        //        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        //        move->ZeroVelocity();

        //        std::shared_ptr<GameObject> enemy = hitObj.gameObject->GetParent();

        //        DirectX::XMFLOAT3 pos = enemy->transform_->GetWorldPosition();
        //        DirectX::XMFLOAT3 front = enemy->transform_->GetWorldFront();
        //        float dist = 2;
        //        pos = { pos.x - front.x * dist,0,pos.z - front.z * dist };

        //        GetGameObject()->transform_->SetWorldPosition(pos);
        //    }
        //}

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

    ImGui::DragFloat("dashSpeed", &dashSpeed_);
    ImGui::DragFloat("justSpeed", &justSpeed_);
    ImGui::DragFloat("justSpeedTime", &justSpeedTime_);

    ImGui::DragFloat("jumpSpeed", &jumpSpeed_, 0.1f);

    ImGui::DragFloat3("up", &up_.x);

    ImGui::Checkbox("dash", &isDash_);
    ImGui::Checkbox("aaa", &aaa);

    //���͏����擾
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();
    ImGui::DragFloat("x", &ax);
    ImGui::DragFloat("y", &ay);
}




////    �ړ��n����       ////

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
    if (gamePad.GetButtonDown() & GamePad::BTN_SPACE)
    {
        inputMoveVec_.y = jumpSpeed_;
    }

    //�i�s�x�N�g�����[���x�N�g���łȂ��ꍇ�͓��͂��ꂽ
    if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.y * inputMoveVec_.y + inputMoveVec_.z * inputMoveVec_.z <= 0.1f) {
        if (!isDash_)
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
    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();    
    //�����蔻��
    std::vector<HitObj> hitGameObj = GetGameObject()->GetComponent<Collider>()->OnHitGameObject();

    std::shared_ptr<GameObject> enemy;
    bool isJust = false;
    for (auto& hitObj : hitGameObj)
    {
        //�W���X�g��������G��ۑ����W���X�g����t���O���I��
        if (COLLIDER_TAG::JustAvoid == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
        {
            isJust = true;
            enemy = hitObj.gameObject->GetParent();
        }
    }

    //�_�b�V��
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER)
    {
        //�_�b�V�����ɓ��͕����ɂ����ړ��ł���悤�ɁA�p�x��ς���
        QuaternionStruct dashDirection = QuaternionStruct::LookRotation(inputMoveVec_, up_);
        GetGameObject()->transform_->SetRotation(dashDirection);

        state = 0;
    }

    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

    switch (state)
    {
    case 0:
        //�_�b�V���ɕύX
        move->ZeroVelocity();
        moveParamType_ = MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);
        state++;
        break;
    case 1:
        if (length > dashMax)
            state++;
        break;
    case 2:
    {
        float acce = move->GetMoveAcceleration();
        acce -= 2 * elapsedTime;
        move->SetMoveAcceleration(acce);

        if (moveParam_[MOVE_PARAM::RUN].moveMaxSpeed >= length)
            state++;
        break;
    }
    case 3:
        moveParamType_ = MOVE_PARAM::RUN;
        move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
        move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
        state = -1;
        break;
    };


    //GamePad& gamePad = Input::Instance().GetGamePad();
    //if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER)
    //{
    //    //�ړ����͂����Z�b�g����
    //    move->ZeroVelocity();

    //    //�_�b�V���ɕύX
    //    moveParamType_ = MOVE_PARAM::DASH;
    //    move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::DASH].moveMaxSpeed);
    //    move->SetMoveAcceleration(moveParam_[MOVE_PARAM::DASH].moveAcceleration);

    //    //���͂��Ă���ꍇ
    //    DirectX::XMFLOAT3 dashVelocity = { 0,0,0 };
    //    if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0.1f)
    //    {
    //        dashVelocity.x = inputMoveVec_.x * dashSpeed_;
    //        dashVelocity.z = inputMoveVec_.z * dashSpeed_;

    //        //���or�_�b�V��
    //        if (isJust) {
    //            DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    //            GetGameObject()->transform_->SetWorldPosition({
    //                pos.x + inputMoveVec_.x * 3 ,pos.y,pos.z + inputMoveVec_.z * 3 });
    //        }
    //        else
    //            move->AddForce(dashVelocity);

    //        //�_�b�V�����ɓ��͕����ɂ����ړ��ł���悤�ɁA�p�x��ς���
    //        QuaternionStruct dashDirection = QuaternionStruct::LookRotation(dashVelocity, up_);
    //        GetGameObject()->transform_->SetRotation(dashDirection);
    //    }
    //    else
    //    {
    //        DirectX::XMFLOAT3 front = GetGameObject()->transform_->GetWorldFront();
    //        dashVelocity.x = -front.x * dashSpeed_ * 2;
    //        dashVelocity.z = -front.z * dashSpeed_ * 2;

    //        //���or�_�b�V��
    //        if (isJust) {
    //            DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    //            GetGameObject()->transform_->SetWorldPosition({
    //                pos.x - front.x * 3 ,pos.y,pos.z - front.z * 3 });
    //        }
    //        else
    //            move->AddForce(dashVelocity);

    //        move->SetMoveMaxSpeed(dashSpeed_ * 2);
    //    }


    //    isDash_ = true;
    //}

    //if (!isDash_)return;



    ////�_�b�V�����͍X�V
    //DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());

    //float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));

    ////RUN�ɐ؂�ւ�
    //if (length < moveParam_[MOVE_PARAM::RUN].moveMaxSpeed)
    //{
    //    moveParamType_ = MOVE_PARAM::RUN;
    //    move->SetMoveMaxSpeed(moveParam_[MOVE_PARAM::RUN].moveMaxSpeed);
    //    move->SetMoveAcceleration(moveParam_[MOVE_PARAM::RUN].moveAcceleration);
    //    isDash_ = false;

    //}

}

/////////////////////////////////////////