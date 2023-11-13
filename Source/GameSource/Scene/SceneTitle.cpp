#include "Graphics/Graphics.h"
#include "SceneTitle.h"

#include "SceneGame.h"
#include "SceneManager.h"
#include "Input/Input.h"
#include "SceneLoading.h"
#include "imgui.h"
#include "GameSource/Math/Mathf.h"
#include "GameSource/Math/easing.h"

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
    //sprite_ = new Sprite("Data/Sprite/titleLogo.png");

    {   //�s�R
        std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
        obj->SetName("picoTitle");
        obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });
        obj->transform_->SetWorldPosition({ 0, 0, -10 });
        obj->transform_->SetEulerRotation(DirectX::XMFLOAT3(0, 6, 7));

        const char* filename = "Data/Model/pico/picoAnim.mdl";
        std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
        r->LoadModel(filename);
        r->SetShaderID(SHADER_ID::UnityChanToon);

        std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();

        //�q�ɃJ�������t�H�[�J�X������
        {
            std::shared_ptr<GameObject> lookCamera = obj->AddChildObject();
            lookCamera->SetName("lookCamera");
            lookCamera->transform_->SetLocalPosition(DirectX::XMFLOAT3(31.4f, 51.5f, 63.1f));
        }
    }

    {   //candy
        std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
        obj->SetName("CandyTitle");
        obj->transform_->SetEulerRotation(DirectX::XMFLOAT3(0, 0, 0));
        obj->transform_->SetWorldPosition(DirectX::XMFLOAT3(0, -14, 0));

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

    //logo
    {
        std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
        obj->SetName("Logo");
        obj->transform_->SetScale(DirectX::XMFLOAT3(0.02f, 0.02f, 1));
        obj->transform_->SetWorldPosition(DirectX::XMFLOAT3(0.7f, 0.96f, -9.6f));
        obj->transform_->SetEulerRotation(DirectX::XMFLOAT3(0, 135, 0));

        const char* filename = "Data/Model/Title/TitleLogo.mdl";
        std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
        r->LoadModel(filename);
        r->SetShaderID(SHADER_ID::UnityChanToon);
    }

    //PushButton
    {
        std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
        obj->SetName("PushButton");
        obj->transform_->SetScale(DirectX::XMFLOAT3(0.003f, 0.003f, 1));
        obj->transform_->SetWorldPosition(DirectX::XMFLOAT3(-1.2f, 0.86f, -8.5f));
        obj->transform_->SetEulerRotation(DirectX::XMFLOAT3(0, 118, 0));

        const char* filename = "Data/Model/Title/TitlePushButton.mdl";
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

    //�t���O�ݒ�
    firstFrameSkip_ = false;
    isSceneEndFlag_ = false;
    startFlag_ = false;
    isStageMove_ = true;
    isStopFlag_ = false;

    //�����|�b�v�ݒ�
    candyID_ = 0;

    candyData_[0].nodeName = "Head";
    std::shared_ptr<GameObject> candy = GameObjectManager::Instance().Find("CandyTitle");
    candyData_[0].pos = { 0.18f,3.91f,2.37f };
    candyData_[0].angle = { -1.15f,0,0.15 };
    candyData_[0].scale = { 0.2f,0.2f,0.2f };
    candy->transform_->SetScale(candyData_[0].scale);

    candyData_[1].nodeName = "RightHandMiddle1";
    candyData_[1].pos = { -0.3f,-2.1f,6.5f };
    candyData_[1].angle = { 0,0,-3.478f };
    candyData_[1].scale = { 2,2,2 };

    candySizeCount_ = 1;

    //�u��
    eyeTime_ = 3;
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
        startFlag_ = true;
    }

    if (isSceneEndFlag_)
    {
        SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame));
        GameObjectManager::Instance().AllRemove();

        //�V�F�C�v�ݒ�
        std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("picoTitle");
        std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
        res->ShapeReset();
    }

    //Render��x�点��
    if (firstFrameSkip_)
        frameDelayRender = true;

    //�����|�b�v�s��ݒ�
    if (firstFrameSkip_)
    {
        std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("picoTitle");
        std::shared_ptr<GameObject> candy = GameObjectManager::Instance().Find("CandyTitle");
        Model::Node* parentNode = pico->GetComponent<RendererCom>()->GetModel()->FindNode(candyData_[candyID_].nodeName.c_str());

        //�q�ɂ���
        DirectX::XMMATRIX Candy = DirectX::XMLoadFloat4x4(&candy->transform_->GetLocalTransform());

        DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(candyData_[candyID_].angle.x, candyData_[candyID_].angle.y, candyData_[candyID_].angle.z);
        DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(candyData_[candyID_].pos.x, candyData_[candyID_].pos.y, candyData_[candyID_].pos.z);
        DirectX::XMMATRIX CP;
        if (candyID_ == 0)
            CP = R * T;
        if (candyID_ == 1)
            CP = R;
        DirectX::XMMATRIX Parent = DirectX::XMLoadFloat4x4(&parentNode->worldTransform);
        DirectX::XMMATRIX Tra = Candy * CP * Parent;

        //�s������
        DirectX::XMFLOAT4X4 tra;
        DirectX::XMStoreFloat4x4(&tra, Tra);
        candy->transform_->SetWorldTransform(tra);
    }


    GameObjectManager::Instance().Update(elapsedTime);


    if (isSceneEndFlag_)return;

    //���o
    TitleProductionUpdate(elapsedTime);

    //�����ōŏ��̃A�j���[�V�����Đ�
    if (!firstFrameSkip_)
    {
        std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("picoTitle");
        std::shared_ptr<AnimationCom> picoAnim = pico->GetComponent<AnimationCom>();

        picoAnim->PlayAnimation(48, true, 0);
        firstFrameSkip_ = true;

        //�V�F�C�v�����ݒ�
        std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
        res->ShapeReset();
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[2].rate = 1;
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[3].rate = 1;
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[7].rate = 0;
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[8].rate = 0;
    }


    //�J�ڃG�t�F�N�g
    {
        //�r���[�|�[�g
        D3D11_VIEWPORT viewport;
        UINT numViewports = 1;
        Graphics::Instance().GetDeviceContext()->RSGetViewports(&numViewports, &viewport);

        //�ϊ��s��
        DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&mainCamera_->GetView());
        DirectX::XMMATRIX Projection = DirectX::XMLoadFloat4x4(&mainCamera_->GetProjection());
        DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

        //std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("picoTitle");
        //DirectX::XMFLOAT3 effPosition = pico->transform_->GetWorldPosition();
        //DirectX::XMVECTOR EffPosition = DirectX::XMLoadFloat3(&effPosition);

        ////���[���h���W����X�N���[�����W�ɕϊ�
        //EffPosition = DirectX::XMVector3Project(
        //    EffPosition,
        //    viewport.TopLeftX, viewport.TopLeftY,
        //    viewport.Width, viewport.Height,
        //    viewport.MinDepth, viewport.MaxDepth,
        //    Projection, View, World
        //);

        DirectX::XMFLOAT3 effPosition;
        DirectX::XMVECTOR EffPosition;
        effPosition.x = -100;
        effPosition.y = -100;
        effPosition.z = 0.5f;
        EffPosition = DirectX::XMLoadFloat3(&effPosition);

        //�X�N���[�����W���烏�[���h���W�ɕϊ�
        EffPosition = DirectX::XMVector3Unproject(
            EffPosition,
            viewport.TopLeftX, viewport.TopLeftY,
            viewport.Width, viewport.Height,
            viewport.MinDepth, viewport.MaxDepth,
            Projection, View, World
        );

        DirectX::XMStoreFloat3(&effPosition, EffPosition);

        SceneManager::Instance().GetParticleObj()->transform_->SetWorldPosition(effPosition);
        SceneManager::Instance().GetParticleObj()->transform_->SetEulerRotation(DirectX::XMFLOAT3(-17, 171, -109));
    }
}

