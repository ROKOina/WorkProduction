#include "Graphics/Graphics.h"
#include "SceneTitle.h"

#include "SceneGame.h"
#include "SceneManager.h"
#include "Input/Input.h"
#include "SceneLoading.h"
#include "imgui.h"
#include "GameSource/Math/Mathf.h"

#include "Components\System\GameObject.h"
#include "Components\RendererCom.h"
#include "Components\TransformCom.h"
#include "Components\CameraCom.h"
#include "Components\AnimationCom.h"
#include "Components\ColliderCom.h"

//������
void SceneTitle::Initialize()
{
    //�X�v���C�g������
    sprite_ = new Sprite("Data/Sprite/Title.png");

    {   //�s�R
        std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
        obj->SetName("picoTitle");
        obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });
        obj->transform_->SetWorldPosition({ 0, 0, -10 });

        const char* filename = "Data/Model/pico/picoAnim.mdl";
        std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
        r->LoadModel(filename);
        r->SetShaderID(SHADER_ID::UnityChanToon);

        std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();

        {
            std::shared_ptr<GameObject> lookCamera = obj->AddChildObject();
            lookCamera->SetName("lookCamera");
            lookCamera->transform_->SetLocalPosition(DirectX::XMFLOAT3(31.4f, 51.5f, 63.1f));

            std::shared_ptr<SphereColliderCom> col = lookCamera->AddComponent<SphereColliderCom>();
            col->SetRadius(0.19f);
        }
    }

    {   //candy
        std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
        obj->SetName("CandyTitle");
        obj->transform_->SetScale(DirectX::XMFLOAT3(0.002f, 0.002f, 0.002f));

        const char* filename = "Data/Model/Swords/Candy/Candy.mdl";
        std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
        r->LoadModel(filename);
        r->SetShaderID(SHADER_ID::UnityChanToon);
    }

    //stage
    {
        std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
        obj->SetName("DonutsTitle");
        obj->transform_->SetScale(DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
        obj->transform_->SetWorldPosition(DirectX::XMFLOAT3(3.4f, 0.36f, -10.4f));
        obj->transform_->SetEulerRotation(DirectX::XMFLOAT3(0, 6, 7));

        const char* filename = "Data/Model/stages/title/Donuts/Donuts.mdl";
        std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
        r->LoadModel(filename);
        r->SetShaderID(SHADER_ID::UnityChanToon);
    }

    //�J�����𐶐�
    {
        std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Create();
        cameraObj->SetName("CameraTitle");

        Graphics& graphics = Graphics::Instance();
        std::shared_ptr<CameraCom> c = cameraObj->AddComponent<CameraCom>();
        c->SetPerspectiveFov(
            DirectX::XMConvertToRadians(45),
            graphics.GetScreenWidth() / graphics.GetScreenHeight(),
            0.1f, 1000.0f
        );
        cameraObj->transform_->SetWorldPosition({ -1.6f, 1.1f, -8.3f });
    }

    //���C���J�����ݒ�
    std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("CameraTitle")->GetComponent<CameraCom>();
    mainCamera_ = camera;

    //�|�X�g�G�t�F�N�g
    {
        Graphics& graphics = Graphics::Instance();
        postEff_ = std::make_unique<PostEffect>(
            static_cast<UINT>(graphics.GetScreenWidth()),
            static_cast<UINT>(graphics.GetScreenHeight()));

        //�u���[��
        graphics.shaderParameter3D_.bloomData2.intensity = 5;
        graphics.shaderParameter3D_.bloomData2.threshold = 1;

        //���z
        graphics.shaderParameter3D_.lightDirection = { -0.972198129f,-0.0744780228f,-0.222000003f,0 };
    }

    firstAnimation_ = false;
    isSceneEndFlag_ = false;
    startFlag_ = false;
    isStageMove_ = true;
    isStopFlag_ = false;
}

