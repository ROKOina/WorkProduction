#include "SceneManager.h"

//�X�V����
void SceneManager::Update(float elapsedTime)
{
    if (nextScene_ != nullptr)
    {
        //�Â��V�[�����I������
        Clear();

        //�V�����V�[����ݒ�
        currentScene_ = nextScene_;
        nextScene_ = nullptr;

        //�V�[������������
        if (!currentScene_->IsReady())currentScene_->Initialize();

    }

    if (currentScene_ != nullptr)
    {
        currentScene_->Update(elapsedTime);
    }
}

//�`�揈��
void SceneManager::Render()
{
    if (currentScene_ != nullptr)
    {
        currentScene_->Render();
    }
}

//�V�[���N���A
void SceneManager::Clear()
{
    if (currentScene_ != nullptr)
    {
        currentScene_->Finalize();
        delete currentScene_;
        currentScene_ = nullptr;
    }
}

//�V�[���؂�ւ�
void SceneManager::ChangeScene(Scene* scene)
{
    //�V�����V�[����ݒ�
    nextScene_ = scene;
}
