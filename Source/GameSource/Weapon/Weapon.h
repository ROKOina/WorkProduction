#pragma once

#include <DirectXMath.h>
#include "Graphics\Model.h"
#include "Graphics/Shader.h"

class Weapon
{
public:
    Weapon(const char* filename, DirectX::XMFLOAT3 Scale = { 1,1,1 });
    ~Weapon(){}

    void Update(float elapsedTime);
    void Render(ID3D11DeviceContext* dc, Shader* shader);

    void ParentTransform(DirectX::XMFLOAT4X4 parent4X4);

    void SetAngle(DirectX::XMFLOAT3 weaponAngle) { angle = weaponAngle; }

private:
    std::unique_ptr<Model> model;
    DirectX::XMFLOAT4X4 parentTransform;
    DirectX::XMFLOAT3   scale = { 1,1,1 };
    DirectX::XMFLOAT3   angle = { 0,0,0 };
    DirectX::XMFLOAT3   offsetPosition = { 0,0,0 };
};