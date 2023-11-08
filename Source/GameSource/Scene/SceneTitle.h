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
    void Render(float elapsedTime)override;

private:
    //���o
    void TitleProductionUpdate(float elapsedTime);

private:
    std::unique_ptr<PostEffect> postEff_;
    Sprite* sprite_ = nullptr;

    bool firstFrameSkip_ = false;
    bool isSceneEndFlag_ = false;
    bool startFlag_ = false;
    bool isStageMove_ = true;   //�X�e�[�W��������
    bool isStopFlag_ = false;   //�A�b�v��~�߂�
    float stopTime_ = -1.0f;    //�A�b�v��~�߂鎞��

    float eyeTime_ = 3;
    bool isShapeEye_ = false;
    float shapeEye_ = 0;    //�u���p

    int candySizeCount_ = 1;

    struct StartAfterData
    {
        float playerEulerY = -90;
        float lookCameraPosY = 65.4f;
        DirectX::XMFLOAT3 cameraPos1 = { -0.768f,-0.12f,-0.14f };
        DirectX::XMFLOAT3 cameraPos2 = { -2.4f,0.4f,-10.0f };

    }startAfter_;

    struct CandyData    //�����|�b�v
    {
        std::string nodeName;
        DirectX::XMFLOAT3 pos = { 0,0,0 };
        DirectX::XMFLOAT3 angle = { 0,0,0 };
        DirectX::XMFLOAT3 scale = { 1,1,1 };
    };
    CandyData candyData_[2];
    int candyID_ = 0;

    //�`��ŏ������̎p�ڂ�̂ŁA�x�点��
    bool frameDelayRender = false;
};