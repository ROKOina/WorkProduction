#include "PlayerCom.h"
#include "Input/Input.h"
#include "Components\CameraCom.h"
#include "Components\TransformCom.h"
#include "Components\AnimationCom.h"
#include <imgui.h>
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"

// �J�n����
void PlayerCom::Start()
{
    //�J�������v���C���[�Ƀt�H�[�J�X����
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");

    //�v���C���[�̃��[���h�|�W�V�������擾
    DirectX::XMFLOAT3 wp = GetGameObject()->transform_->GetWorldPosition();
    wp.z -= 10;
    wp.y += 6;
    cameraObj->transform_->SetPosition(wp);

    //{   //���A�j���[�V����
    //    std::shared_ptr<AnimationCom> anim = GetGameObject()->GetComponent<AnimationCom>();
    //    anim->ImportFbxAnimation("Data/Model/pico/attack3Combo.fbx");
    //    anim->PlayAnimation(3, true);
    //}

    //�p�����[�^�[������
    moveParam_[MOVE_PARAM::WALK].moveMaxSpeed = 3.0f;
    moveParam_[MOVE_PARAM::WALK].moveAcceleration = 0.2f;
    moveParam_[MOVE_PARAM::WALK].turnSpeed =4.0f;

    moveParam_[MOVE_PARAM::RUN].moveMaxSpeed = 8.0f;
    moveParam_[MOVE_PARAM::RUN].moveAcceleration = 0.2f;
    moveParam_[MOVE_PARAM::RUN].turnSpeed =8.0f;

    dashSpeed_ = 15.0f;
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
        cameraObj->transform_->LookAtTransform(GetGameObject()->transform_->GetPosition());

        //�v���C���[�̃��[���h�|�W�V�������擾
        DirectX::XMFLOAT3 wp = GetGameObject()->transform_->GetWorldPosition();
        wp.z -= 10;
        wp.y += 6;
        cameraObj->transform_->SetPosition(wp);

    }

    //�ړ�
    {
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

        //�c�����ړ��X�V
        VerticalUpdate(elapsedTime);
        //�������ړ��X�V
        HorizonUpdate(elapsedTime);

        //���͂��|�W�V�����ɍX�V
        VelocityAppPosition(elapsedTime);
    }

    //�����蔻��
    std::vector<std::shared_ptr<GameObject>> hitGameObj = GetGameObject()->GetComponent<SphereColliderCom>()->OnHitGameObject();
    for (auto& hitObj : hitGameObj)
    {
        if (std::strcmp(hitObj->GetName(), "picolabo") != 0)continue;
        GameObjectManager::Instance().Remove(hitObj);
    }
}

// GUI�`��
void PlayerCom::OnGUI()
{
    ImGui::SliderInt("moveParamType", &moveParamType_, 0, 1);
    int i = 0;
    for (auto& moveP : moveParam_)
    {
        std::string m;
        m = "moveSpeed" + std::to_string(i);
        ImGui::DragFloat(m.c_str(), &moveP.moveMaxSpeed, 0.1f);
        m = "moveAcceleration" + std::to_string(i);
        ImGui::DragFloat(m.c_str(), &moveP.moveAcceleration, 0.1f);
        m = "turnSpeed" + std::to_string(i);
        ImGui::DragFloat(m.c_str(), &moveP.turnSpeed, 0.1f);
        i+=10;
    }

    ImGui::DragFloat("jumpSpeed", &jumpSpeed_, 0.1f);
    ImGui::DragFloat("gravity", &gravity_, 0.1f);
    ImGui::DragFloat("friction", &friction_, 0.1f);

    ImGui::DragFloat3("up", &up_.x);
    ImGui::DragFloat3("velocity", &velocity_.x);

    DirectX::XMFLOAT3 p;
    p = GetGameObject()->transform_->GetUp();
    ImGui::DragFloat3("myUp", &p.x);
    

    ImGui::DragFloat("dashSpeed", &dashSpeed_);
    float s1 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&velocity_)));
    float s2 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&dashVelocity_)));
    float speed = s1 + s2;
    ImGui::InputFloat("speedLook", &speed);

    ImGui::Checkbox("aaa", &aaa);
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
        //���͂��I���ƕ�����
        moveParamType_ = MOVE_PARAM::WALK;
        return false;
    }

    //�_�b�V��
    if (gamePad.GetButton() & GamePad::BTN_LSHIFT)
    {
        moveParamType_ = MOVE_PARAM::RUN;
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
    AddForce({ 0,inputMoveVec_.y,0 });
}

//�������ړ�
void PlayerCom::HorizonMove()
{
    //�O�����Ɉړ�����͂𐶐�
    DirectX::XMFLOAT3 forward = GetGameObject()->transform_->GetFront();
    DirectX::XMVECTOR Forward = DirectX::XMLoadFloat3(&forward);
    DirectX::XMFLOAT3 moveVec;
    DirectX::XMStoreFloat3(&moveVec, DirectX::XMVectorScale(Forward, moveParam_[moveParamType_].moveAcceleration));

    //�͂ɉ�����
    AddForce(moveVec);
}