//�I����
void SceneTitle::Finalize()
{
    //�X�v���C�g�I����
    if (sprite_ != nullptr)
    {
        delete sprite_;
        sprite_ = nullptr;
    }
}

//�X�V����
void SceneTitle::Update(float elapsedTime)
{
    GamePad& gamePad = Input::Instance().GetGamePad();

    //�����{�^������������Q�[���V�[���֐؂肩��
    const GamePadButton anyButton =
        GamePad::BTN_UP |
        GamePad::BTN_RIGHT |
        GamePad::BTN_DOWN |
        GamePad::BTN_LEFT |
        GamePad::BTN_A |
        GamePad::BTN_B |
        GamePad::BTN_X |
        GamePad::BTN_Y |
        GamePad::BTN_START |
        GamePad::BTN_BACK |
        GamePad::BTN_LEFT_THUMB |
        GamePad::BTN_RIGHT_THUMB |
        GamePad::BTN_LEFT_SHOULDER |
        GamePad::BTN_RIGHT_SHOULDER |
        GamePad::BTN_LEFT_TRIGGER |
        GamePad::BTN_RIGHT_TRIGGER;

    if (gamePad.GetButtonDown() & anyButton)
    {
        //SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame));
        //GameObjectManager::Instance().AllRemove();
        //isSceneEndFlag_ = true;
        startFlag_ = true;
    }

    if (isSceneEndFlag_)
    {
        SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame));
        GameObjectManager::Instance().AllRemove();
    }

    GameObjectManager::Instance().Update(elapsedTime);

    if (isSceneEndFlag_)return;


    std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("picoTitle");
    std::shared_ptr<AnimationCom> picoAnim = pico->GetComponent<AnimationCom>();
    std::shared_ptr<GameObject> stage = GameObjectManager::Instance().Find("DonutsTitle");
    std::shared_ptr<GameObject> lookCamera = GameObjectManager::Instance().Find("lookCamera");

    //�㉺�ɓ�����
    if (isStageMove_)
    {
        static float sinPosY = 0;
        static float time = 0;
        static DirectX::XMFLOAT3 picoPos = pico->transform_->GetWorldPosition();
        static DirectX::XMFLOAT3 stagePos = stage->transform_->GetWorldPosition();
        time += elapsedTime;
        sinPosY = sin(time) * 0.3f;

        DirectX::XMFLOAT3 pPos = picoPos;
        pPos.y += sinPosY;
        DirectX::XMFLOAT3 sPos = stagePos;
        sPos.y += sinPosY;

        pico->transform_->SetWorldPosition(pPos);
        stage->transform_->SetWorldPosition(sPos);
    }

    //�J�������t�H�[�J�X
    {
        DirectX::XMFLOAT3 lerpPos = lookCamera->transform_->GetWorldPosition();
        static  DirectX::XMFLOAT3 oldPos = lerpPos;

        DirectX::XMStoreFloat3(&lerpPos, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&oldPos), DirectX::XMLoadFloat3(&lerpPos), 0.1f));
        oldPos = lerpPos;

        mainCamera_->SetLookAt(lerpPos);

    }

    //step�Q��ɃA�b�v�㏈��
    {
        if (picoAnim->GetCurrentAnimationEventIsEnd("upCamera") && !isStopFlag_)
        {

            isStopFlag_ = true;
            isStageMove_ = false;
            picoAnim->SetEnabled(false);
            stopTime_ = 1.0f;
        }

        if (stopTime_ > 0)
        {
            stopTime_ -= elapsedTime;
        }
        else
        {
            if (isStopFlag_)
            {
                isStageMove_ = true;
                picoAnim->SetEnabled(true);
            }
        }
    }

    //step�R��A�b�v�����������
    {
        if (picoAnim->GetCurrentAnimationEventIsEnd("headFocus"))
        {
            DirectX::XMFLOAT3 cPos = mainCamera_->GetGameObject()->transform_->GetWorldPosition();
            DirectX::XMStoreFloat3(&cPos, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&cPos), DirectX::XMLoadFloat3(&startAfter_.cameraPos2), 0.08f));
            mainCamera_->GetGameObject()->transform_->SetWorldPosition(cPos);
        }
    }

    //lookCamera����ɓ�����
    {
        DirectX::XMFLOAT3 headPos;
        if (picoAnim->GetCurrentAnimationEvent("upCamera", headPos))    //step�P��A�b�v
        {
            //�����_
            lookCamera->transform_->SetWorldPosition(headPos);

            //�J����������
            DirectX::XMFLOAT3 lookPos = { headPos.x + startAfter_.cameraPos1.x,headPos.y + startAfter_.cameraPos1.y,headPos.z + startAfter_.cameraPos1.z };
            DirectX::XMFLOAT3 cPos = mainCamera_->GetGameObject()->transform_->GetWorldPosition();
            DirectX::XMStoreFloat3(&cPos, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&cPos), DirectX::XMLoadFloat3(&lookPos), 0.08f));
            mainCamera_->GetGameObject()->transform_->SetWorldPosition(cPos);
        }
        else if (picoAnim->GetCurrentAnimationEvent("headFocus", headPos))  //�J�������t�H�[�J�X
        {
            lookCamera->transform_->SetWorldPosition(headPos);
        }
    }

    //�X�^�[�g������
    if (startFlag_)
    {
        if (picoAnim->GetCurrentAnimationIndex() != 49)
            picoAnim->PlayAnimation(49, false);

        if (picoAnim->GetCurrentAnimationEvent("turn", DirectX::XMFLOAT3()))
        {
            //��]
            DirectX::XMFLOAT3 picoEuler = pico->transform_->GetEulerRotation();
            picoEuler.y = Mathf::Lerp(picoEuler.y, startAfter_.playerEulerY, 0.1f);
            pico->transform_->SetEulerRotation(picoEuler);
        }

        if (picoAnim->GetCurrentAnimationEventIsEnd("headFocus"))
        {
            //lookCamera�𓮂���
            DirectX::XMFLOAT3 lookPos = lookCamera->transform_->GetLocalPosition();
            lookPos.y = Mathf::Lerp(lookPos.y, startAfter_.lookCameraPosY, 0.1f);
            lookCamera->transform_->SetLocalPosition(lookPos);

        }

        //�A�j���[�V�����I���
        if (!picoAnim->IsPlayAnimation())
        {
            isSceneEndFlag_ = true;

            //������
            picoAnim->PlayAnimation(48, true, 0);
            pico->transform_->SetEulerRotation(DirectX::XMFLOAT3(0, 6, 7));
            mainCamera_->GetGameObject()->transform_->SetWorldPosition({ -1.6f, 1.1f, -8.3f });
            isStopFlag_ = false;
            lookCamera->transform_->SetLocalPosition(DirectX::XMFLOAT3(31.4f, 51.5f, 63.1f));
            startFlag_ = false;
        }
    }

    //�����ŃA�j���[�V�����Đ�
    if (!firstAnimation_)
    {
        picoAnim->PlayAnimation(48, true, 0);
        firstAnimation_ = true;

        //�V�F�C�v�ݒ�
        std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[2].rate = 1;
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[3].rate = 1;
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[7].rate = 0;
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[8].rate = 0;
    }

    //�u��
    eyeTime_ -= elapsedTime;
    static bool eyeDir = true;  //�u���p�t���O
    if (eyeTime_ < 0)
    {
        eyeTime_ = Mathf::RandomRange(3, 6);
        isShapeEye_ = true;
        eyeDir = true;
    }
    if (isShapeEye_)    //�u�����s
    {
        if (eyeDir)
        {
            shapeEye_ += elapsedTime * 2* Mathf::RandomRange(1, 2);
            if (shapeEye_ >= 1)
                eyeDir = false;
        }
        else
        {
            shapeEye_ -= elapsedTime * 2;
            if (shapeEye_ < 0)
            {
                shapeEye_ = 0;
                isShapeEye_ = false;
            }
        }
        //�V�F�C�v�ݒ�
        std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[0].rate = shapeEye_;
    }
}

