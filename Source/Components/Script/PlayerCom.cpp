#include "PlayerCom.h"
#include "Input/Input.h"
#include "../CameraCom.h"

// �J�n����
void PlayerCom::Start()
{

}

// �X�V����
void PlayerCom::Update(float elapsedTime)
{
    std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
    camera->SetLookAt(GetGameObject()->GetPosition());
}

// GUI�`��
void PlayerCom::OnGUI()
{

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
DirectX::XMFLOAT3 PlayerCom::GetMoveVec() const
{
    //���͏����擾
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();

    return {};
}

//�ړ����͏���
bool PlayerCom::InputMove(float elapsedTime)
{
    return false;
}

