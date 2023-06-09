#pragma once

//
// ���f���̃A�j���[�V�����������N���X
//
#include <DirectXMath.h>
#include <memory>
class ModelWatch
{
public:

    ModelWatch();
    ~ModelWatch() {}

    void Update(float elapsedTime);
    void Render(float elapsedTime);

private:
    DirectX::XMFLOAT2 cameraAngle;
    DirectX::XMFLOAT3 cameraEye;
};