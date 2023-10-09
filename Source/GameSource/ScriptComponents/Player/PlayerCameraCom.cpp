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
    lerpFocusPos_ = GetGameObject()->transform_->GetWorldPosition();
}


// �X�V����
void PlayerCameraCom::Update(float elapsedTime)
{
    //���͏����擾
    GamePad& gamePad = Input::Instance().GetGamePad();
    angleY_ += gamePad.GetAxisRX() * 180.0f * elapsedTime;
    angleX_ -= gamePad.GetAxisRY() * 100.0f * elapsedTime;


    //�p�x����(�c�̂�)
    if (angleX_ > angleLimit_) {
        angleX_ = angleLimit_;
    }
    if (angleX_ < -angleLimit_) {
        angleX_ = -angleLimit_;
    }

    //�J�������v���C���[�Ƀt�H�[�J�X����
    DirectX::XMFLOAT3 focusPos = GetGameObject()->transform_->GetWorldPosition();
    focusPos.y += 1;

    DirectX::XMVECTOR FocusPos = DirectX::XMLoadFloat3(&focusPos);

    //�����J�����̃t�H�[�J�X��x�点��
    {
        DirectX::XMStoreFloat3(&lerpFocusPos_, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&lerpFocusPos_), FocusPos, 0.1f * lerpSpeed_));
    }

    //�J������]�l����]�s��ɕϊ�
    DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(angleX_), DirectX::XMConvertToRadians(angleY_), 0);

    //��]�s�񂩂�O�����x�N�g�������o��
    DirectX::XMVECTOR Front = Transform.r[2];
    DirectX::XMFLOAT3 front;
    DirectX::XMStoreFloat3(&front, Front);

    //�����W����
    float range = range_;

    DirectX::XMFLOAT3 cameraPos;
    //�����_������x�N�g�������Ɉ�苗�����ꂽ�J�������_�����߂�
    DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&lerpFocusPos_), DirectX::XMVectorScale(Front, range)));
    //DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(FocusPos, DirectX::XMVectorScale(Front, range_)));

    //// �X���[�Y�ȃJ�����ړ�
    //{
    //    DirectX::XMStoreFloat3(&oldCameraPos_, DirectX::XMVectorLerp(XMLoadFloat3(&cameraPos), DirectX::XMLoadFloat3(&oldCameraPos_), 1.0f - 0.08f));

    //    // �V�����ڕW�� Target ���v�Z
    //    DirectX::XMStoreFloat3(&focusPos, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&cameraPos), DirectX::XMLoadFloat3(&focusPos), 1.0f - 0.09f));
    //}

    //�J�����߂荞�݉������
    {
        DirectX::XMFLOAT3 kabePlus = GameObjectManager::Instance().Find("kabePlus")->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 kabeMinas = GameObjectManager::Instance().Find("kabeMinas")->transform_->GetWorldPosition();
        //�␳
        kabePlus = { kabePlus.x + 1,0,kabePlus.z + 1 };
        kabeMinas = { kabeMinas.x - 1,0,kabeMinas.z - 1 };

        while (cameraPos.y < 0 ||   //�n��
            cameraPos.x > kabePlus.x || cameraPos.x < kabeMinas.x ||    //��
            cameraPos.z > kabePlus.z || cameraPos.z < kabeMinas.z)
        {
            range -= 0.01f;
            DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&lerpFocusPos_), DirectX::XMVectorScale(Front, range)));

            //DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(FocusPos, DirectX::XMVectorScale(Front, range)));
            //DirectX::XMStoreFloat3(&oldCameraPos_, DirectX::XMVectorLerp(XMLoadFloat3(&cameraPos), DirectX::XMLoadFloat3(&oldCameraPos_), 1.0f - 0.08f));

            if (range <= 0.1f)break;
        }
    }

    //�v���C���[���O�ɃJ�������s���Ȃ�����
    if (angleX_ < -30)
    {
        while (1)
        {
            //���[�v�Ȃ��̃J�����̈ʒu
            DirectX::XMVECTOR C = DirectX::XMVectorSubtract(FocusPos, DirectX::XMVectorScale(Front, range));

            //���[�v�Ȃ��̃J�����ƃv���C���[�̃x�N�g��
            DirectX::XMVECTOR NonLVec = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(C, FocusPos));
            //���[�v����̃J�����ƃv���C���[�̃x�N�g��
            DirectX::XMVECTOR LVec = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&cameraPos), FocusPos));

            float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(NonLVec, LVec));
            if (dot > 0.9f) //�O�ɍs���Ă��Ȃ��̂�OK
                break;

            //�␳
            DirectX::XMStoreFloat3(&lerpFocusPos_, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&lerpFocusPos_), FocusPos, 0.1f * lerpSpeed_));
            DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&lerpFocusPos_), DirectX::XMVectorScale(Front, range)));
        }
    }
    oldCameraPos_ = cameraPos;

    //�J�����ɒl����
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");
    cameraObj->transform_->SetWorldPosition(cameraPos);
    //cameraObj->transform_->SetWorldPosition(oldCameraPos_);
    cameraObj->transform_->LookAtTransform(focusPos);
}


// GUI�`��
void PlayerCameraCom::OnGUI()
{
    ImGui::DragFloat("angleX", &angleX_, 0.1f);
    ImGui::DragFloat("angleY", &angleY_, 0.1f);
    ImGui::DragFloat("angleLimit_", &angleLimit_, 0.1f);
    ImGui::DragFloat("range", &range_, 0.1f);
    ImGui::DragFloat("lerpSpeed_", &lerpSpeed_, 0.1f);
}