//�`�揈��
void SceneTitle::Render()
{
    if (isSceneEndFlag_)return;
    if (!firstAnimation_)return;

    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();
    ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
    ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

    //��ʃN���A�������_�[�^�[�Q�b�g�ݒ�
    FLOAT color[] = { 0.0f,0.0f,0.5f,1.0f };    //RGBA(0.0~1.0)
    dc->ClearRenderTargetView(rtv, color);
    dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,1.0f, 0);
    dc->OMSetRenderTargets(1, &rtv, dsv);

    // �`�揈��
    ShaderParameter3D& rc = graphics.shaderParameter3D_;
    rc.lightDirection = graphics.shaderParameter3D_.lightDirection;	// ���C�g����

    //�J�����p�����[�^�[�ݒ�
    rc.view = mainCamera_->GetView();
    rc.projection = mainCamera_->GetProjection();
    DirectX::XMFLOAT3 cameraPos = mainCamera_->GetGameObject()->transform_->GetWorldPosition();
    rc.viewPosition = { cameraPos.x,cameraPos.y,cameraPos.z,1 };

    ////2D�X�v���C�g�`��
    //{
    //    float screenWidth = static_cast<float>(graphics.GetScreenWidth());
    //    float screenHeight = static_cast<float>(graphics.GetScreenHeight());
    //    float textureWidth = static_cast<float>(sprite_->GetTextureWidth());
    //    float textureHeight = static_cast<float>(sprite_->GetTextureHeight());
    //    //�^�C�g���X�v���C�g�`��
    //    sprite_->Render(dc,
    //        0, 0, screenWidth, screenHeight,
    //        0, 0, textureWidth, textureHeight,
    //        0,
    //        1, 1, 1, 1);
    //}


    //3D�`��
    {
        //�o�b�t�@���
        Graphics::Instance().CacheRenderTargets();

        //�|�X�g�G�t�F�N�g�p�؂�ւ�
        PostRenderTarget* ps = Graphics::Instance().GetPostEffectModelRenderTarget().get();
        PostDepthStencil* ds = Graphics::Instance().GetPostEffectModelDepthStencilView().get();

        // ��ʃN���A�������_�[�^�[�Q�b�g�ݒ�
        rtv = {};
        dsv = {};
        rtv = ps->renderTargetView.Get();
        dsv = ds->depthStencilView.Get();
        dc->ClearRenderTargetView(rtv, color);
        dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        dc->OMSetRenderTargets(1, &rtv, dsv);

        // �r���[�|�[�g�̐ݒ�
        D3D11_VIEWPORT	vp = {};
        vp.Width = static_cast<float>(ps->width);
        vp.Height = static_cast<float>(ps->height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        dc->RSSetViewports(1, &vp);

        //�X�J�C�}�b�v�`��
        postEff_->SkymapRender(mainCamera_);

        GameObjectManager::Instance().UpdateTransform();

        GameObjectManager::Instance().Render(mainCamera_->GetView(), mainCamera_->GetProjection());

        // �f�o�b�O�����_���`����s
        graphics.GetDebugRenderer()->Render(dc, mainCamera_->GetView(), mainCamera_->GetProjection());

        //�o�b�t�@�߂�
        Graphics::Instance().RestoreRenderTargets();

        postEff_->Render(mainCamera_);
        postEff_->ImGuiRender();

    }



    //if (ImGui::Begin("A", nullptr, ImGuiWindowFlags_None))
    //if(ImGui::TreeNode("A"))
    //{
    //    static float a;
    //    ImGui::DragFloat("a",&a );
    //    ImGui::TreePop();
    //}
    //ImGui::End();
}
