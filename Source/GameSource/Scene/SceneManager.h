#pragma once

#include "Scene.h"

class GameObject;

//�V�[���}�l�[�W���[
class SceneManager
{
private:
    SceneManager();
    ~SceneManager(){}

public:
    //�B��̃C���X�^���X�擾
    static SceneManager& Instance()
    {
        static SceneManager instance;
        return instance;
    }

    //�X�V����
    void Update(float elapsedTime);

    //�`�揈��
    void Render(float elapsedTime);

    //�V�[���N���A
    void Clear();

    //�V�[���؂�ւ�
    void ChangeScene(Scene* scene);

    void ChangeParticleUpdate(float elapsedTime);
    void ChangeParticleRender();
    void ChangeParticleGui();

    std::shared_ptr<GameObject> GetParticleObj()
    {
        return changeParticle_;
    }

private:
    Scene* currentScene_ = nullptr;
    Scene* nextScene_ = nullptr;

    std::shared_ptr<GameObject> changeParticle_;
    std::shared_ptr<GameObject> changeParticleCamera_;
};