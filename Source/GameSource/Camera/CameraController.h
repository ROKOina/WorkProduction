#pragma once

#include <DirectXMath.h>

#include "../Source/Components/CameraCom.h"

//カメラコントローラー
class CameraController
{
public:
    CameraController(){}
    ~CameraController(){}

    //更新処理
    void Update(float elapsedTime);

    //ターゲット位置設定
    void SetTarget(const DirectX::XMFLOAT3& target) { this->target = target; }

    //カメラ
    std::shared_ptr<CameraCom> GetCamera() { return camera; }
    void SetCamera(std::shared_ptr<CameraCom> c) { camera = c; }


    //ImGui
    void DrawDebugGUI();

private:
    DirectX::XMFLOAT3   target = { 0,0,0 };
    DirectX::XMFLOAT3   angle = { 0,0,0 };
    float               rollSpeed = DirectX::XMConvertToRadians(90);
    float               range = 10.0f;
    float               maxAngleX = DirectX::XMConvertToRadians(45);
    float               minAngleX = DirectX::XMConvertToRadians(-45);

    std::shared_ptr<CameraCom> camera;
};