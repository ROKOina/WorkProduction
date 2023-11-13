#include "SceneManager.h"

#include "Components/System/GameObject.h"
#include "Components\ParticleSystemCom.h"
#include "Components\TransformCom.h"
#include "Graphics/Graphics.h"

SceneManager::SceneManager()
{
    changeParticle_ = std::make_shared<GameObject>();
    changeParticle_->AddComponent<TransformCom>();
    changeParticle_->SetName("changeParticle");

    std::shared_ptr<ParticleSystemCom> c = changeParticle_->AddComponent<ParticleSystemCom>(1000);
    c->SetSweetsParticle(true);	//���َq�p
    //c->LoadTexture("./Data/Sprite/sweetsParticle.png");
    c->Load("Data/Effect/para/TitleParticle.ipff");

    changeParticle_->Start();
}

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
void SceneManager::Render(float elapsedTime)
{
    if (currentScene_ != nullptr)
    {
        currentScene_->Render(elapsedTime);
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

void SceneManager::ChangeParticleUpdate(float elapsedTime)
{
    changeParticle_->Update(elapsedTime);
}

void SceneManager::ChangeParticleRender()
{
    //Graphics& graphics = Graphics::Instance();

    ////��ԍŏ��̃V�F�[�_�[����ۑ�
    //static DirectX::XMFLOAT4 viewPosition = graphics.shaderParameter3D_.viewPosition;
    //static DirectX::XMFLOAT4X4 view = graphics.shaderParameter3D_.view;
    //static DirectX::XMFLOAT4X4 projection = graphics.shaderParameter3D_.projection;
    //static DirectX::XMFLOAT4 lightDirection = graphics.shaderParameter3D_.lightDirection;
    //DirectX::XMFLOAT4 saveViewPosition = graphics.shaderParameter3D_.viewPosition;
    //DirectX::XMFLOAT4X4 saveView = graphics.shaderParameter3D_.view;
    //DirectX::XMFLOAT4X4 saveProjection = graphics.shaderParameter3D_.projection;
    //DirectX::XMFLOAT4 saveLightDirection = graphics.shaderParameter3D_.lightDirection;

    ////�ŏ��̃V�F�[�_�[������
    //graphics.shaderParameter3D_.viewPosition = viewPosition;
    //graphics.shaderParameter3D_.view = view;
    //graphics.shaderParameter3D_.projection = projection;
    //graphics.shaderParameter3D_.lightDirection = lightDirection;

    changeParticle_->GetComponent<ParticleSystemCom>()->Render();

    ////�O�̏��ɖ߂�
    //graphics.shaderParameter3D_.viewPosition = saveViewPosition;
    //graphics.shaderParameter3D_.view = saveView;
    //graphics.shaderParameter3D_.projection = saveProjection;
    //graphics.shaderParameter3D_.lightDirection = saveLightDirection;
}

void SceneManager::ChangeParticleGui()
{
    changeParticle_->OnGUI();
}
