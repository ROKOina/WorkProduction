#pragma once

#include <DirectXMath.h>

#include "../Source/Components/CameraCom.h"

//�J�����R���g���[���[
class CameraController
{
public:
    CameraController(){}
    ~CameraController(){}

    //�X�V����
    void Update(float elapsedTime);

    //�^�[�Q�b�g�ʒu�ݒ�
    void SetTarget(const DirectX::XMFLOAT3& target) { this->target = target; }

    //�J����
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