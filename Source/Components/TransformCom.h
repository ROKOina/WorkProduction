#pragma once

#include "System\Component.h"
#include "SystemStruct\QuaternionStruct.h"

class TransformCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    TransformCom() {}
    ~TransformCom() {}

    // ���O�擾
    const char* GetName() const override { return "Transform"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;


    //TransformCom�N���X
public:
    //�s��X�V
    void UpdateTransform();

    // ���[�J���|�W�V����
    void SetLocalPosition(const DirectX::XMFLOAT3& position) { this->localPosition_ = position; }
    const DirectX::XMFLOAT3& GetLocalPosition() const { return localPosition_; }

    // ���[���h�|�W�V����
    void SetWorldPosition(const DirectX::XMFLOAT3& worldPosition) {
        //�e�����Ȃ����͂��̂܂܃|�W�V�����ɂȂ�
        if (!GetGameObject()->GetParent()) {
            this->localPosition_ = worldPosition;
        }
        else{
            DirectX::XMMATRIX ParentTransformInverse = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&GetGameObject()->GetParent()->transform_->GetWorldTransform()));
            DirectX::XMVECTOR WorldPosition = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&worldPosition), ParentTransformInverse);
            DirectX::XMStoreFloat3(&this->localPosition_, WorldPosition);
        }
    }
    const DirectX::XMFLOAT3& GetWorldPosition() const { return worldPosition_; }

    // �N�H�[�^�j�I����]
    void SetRotation(const DirectX::XMFLOAT4& rotation) { this->rotation_ = rotation; }
    const DirectX::XMFLOAT4& GetRotation() const { return rotation_; }

    // �I�C���[��](�N�H�[�^�j�I��->�I�C���[�͂����̂ŃZ�b�g����)
    void SetEulerRotation(const DirectX::XMFLOAT3& setEuler) { 
        DirectX::XMFLOAT3 euler;
        euler.x = DirectX::XMConvertToRadians(setEuler.x);
        euler.y = DirectX::XMConvertToRadians(setEuler.y);
        euler.z = DirectX::XMConvertToRadians(setEuler.z);
        DirectX::XMVECTOR ROT = DirectX::XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
        DirectX::XMStoreFloat4(&rotation_.dxFloat4, ROT);
        euler.y = DirectX::XMConvertToDegrees(euler.y);
        euler.x = DirectX::XMConvertToDegrees(euler.x);
        euler.z = DirectX::XMConvertToDegrees(euler.z);
        eulerRotation_ = euler;
    }

    // �X�P�[��
    void SetScale(const DirectX::XMFLOAT3& scale) { this->scale_ = scale; }
    const DirectX::XMFLOAT3& GetScale() const { return scale_; }

    //�e�s��
    void SetParentTransform(const DirectX::XMFLOAT4X4& parentTransform) { this->parentTransform_ = parentTransform; }
    const DirectX::XMFLOAT4X4& GetParentTransform() const { return parentTransform_; }

    //���[�J���s��
    const DirectX::XMFLOAT4X4& GetLocalTransform() const { return localTransform_; }

    //���[���h�s��
    void SetWorldTransform(const DirectX::XMFLOAT4X4& transform) { this->worldTransform_ = transform; }
    const DirectX::XMFLOAT4X4& GetWorldTransform() const { return worldTransform_; }
    const DirectX::XMFLOAT3& GetWorldUp() const {
        DirectX::XMFLOAT3 up = {worldTransform_._21, worldTransform_._22, worldTransform_._23};
        DirectX::XMStoreFloat3(&up, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&up)));
        return up; 
    }
    const DirectX::XMFLOAT3& GetWorldFront() const {
        DirectX::XMFLOAT3 front = {worldTransform_._31, worldTransform_._32, worldTransform_._33};
        DirectX::XMStoreFloat3(&front, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&front)));
        return front;
    }
    const DirectX::XMFLOAT3& GetWorldRight() const {
        DirectX::XMFLOAT3 right = {worldTransform_._11, worldTransform_._12, worldTransform_._13};
        DirectX::XMStoreFloat3(&right, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&right)));
        return right;
    }

    //�w�����������
    void LookAtTransform(const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up = { 0,1,0 });

    //�w���Up�ɍ��킹��
    void SetUpTransform(const DirectX::XMFLOAT3& up);

private:
    DirectX::XMFLOAT3	localPosition_ = DirectX::XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT3	worldPosition_ = DirectX::XMFLOAT3(0, 0, 0);

    QuaternionStruct	rotation_ = DirectX::XMFLOAT4(0, 0, 0, 1);   //��̓N�H�[�^�j�I��
    DirectX::XMFLOAT3   eulerRotation_ = DirectX::XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT3	scale_ = DirectX::XMFLOAT3(1, 1, 1);
    DirectX::XMFLOAT4X4	worldTransform_ = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    DirectX::XMFLOAT4X4	localTransform_ = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    DirectX::XMFLOAT4X4 parentTransform_ = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
};

