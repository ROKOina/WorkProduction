#include "Weapon.h"

Weapon::Weapon(const char* filename, DirectX::XMFLOAT3 Scale)
    :model(std::make_unique<Model>(filename))
    ,scale(Scale)
{
    DirectX::XMStoreFloat4x4(&parentTransform, DirectX::XMMatrixIdentity());
}

void Weapon::Update(float elapsedTime)
{
    //変換行列
    //スケール行列を作成
    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

    //回転行列を作成
    DirectX::XMMATRIX X = DirectX::XMMatrixRotationX(angle.x);
    DirectX::XMMATRIX Y = DirectX::XMMatrixRotationY(angle.y);
    DirectX::XMMATRIX Z = DirectX::XMMatrixRotationZ(angle.z);
    DirectX::XMMATRIX R = X * Y * Z;

    //位置行列を作成
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(offsetPosition.x, offsetPosition.y, offsetPosition.z);

    //3つの行列を組み合わせて、ワールド行列を作成
    DirectX::XMMATRIX W = S * R * T;


    //変換行列とオリジナルをかけ合わせる
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