//�`�揈��
void SceneTitle::Render(float elapsedTime)
{
    if (isSceneEndFlag_)return;
    if (!frameDelayRender)return;

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

        //�o�b�t�@�߂�
        Graphics::Instance().RestoreRenderTargets();

        postEff_->Render(mainCamera_);
        postEff_->ImGuiRender();

    }



    if (ImGui::Begin("A", nullptr, ImGuiWindowFlags_None))
        ImGui::DragFloat3("angleCandy", &candyData_[0].angle.x, 0.01f);
        ImGui::DragFloat3("posCandy", &candyData_[0].pos.x, 0.01f);
        ImGui::DragFloat3("angleCandy1", &candyData_[1].angle.x, 0.01f);

    ImGui::End();
}

//���o
void SceneTitle::TitleProductionUpdate(float elapsedTime)
{
    std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("picoTitle");
    std::shared_ptr<AnimationCom> picoAnim = pico->GetComponent<AnimationCom>();
    std::shared_ptr<GameObject> stage = GameObjectManager::Instance().Find("DonutsTitle");
    std::shared_ptr<GameObject> logo = GameObjectManager::Instance().Find("Logo");
    std::shared_ptr<GameObject> pushButton = GameObjectManager::Instance().Find("PushButton");
    std::shared_ptr<GameObject> lookCamera = GameObjectManager::Instance().Find("lookCamera");

    //�X�e�[�W�㉺�ɓ�����
    if (isStageMove_)
    {
        static float sinPosY = 0;
        static float time = 0;
        static DirectX::XMFLOAT3 picoPos = pico->transform_->GetWorldPosition();
        static DirectX::XMFLOAT3 stagePos = stage->transform_->GetWorldPosition();
        static DirectX::XMFLOAT3 logoPos = logo->transform_->GetWorldPosition();
        static DirectX::XMFLOAT3 pushButtonPos = pushButton->transform_->GetWorldPosition();
        time += elapsedTime;
        sinPosY = sin(time) * 0.3f;

        DirectX::XMFLOAT3 pPos = picoPos;
        pPos.y += sinPosY;
        DirectX::XMFLOAT3 sPos = stagePos;
        sPos.y += sinPosY;
        DirectX::XMFLOAT3 lPos = logoPos;
        lPos.y += sinPosY;
        DirectX::XMFLOAT3 buttonPos = pushButtonPos;
        buttonPos.y += sinPosY*0.2f;

        pico->transform_->SetWorldPosition(pPos);
        stage->transform_->SetWorldPosition(sPos);
        logo->transform_->SetWorldPosition(lPos);
        pushButton->transform_->SetWorldPosition(buttonPos);
    }

    //�����|�b�v�p�x�ݒ�
    if (!startFlag_)
    {
        //X�̉������݂āAZ�𓮂���
        static int roundCount = 0;
        static int roundRand = Mathf::RandomRange(2, 6);

        //X�̓���
        static float dirX = 1;
        static float speedX = 0.2f;
        float x = candyData_[candyID_].angle.x;
        x += elapsedTime * speedX * dirX;
        candyData_[candyID_].angle.x = x;
        if (x > -1.0f)
        {
            dirX = -1.0f;
            speedX = Mathf::RandomRange(0.2f, 0.5f);
            roundCount++;
        }
        if (x < -1.2f)
        {
            dirX = 1.0f;
            speedX = Mathf::RandomRange(0.2f, 0.5f);
            roundCount++;
        }

        //Z�̓���
        static bool moveZ = false;
        if (roundCount >= roundRand)    //�����J�E���g���ݒ肵���l�ɂȂ�����
        {
            roundCount = 0;
            roundRand = Mathf::RandomRange(2, 6);
            moveZ = true;
        }

        if (moveZ)
        {
            static float dirZ = 1;
            static float speedZ = 0.2f;
            float z = candyData_[candyID_].angle.z;
            z += elapsedTime * speedZ * dirZ;
            candyData_[candyID_].angle.z = z;
            if (z > 0.15f && dirZ > 0)
            {
                dirZ = -1.0f;
                speedZ = Mathf::RandomRange(0.2f, 0.5f);
                moveZ = false;
            }
            if (z < 0 && dirZ < 0)
            {
                dirZ = 1.0f;
                speedZ = Mathf::RandomRange(0.2f, 0.5f);
                moveZ = false;
            }
        }
    }

    //�u��
    {
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
                shapeEye_ += elapsedTime * 2 * Mathf::RandomRange(1, 2);
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

    //�J������look�Ƀt�H�[�J�X
    {
        DirectX::XMFLOAT3 lerpPos = lookCamera->transform_->GetWorldPosition();
        static  DirectX::XMFLOAT3 oldPos = lerpPos;

        DirectX::XMStoreFloat3(&lerpPos, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&oldPos), DirectX::XMLoadFloat3(&lerpPos), 0.1f));
        oldPos = lerpPos;

        mainCamera_->SetLookAt(lerpPos);

    }


    //�X�^�[�g������
    if (startFlag_)
    {
        if (picoAnim->GetCurrentAnimationIndex() != 49)
            picoAnim->PlayAnimation(49, false);

        //��]
        if (picoAnim->GetCurrentAnimationEvent("turn", DirectX::XMFLOAT3()))
        {
            //��]
            DirectX::XMFLOAT3 picoEuler = pico->transform_->GetEulerRotation();
            picoEuler.y = Mathf::Lerp(picoEuler.y, startAfter_.playerEulerY, 0.1f);
            pico->transform_->SetEulerRotation(picoEuler);
        }
        //lookCamera�ݒ�
        if (picoAnim->GetCurrentAnimationEventIsEnd("headFocus"))
        {
            //lookCamera�𓮂���
            DirectX::XMFLOAT3 lookPos = lookCamera->transform_->GetLocalPosition();
            lookPos.y = Mathf::Lerp(lookPos.y, startAfter_.lookCameraPosY, 0.1f);
            lookCamera->transform_->SetLocalPosition(lookPos);

        }

        //step�P��A�b�v
        {
            DirectX::XMFLOAT3 headPos;
            if (picoAnim->GetCurrentAnimationEvent("upCamera", headPos))
            {
                //�����_
                lookCamera->transform_->SetWorldPosition(headPos);

                //�J����������
                DirectX::XMFLOAT3 lookPos = { headPos.x + startAfter_.cameraPos1.x,headPos.y + startAfter_.cameraPos1.y,headPos.z + startAfter_.cameraPos1.z };
                DirectX::XMFLOAT3 cPos = mainCamera_->GetGameObject()->transform_->GetWorldPosition();
                DirectX::XMStoreFloat3(&cPos, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&cPos), DirectX::XMLoadFloat3(&lookPos), 0.08f));
                mainCamera_->GetGameObject()->transform_->SetWorldPosition(cPos);

                //���S��]
                DirectX::XMFLOAT3 logoRotate = logo->transform_->GetEulerRotation();
                logoRotate.y += 100*elapsedTime;
                if (logoRotate.y > 200)
                    logoRotate.y = 200;
                logo->transform_->SetEulerRotation(logoRotate);
            }
            else if (picoAnim->GetCurrentAnimationEvent("headFocus", headPos))  //�J�������t�H�[�J�X
            {
                lookCamera->transform_->SetWorldPosition(headPos);
            }
        }

        //step�Q��ɃA�b�v�㏈��
        {
            if (picoAnim->GetCurrentAnimationEventIsEnd("upCamera") && !isStopFlag_)
            {
                isStopFlag_ = true;
                isStageMove_ = false;
                picoAnim->SetEnabled(false);
                stopTime_ = 1.0f;

                candyID_ = 1;
                GameObjectManager::Instance().Find("CandyTitle")->transform_->SetEulerRotation(DirectX::XMFLOAT3(-154, -85, 82));
                GameObjectManager::Instance().Find("CandyTitle")->transform_->SetWorldPosition(candyData_[candyID_].pos);

                //�V�F�C�v�ݒ�
                std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
                res->ShapeReset();
            }

            {
                if (isStopFlag_)
                {
                    //�V�F�C�v�ݒ�
                    std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
                    float rate7 = res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[7].rate;
                    float rate8 = res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[8].rate;
                    rate7 = Mathf::Lerp(rate7, 1, 0.1f);
                    rate8 = Mathf::Lerp(rate8, 0.5f, 0.1f);
                    res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[7].rate = rate7;
                    res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[8].rate = rate8;
                }
            }

            //�����Ƃ߂�
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

        //�����|�b�v�T�C�Y�ݒ�
        {
            std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("picoTitle");
            std::shared_ptr<AnimationCom> picoAnim = pico->GetComponent<AnimationCom>();

            if (picoAnim->GetCurrentAnimationEvent("bigCandy", DirectX::XMFLOAT3(0, 0, 0)))
            {
                static int sizeCount = 3;
                auto& animCandy = picoAnim->GetAnimationEvent("bigCandy");
                static float start = animCandy.resourceEventData.startFrame;
                static float len = animCandy.resourceEventData.endFrame - animCandy.resourceEventData.startFrame;
                float animTime = picoAnim->GetCurrentAnimationSecoonds();

                if (start + (len * candySizeCount_ / sizeCount) > animTime)
                {
                    std::shared_ptr<GameObject> candy = GameObjectManager::Instance().Find("CandyTitle");
                    static float distSize = candyData_[1].scale.x - candyData_[0].scale.x;
                    float size = candyData_[0].scale.x + distSize * candySizeCount_ / sizeCount;
                    candy->transform_->SetScale(DirectX::XMFLOAT3(size, size, size));
                }
                else
                    candySizeCount_++;

            }
        }


        //�A�j���[�V�����I���
        if (!picoAnim->IsPlayAnimation())
        {
            //�V�[���J�ڃt���OON
            isSceneEndFlag_ = true;

            //������
            if(0)
            {
                picoAnim->PlayAnimation(48, true, 0);
                pico->transform_->SetEulerRotation(DirectX::XMFLOAT3(0, 6, 7));
                mainCamera_->GetGameObject()->transform_->SetWorldPosition({ -1.6f, 1.1f, -8.3f });
                isStopFlag_ = false;
                lookCamera->transform_->SetLocalPosition(DirectX::XMFLOAT3(31.4f, 51.5f, 63.1f));
                startFlag_ = false;
                candyID_ = 0;

                std::shared_ptr<GameObject> candy = GameObjectManager::Instance().Find("CandyTitle");
                candy->transform_->SetEulerRotation(DirectX::XMFLOAT3(-0, -0, 0));
                candy->transform_->SetWorldPosition(DirectX::XMFLOAT3(-0, -14, 0));
                candy->transform_->SetScale(DirectX::XMFLOAT3(candyData_[0].scale.x, candyData_[0].scale.y, candyData_[0].scale.z));
                candySizeCount_ = 1;

                eyeTime_ = 3;

                //�V�F�C�v�����ݒ�
                std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
                res->ShapeReset();
                res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[2].rate = 1;
                res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[3].rate = 1;
                res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[7].rate = 0;
                res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[8].rate = 0;
            }
        }

        //�u�������Ȃ�����
        eyeTime_ = 100;
    }
}
