#include "PlayerCom.h"
#include "Input/Input.h"
#include "Components\CameraCom.h"
#include "Components\TransformCom.h"
#include "Components\AnimationCom.h"
#include <imgui.h>
#include "Components\RendererCom.h"

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

    {   //���A�j���[�V����
        std::shared_ptr<AnimationCom> anim = GetGameObject()->GetComponent<AnimationCom>();
        anim->ImportFbxAnimation("Data/Model/pico/attack3Combo.fbx");
        anim->PlayAnimation(3, true);
    }
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
        Move(elapsedTime);

        //���͏����X�V
        DirectX::XMFLOAT3 p = GetGameObject()->transform_->GetPosition();
        DirectX::XMFLOAT4 r = GetGameObject()->transform_->GetRotation();
        //UpdateVelocity(elapsedTime, p, r);
        //UpdateVelocity(elapsedTime, p, DirectX::XMFLOAT4(0, 0, 0, 0),up_);
        GetGameObject()->transform_->SetPosition(p);
        GetGameObject()->transform_->SetRotation(r);
    }

    //����]
    {
        //DirectX::XMFLOAT3 pos = GameObjectManager::Instance().Find("miru")->transform->GetPosition();
        //GetGameObject()->transform->LookAtTransform(pos, { 0,1,0 });
    }
}

// GUI�`��
void PlayerCom::OnGUI()
{
    ImGui::DragFloat("moveSpeed", &moveSpeed_);
    ImGui::DragFloat("turnSpeed", &turnSpeed_);

    ImGui::DragFloat3("lookP", &look_.x);
    ImGui::DragFloat3("up", &up_.x);

    DirectX::XMFLOAT3 p;
    p = GetGameObject()->transform_->GetUp();
    ImGui::DragFloat3("myUp", &p.x);
    ImGui::Checkbox("aaa", &aaa);
}


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
bool PlayerCom::Move(float elapsedTime)
{
    //�i�s�x�N�g���擾
    inputMoveVec_ = GetMoveVec();

    //�i�s�x�N�g�����[���x�N�g���łȂ��ꍇ�͓��͂��ꂽ
    if (inputMoveVec_.x * inputMoveVec_.x + inputMoveVec_.y * inputMoveVec_.y + inputMoveVec_.z * inputMoveVec_.z <= 0.1f) {
        return false;
    }

    ////�ړ�����
    //Move(moveVec.x, moveVec.z, moveSpeed_);


    //���񏈗����n�ʂ̖@���ŉ�]
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    look_ = { inputMoveVec_.x * 3 + pos.x
        ,pos.y
        ,inputMoveVec_.z * 3 + pos.z };

    //���͕����̃N�H�[�^�j�I������
    QuaternionStruct inputQuaternion = QuaternionStruct::LookRotation(inputMoveVec_, up_);
    QuaternionStruct playerQuaternion = GetGameObject()->transform_->GetRotation();

    //DirectX::XMQuaternionSlerp()

    //GetGameObject()->transform_->LookAtTransform(look_, up_);
    //GetGameObject()->transform_->SetUpTransform(up_);

    return true;
}