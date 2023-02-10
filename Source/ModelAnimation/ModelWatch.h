#pragma once

//
// モデルのアニメーションを見れるクラス
//

#include "CameraController.h"
#include <memory>
class ModelWatch
{
public:

    ModelWatch();
    ~ModelWatch() {}

    void Update(float elapsedTime);
    void Render(float elapsedTime);

private:
    std::unique_ptr<CameraController> cameraController;
    DirectX::XMFLOAT2 cameraAngle;
    DirectX::XMFLOAT3 cameraEye;
};