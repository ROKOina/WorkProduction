#include "PlayerCom.h"
#include "Input/Input.h"
#include "../CameraCom.h"
#include "Components\TransformCom.h"
#include <imgui.h>

// �J�n����
void PlayerCom::Start()
{

}

// �X�V����
void PlayerCom::Update(float elapsedTime)
{
    //�J�������v���C���[�Ƀt�H�[�J�X����
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");
    std::shared_ptr<CameraCom> camera = cameraObj->GetComponent<CameraCom>();
    camera->SetLookAt(GetGameObject()->transform->GetPosition());

    //�v���C���[�̃��[���h�|�W�V�������擾
    DirectX::XMFLOAT3 wp = GetGameObject()->transform->GetWorldPosition();
    wp.z -= 10;
    wp.y += 5;
    cameraObj->transform->SetPosition(wp);

    InputMove(elapsedTime);

    //���͏����X�V
    DirectX::XMFLOAT3 p = GetGameObject()->transform->GetPosition();
    DirectX::XMFLOAT4 r = GetGameObject()->transform->GetRotation();
    //UpdateVelocity(elapsedTime, p, r);
    UpdateVelocity(elapsedTime, p, DirectX::XMFLOAT4(0, 0, 0, 0));
    GetGameObject()->transform->SetPosition(p);
    GetGameObject()->transform->SetRotation(r);
}

// GUI�`��
void PlayerCom::OnGUI()
{
    DirectX::XMFLOAT3 a = GetGameObject()->transform->GetUp();
    ImGui::InputFloat3("up", &a.x);
    ImGui::DragFloat("turnSpeed", &turnSpeed);
}

//�J����
void PlayerCom::CameraControll(float elapsedTime)
{
    //GamePad& gamePad = Input::Instance().GetGamePad();
    //float ax = gamePad.GetAxisRX();
    //float ay = gamePad.GetAxisRY();
    ////�J�����̉�]���x
    //float speed = rollSpeed * elapsedTime;

    ////�X�e�B�b�N�̓��͒l�����킹��X����Y������]
    //if (ay > 0)
    //{
    //    angle.x += speed;
    //}
    //if (ay < 0)
    //{
    //    angle.x -= speed;
    //}
    //if (ax > 0)
    //{
    //    angle.y += speed;
    //}
    //if (ax < 0)
    //{
    //    angle.y -= speed;
    //}

    ////X���̃J������]�𐧌�
    //if (angle.x > maxAngleX)
    //{
    //    angle.x = maxAngleX;
    //}
    //if (angle.x < minAngleX)
    //{
    //    angle.x = minAngleX;
    //}

    ////Y���̃J������]�l��-3.14�`3.14�Ɏ��܂�悤�ɂ���
    //if (angle.y < -DirectX::XM_PI)
    //{
    //    angle.y += DirectX::XM_2PI;
    //}
    //if (angle.y > DirectX::XM_PI)
    //{
    //    angle.y -= DirectX::XM_2PI;
    //}

    ////�J������]�l����]�s��ɕϊ�
    //DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);

    ////��]�s�񂩂�O�����x�N�g�������o��
    //DirectX::XMVECTOR Front = Transform.r[2];
    //DirectX::XMFLOAT3 front;
    //DirectX::XMStoreFloat3(&front, Front);

    ////�����_������x�N�g�������Ɉ�苗�����ꂽ�J�������_�����߂�
    //DirectX::XMFLOAT3 eye;
    //eye.x = target.x - front.x * range;
    //eye.y = target.y - front.y * range;
    //eye.z = target.z - front.z * range;

    //std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
    //camera->set
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
bool PlayerCom::InputMove(float elapsedTime)
{
    //�i�s�x�N�g���擾
    DirectX::XMFLOAT3 moveVec = GetMoveVec();

    //�ړ�����
    Move(moveVec.x, moveVec.z, moveSpeed);

    //���񏈗�
    DirectX::XMFLOAT4 r = GetGameObject()->transform->GetRotation();
    Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed, r, GetGameObject()->transform->GetUp());
    //Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed, DirectX::XMFLOAT4(0, 0, 0, 0), { 0,1,0 });
    GetGameObject()->transform->SetRotation(r);

    //�i�s�x�N�g�����[���x�N�g���łȂ��ꍇ�͓��͂��ꂽ
    return moveVec.x * moveVec.x + moveVec.y * moveVec.y + moveVec.z * moveVec.z;
}