//�c�����ړ��X�V
void PlayerCom::VerticalUpdate(float elapsedTime)
{
    float gravity= gravity_ * (elapsedTime * Graphics::Instance().GetFPS());
    AddForce({ 0,gravity,0 });

    //�Ƃ肠����0�ȉ��␳
    DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetPosition();
    if (playerPos.y < 0 && velocity_.y < 0)
    {
        velocity_.y = 0;
        playerPos.y = 0;
        GetGameObject()->transform_->SetPosition(playerPos);
    }
}

//�������ړ��X�V
void PlayerCom::HorizonUpdate(float elapsedTime)
{
    DirectX::XMFLOAT3 horizonVelocity = { velocity_.x,0,velocity_.z };
    DirectX::XMVECTOR HorizonVelocity = DirectX::XMLoadFloat3(&horizonVelocity);
    float horiLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(HorizonVelocity));


    //�ő呬�x�ݒ�
    if (horiLength > moveParam_[moveParamType_].moveMaxSpeed)
    {
        DirectX::XMVECTOR MaxSpeed = DirectX::XMVectorScale(DirectX::XMVector3Normalize(HorizonVelocity), moveParam_[moveParamType_].moveMaxSpeed);
        DirectX::XMFLOAT3 newMaxVelocity;
        DirectX::XMStoreFloat3(&newMaxVelocity, MaxSpeed);
        velocity_.x = newMaxVelocity.x;
        velocity_.z = newMaxVelocity.z;
    }

    float friction = friction_ * (elapsedTime * Graphics::Instance().GetFPS());
    //���C��
    if (horiLength > friction)
    {
        DirectX::XMVECTOR FriVelocity = DirectX::XMVectorScale(DirectX::XMVector3Normalize(HorizonVelocity), -friction);
        DirectX::XMFLOAT3 newVelocity;
        DirectX::XMStoreFloat3(&newVelocity, FriVelocity);
        AddForce(newVelocity);
    }
    else
    {
        velocity_.x = 0;
        velocity_.z = 0;
    }
}

//���͂��X�V
void PlayerCom::VelocityAppPosition(float elapsedTime)
{
    DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetPosition();
    DirectX::XMFLOAT3 velocity = velocity_;

    //�_�b�V�����͒ǉ�
    velocity.x += dashVelocity_.x;
    velocity.y += dashVelocity_.y;
    velocity.z += dashVelocity_.z;

    playerPos.x += velocity.x * elapsedTime;
    playerPos.y += velocity.y * elapsedTime;
    playerPos.z += velocity.z * elapsedTime;
    GetGameObject()->transform_->SetPosition(playerPos);
}

//�_�b�V��
void PlayerCom::DashMove(float elapsedTime)
{
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_LSHIFT)
    {
        //�ړ����͂����Z�b�g����
        velocity_.x = 0;
        velocity_.z = 0;

        //���͂��Ă���ꍇ
        if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.z * inputMoveVec_.z > 0)
        {
            dashVelocity_.x = inputMoveVec_.x * dashSpeed_;
            dashVelocity_.z = inputMoveVec_.z * dashSpeed_;

            //�_�b�V�����ɓ��͕����ɂ����ړ��ł���悤�ɁA�p�x��ς���
            QuaternionStruct dashDirection = QuaternionStruct::LookRotation(dashVelocity_, up_);
            GetGameObject()->transform_->SetRotation(dashDirection);
        }
        else
        {
            DirectX::XMFLOAT3 front = GetGameObject()->transform_->GetFront();
            dashVelocity_.x = -front.x * dashSpeed_;
            dashVelocity_.z = -front.z * dashSpeed_;
        }

        isDash_ = true;
    }

    //�_�b�V�����͍X�V
    if (dashVelocity_.x * dashVelocity_.x + dashVelocity_.z * dashVelocity_.z <= 0)return;
    
    DirectX::XMFLOAT3 dash = { dashVelocity_.x,0,dashVelocity_.z };
    DirectX::XMVECTOR Dash = DirectX::XMLoadFloat3(&dash);
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Dash));

    float friction = friction_ * (elapsedTime * Graphics::Instance().GetFPS());

    //���C��
    if (length > friction)
    {
        DirectX::XMVECTOR FriVelocity = DirectX::XMVectorScale(DirectX::XMVector3Normalize(Dash), -friction);
        DirectX::XMFLOAT3 newVelocity;
        DirectX::XMStoreFloat3(&newVelocity, FriVelocity);
        dashVelocity_.x += newVelocity.x;
        dashVelocity_.z += newVelocity.z;
    }
    else
    {
        dashVelocity_.x = 0;
        dashVelocity_.z = 0;
        isDash_ = false;
    }

}

/////////////////////////////////////////