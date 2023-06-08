#pragma once

#include "System\Component.h"

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
    void SetPosition(const DirectX::XMFLOAT3& position) { this->position = position; }
    const DirectX::XMFLOAT3& GetPosition() const { return position; }

    // ���[���h�|�W�V����
    void SetWorldPosition(const DirectX::XMFLOAT3& worldPosition) { this->worldPosition = worldPosition; }
    const DirectX::XMFLOAT3& GetWorldPosition() const { return worldPosition; }

    // �N�H�[�^�j�I����]
    void SetRotation(const DirectX::XMFLOAT4& rotation) { this->rotation = rotation; }
    const DirectX::XMFLOAT4& GetRotation() const { return rotation; }

    // �I�C���[��](�N�H�[�^�j�I��->�I�C���[�͂����̂ŃZ�b�g����)
    void SetEulerRotation(const DirectX::XMFLOAT3& setEuler) { 
        DirectX::XMFLOAT3 euler;
        euler.x = DirectX::XMConvertToRadians(setEuler.x);
        euler.y = DirectX::XMConvertToRadians(setEuler.y);
        euler.z = DirectX::XMConvertToRadians(setEuler.z);
        DirectX::XMVECTOR ROT = DirectX::XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
        DirectX::XMStoreFloat4(&rotation, ROT);
        euler.y = DirectX::XMConvertToDegrees(euler.y);
        euler.x = DirectX::XMConvertToDegrees(euler.x);
        euler.z = DirectX::XMConvertToDegrees(euler.z);
        eulerRotation = euler;
    }


    // �X�P�[��
    void SetScale(const DirectX::XMFLOAT3& scale) { this->scale = scale; }
    const DirectX::XMFLOAT3& GetScale() const { return scale; }

    //���[�J���s��
    void SetParentTransform(const DirectX::XMFLOAT4X4& parentTransform) { this->parentTransform = parentTransform; }
    const DirectX::XMFLOAT4X4& GetParentTransform() const { return parentTransform; }

    //���[���h�s��
    void SetTransform(const DirectX::XMFLOAT4X4& transform) { this->transform = transform; }
    const DirectX::XMFLOAT4X4& GetTransform() const { return transform; }
    const DirectX::XMFLOAT3& GetUp() const {
        DirectX::XMFLOAT3 up = {transform._21, transform._22, transform._23};
        DirectX::XMStoreFloat3(&up, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&up)));
        return up; 
    }
    const DirectX::XMFLOAT3& GetFront() const {
        DirectX::XMFLOAT3 front = {transform._31, transform._32, transform._33};
        DirectX::XMStoreFloat3(&front, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&front)));
        return front;
    }
    const DirectX::XMFLOAT3& GetRight() const {
        DirectX::XMFLOAT3 right = {transform._11, transform._12, transform._13};
        DirectX::XMStoreFloat3(&right, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&right)));
        return right;
    }

    //�w�����������
    void LookAtTransform(const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up = { 0,1,0 });

    //�w���Up�ɍ��킹��
    void SetUpTransform(const DirectX::XMFLOAT3& up);

private:
    DirectX::XMFLOAT3	position = DirectX::XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT3	worldPosition = DirectX::XMFLOAT3(0, 0, 0);

    DirectX::XMFLOAT4	rotation = DirectX::XMFLOAT4(0, 0, 0, 1);   //��̓N�H�[�^�j�I��
    DirectX::XMFLOAT3   eulerRotation = DirectX::XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT3	scale = DirectX::XMFLOAT3(1, 1, 1);
    DirectX::XMFLOAT4X4	transform = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    DirectX::XMFLOAT4X4	parentTransform = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
};

