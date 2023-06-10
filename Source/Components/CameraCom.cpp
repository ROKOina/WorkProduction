#include "CameraCom.h"

#include "TransformCom.h"
#include <imgui.h>
#include <cmath>

// �J�n����
void CameraCom::Start()
{

}


// �X�V����
void CameraCom::Update(float elapsedTime)
{
    //LookAt�֐��g���Ă��Ȃ��Ȃ�X�V����
    if (!isLookAt_) {
        //�J�����̃t�H���[�h���t�H�[�J�X����
        DirectX::XMFLOAT3 forwardPoint;
        DirectX::XMFLOAT3 wPos = GetGameObject()->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 forwardNormalVec = GetGameObject()->transform_->GetFront();
        forwardPoint = { forwardNormalVec.x * 2 + wPos.x,
            forwardNormalVec.y * 2 + wPos.y,
            forwardNormalVec.z * 2 + wPos.z };

        SetLookAt(forwardPoint, GetGameObject()->transform_->GetUp());
    }

    isLookAt_ = false;
}

// GUI�`��
void CameraCom::OnGUI()
{
    ImGui::DragFloat3("Focus", &focus_.x);
}

//�w�����������
void CameraCom::SetLookAt(const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up)
{
    //���_�A�����_�A���������r���[�s����쐬
    DirectX::XMFLOAT3 cameraPos = GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMVECTOR Eye = DirectX::XMLoadFloat3(&cameraPos);
    DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
    DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);

    //�������W�̏ꍇ�A�������炷
    if (focus.x == cameraPos.x && focus.y == cameraPos.y && focus.z == cameraPos.z)
    {
        cameraPos.y += 0.0001f;
        Eye = DirectX::XMLoadFloat3(&cameraPos);
    }

    DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
    DirectX::XMStoreFloat4x4(&view_, View);

    //�r���[���t�s�񉻂��A���[���h���W�ɖ߂�
    DirectX::XMMATRIX World = DirectX::XMMatrixInverse(nullptr, View);
    DirectX::XMFLOAT4X4 world;
    DirectX::XMStoreFloat4x4(&world, World);

    if (!std::isfinite(world._11))return;

    //�J�����̕��������o��
    this->right_.x   = world._11;
    this->right_.y   = world._12;
    this->right_.z   = world._13;

    this->up_.x      = world._21;
    this->up_.y      = world._22;
    this->up_.z      = world._23;

    this->front_.x   = world._31;
    this->front_.y   = world._32;
    this->front_.z   = world._33;

    GetGameObject()->transform_->SetTransform(world);


    //���_�A�����_��ۑ�
    this->focus_ = focus;

    isLookAt_ = true;
}

//�p�[�X�y�N�e�B�u�ݒ�
void CameraCom::SetPerspectiveFov(float fovY, float aspect, float nearZ, float farZ)
{
    //��p�A��ʔ䗦�A�N���b�v��������v���W�F�N�V�����s����쐬
    DirectX::XMMATRIX Projection = DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);	//�v���W�F�N�V�����s��쐬
    DirectX::XMStoreFloat4x4(&projection_, Projection);	//rc�ɓn��
}