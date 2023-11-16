#include "SceneManager.h"

#include "Components/System/GameObject.h"
#include "Components\ParticleSystemCom.h"
#include "Components\TransformCom.h"
#include "Components\CameraCom.h"
#include "Graphics/Graphics.h"

SceneManager::SceneManager()
{
    //�p�[�e�B�N��
    {
        changeParticle_ = std::make_shared<GameObject>();
        changeParticle_->AddComponent<TransformCom>();
        changeParticle_->SetName("changeParticle");
        changeParticle_->transform_->SetWorldPosition(DirectX::XMFLOAT3(-0.5f, 0.3f, 0.6f));
        changeParticle_->transform_->SetEulerRotation(DirectX::XMFLOAT3(3, 95, -102));

        std::shared_ptr<ParticleSystemCom> c = changeParticle_->AddComponent<ParticleSystemCom>(5000);
        c->SetSweetsParticle(true);	//���َq�p
        c->Load("Data/Effect/para/TitleParticle2.ipff");
        c->SetEnabled(false);

        changeParticle_->Start();
    }

    //�p�[�e�B�N���J����
    {
        changeParticleCamera_ = std::make_shared<GameObject>();
        changeParticleCamera_->AddComponent<TransformCom>();
        changeParticleCamera_->SetName("changeParticleCamera");

        Graphics& graphics = Graphics::Instance();
        std::shared_ptr<CameraCom> c = changeParticleCamera_->AddComponent<CameraCom>();
        c->SetPerspectiveFov(
            DirectX::XMConvertToRadians(45),
            graphics.GetScreenWidth() / graphics.GetScreenHeight(),
            0.1f, 1000.0f
        );
    }
}

SceneManager::~SceneManager()
{
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

    //�p�[�e�B�N���p
    if(isParticleUpdate_)
    {
        changeParticle_->transform_->UpdateTransform();
        changeParticle_->Update(elapsedTime);
        changeParticleCamera_->Update(elapsedTime);
    }
}

//�`�揈��
void SceneManager::Render(float elapsedTime)
{
    if (currentScene_ != nullptr)
    {
        currentScene_->Render(elapsedTime);
    }

    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();
    Dx11StateLib* dx11State = graphics.GetDx11State().get();

    //�p�[�e�B�N���p
    {
        dc->OMSetDepthStencilState(
            dx11State->GetDepthStencilState(Dx11StateLib::DEPTHSTENCIL_STATE_TYPE::DEPTH_ON_2D).Get(),
            0);

        // �`�揈��
        ShaderParameter3D& rc = graphics.shaderParameter3D_;
        std::shared_ptr<CameraCom> particleCamera = changeParticleCamera_->GetComponent<CameraCom>();
        //�J�����p�����[�^�[�ݒ�
        rc.view = particleCamera->GetView();
        rc.projection = particleCamera->GetProjection();
        DirectX::XMFLOAT3 cameraPos = particleCamera->GetGameObject()->transform_->GetWorldPosition();
        rc.viewPosition = { cameraPos.x,cameraPos.y,cameraPos.z,1 };

        changeParticle_->GetComponent<ParticleSystemCom>()->Render();
        changeParticle_->OnGUI();
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

