#include "Weapon.h"

Weapon::Weapon(const char* filename, DirectX::XMFLOAT3 Scale)
    :model(std::make_unique<Model>(filename))
    ,scale(Scale)
{
    DirectX::XMStoreFloat4x4(&parentTransform, DirectX::XMMatrixIdentity());
}

void Weapon::Update(float elapsedTime)
{
    //�ϊ��s��
    //�X�P�[���s����쐬
    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

    //��]�s����쐬
    DirectX::XMMATRIX X = DirectX::XMMatrixRotationX(angle.x);
    DirectX::XMMATRIX Y = DirectX::XMMatrixRotationY(angle.y);
    DirectX::XMMATRIX Z = DirectX::XMMatrixRotationZ(angle.z);
    DirectX::XMMATRIX R = X * Y * Z;

    //�ʒu�s����쐬
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(offsetPosition.x, offsetPosition.y, offsetPosition.z);

    //3�̍s���g�ݍ��킹�āA���[���h�s����쐬
    DirectX::XMMATRIX W = S * R * T;


    //�ϊ��s��ƃI���W�i�����������킹��
    DirectX::XMFLOAT4X4 newParentT;
    DirectX::XMStoreFloat4x4(&newParentT, W*DirectX::XMLoadFloat4x4(&parentTransform));

    model->UpdateTransform(newParentT);
}

void Weapon::Render(ID3D11DeviceContext* dc, Shader* shader)
{
    shader->Draw(dc, model.get());
}

void Weapon::ParentTransform(DirectX::XMFLOAT4X4 parent4X4)
{
    parentTransform = parent4X4;
}
