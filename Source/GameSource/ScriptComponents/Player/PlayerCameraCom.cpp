#include "PlayerCameraCom.h"

#include <imgui.h>
#include "Components\TransformCom.h"
#include "Input/Input.h"
#include "GameSource/Math/Collision.h"

// �J�n����
void PlayerCameraCom::Start()
{
    angleX_ = 30;
    oldCameraPos_= GetGameObject()->transform_->GetWorldPosition();
}


// �X�V����
void PlayerCameraCom::Update(float elapsedTime)
{
    //�J�������t�H�[�J�X����
    DirectX::XMFLOAT3 focusPos = GetGameObject()->transform_->GetWorldPosition();
    focusPos.y += 1;
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");

    //���͏����擾
    GamePad& gamePad = Input::Instance().GetGamePad();
    angleY_ += gamePad.GetAxisRX() * 180.0f * elapsedTime;
    angleX_ -= gamePad.GetAxisRY() * 100.0f * elapsedTime;


    //�p�x����(�c�̂�)
    if (angleX_ > 65) {
        angleX_ = 65;
    }
    if (angleX_ < -65) {
        angleX_ = -65;
    }

    //�����W��߂�
    range_ = 4;

    //�J������]�l����]�s��ɕϊ�
    DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(angleX_), DirectX::XMConvertToRadians(angleY_), 0);

    //��]�s�񂩂�O�����x�N�g�������o��
    DirectX::XMVECTOR Front = Transform.r[2];
    DirectX::XMFLOAT3 front;
    DirectX::XMStoreFloat3(&front, Front);
    DirectX::XMVECTOR FocusPos = DirectX::XMLoadFloat3(&focusPos);

    DirectX::XMFLOAT3 cameraPos;
    //�����_������x�N�g�������Ɉ�苗�����ꂽ�J�������_�����߂�
    DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(FocusPos, DirectX::XMVectorScale(Front, range_)));

    // �X���[�Y�ȃJ�����ړ�
    DirectX::XMStoreFloat3(&oldCameraPos_, DirectX::XMVectorLerp(XMLoadFloat3(&cameraPos), DirectX::XMLoadFloat3(&oldCameraPos_), 1.0f - 0.08f));

    // �V�����ڕW�� Target ���v�Z
    DirectX::XMStoreFloat3(&focusPos, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&cameraPos), DirectX::XMLoadFloat3(&focusPos), 1.0f - 0.09f));

    //�J�����߂荞�݉������
    DirectX::XMFLOAT3 kabePlus = GameObjectManager::Instance().Find("kabePlus")->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 kabeMinas = GameObjectManager::Instance().Find("kabeMinas")->transform_->GetWorldPosition();
    //�␳
    kabePlus = { kabePlus.x + 1,0,kabePlus.z + 1 };
    kabeMinas = { kabeMinas.x - 1,0,kabeMinas.z - 1 };

    while (oldCameraPos_.y < 0 ||   //�n��
        oldCameraPos_.x > kabePlus.x || oldCameraPos_.x < kabeMinas.x ||    //��
        oldCameraPos_.z > kabePlus.z || oldCameraPos_.z < kabeMinas.z)
    {
        range_ -= 0.01f;
        DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(FocusPos, DirectX::XMVectorScale(Front, range_)));
        DirectX::XMStoreFloat3(&oldCameraPos_, DirectX::XMVectorLerp(XMLoadFloat3(&cameraPos), DirectX::XMLoadFloat3(&oldCameraPos_), 1.0f - 0.08f));

        if (range_ <= 0.1f)break;
    }

    //�ʒu���
    cameraObj->transform_->SetWorldPosition(oldCameraPos_);
    cameraObj->transform_->LookAtTransform(focusPos);
}


// GUI�`��
void PlayerCameraCom::OnGUI()
{
    ImGui::DragFloat("angleX", &angleX_, 0.1f);
    ImGui::DragFloat("angleY", &angleY_, 0.1f);
    ImGui::DragFloat("range", &range_, 0.1f);
}

