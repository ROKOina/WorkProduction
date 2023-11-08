#pragma once

#include "Graphics/Sprite/Sprite.h"
#include "Scene.h"
#include <thread>

//���[�f�B���O�V�[��
class SceneLoading :public Scene
{
public:
    SceneLoading(Scene* nextScene):nextScene_(nextScene){}
    ~SceneLoading(){}

    //������
    void Initialize()override;

    //�I����
    void Finalize()override;

    //�X�V����
    void Update(float elapsedTime)override;

    //�`�揈��
    void Render(float elapsedTime)override;

private:
    //���[�f�B���O�X���b�h
    static void LoadingThread(SceneLoading* scene);

private :
    Sprite* sprite_ = nullptr;
    float angle_ = 0.0f;
    Scene* nextScene_ = nullptr;
    std::thread* thread_ = nullptr;
};