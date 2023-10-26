#pragma once

#include "Graphics/Sprite/Sprite.h"
#include "Graphics\Shaders\PostEffect.h"
#include "Scene.h"

//�^�C�g���V�[��
class SceneTitle :public Scene
{
public:
    SceneTitle(){}
    ~SceneTitle()override{}

    //������
    void Initialize()override;

    //�I����
    void Finalize()override;

    //�X�V����
    void Update(float elapsedTime)override;

    //�`�揈��
    void Render()override;

private:
    std::unique_ptr<PostEffect> postEff_;
    Sprite* sprite_ = nullptr;

    bool firstAnimation_ = false;
    bool isSceneEndFlag_ = false;
    bool startFlag_ = false;
    bool isStageMove_ = true;   //�X�e�[�W��������
    bool isStopFlag_ = false;   //�A�b�v��~�߂�
    float stopTime_ = -1.0f;    //�A�b�v��~�߂鎞��

    float eyeTime_ = 3;
    bool isShapeEye_ = false;
    float shapeEye_ = 0;    //�u���p

    struct StartAfterData
    {
        float playerEulerY = -82;
        float lookCameraPosY = 65.4f;
        DirectX::XMFLOAT3 cameraPos1 = { -0.768f,-0.12f,-0.14f };
        DirectX::XMFLOAT3 cameraPos2 = { -1.8f,0.4f,-10.0f };

    }startAfter_;
};