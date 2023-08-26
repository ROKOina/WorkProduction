#include "PlayerCameraCom.h"

#include <imgui.h>
#include "Components\TransformCom.h"
#include "Input/Input.h"

// �J�n����
void PlayerCameraCom::Start()
{
    angleX_ = 30;
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
    if (angleX_ > 85) {
        angleX_ = 85;
    }
    if (angleX_ < -85) {
        angleX_ = -85;
    }

    //�J������]�l����]�s��ɕϊ�
    DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(angleX_), DirectX::XMConvertToRadians(angleY_), 0);

    //��]�s�񂩂�O�����x�N�g�������o��
    DirectX::XMVECTOR Front = Transform.r[2];
    DirectX::XMVECTOR FocusPos = DirectX::XMLoadFloat3(&focusPos);
    DirectX::XMFLOAT3 cameraPos;
    //�����_������x�N�g�������Ɉ�苗�����ꂽ�J�������_�����߂�
    DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(FocusPos, DirectX::XMVectorScale(Front, range_)));

    //�ʒu���
    cameraObj->transform_->SetWorldPosition(cameraPos);
    cameraObj->transform_->LookAtTransform(focusPos);
}


// GUI�`��
void PlayerCameraCom::OnGUI()
{
    ImGui::DragFloat("angleX", &angleX_, 0.1f);
    ImGui::DragFloat("angleY", &angleY_, 0.1f);
    ImGui::DragFloat("range", &range_, 0.1f);
}

