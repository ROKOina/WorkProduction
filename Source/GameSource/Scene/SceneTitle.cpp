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
#include "Components\ParticleSystemCom.h"
#include "GameSource\Render\Effect\EffectManager.h"
#include "../ScriptComponents/Score.h"

//初期化
void SceneTitle::Initialize()
{

    {   //ピコ
        std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
        obj->SetName("picoTitle");
        obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });
        obj->transform_->SetWorldPosition({ 0, 0, -10 });
        obj->transform_->SetEulerRotation(DirectX::XMFLOAT3(0, 6, 7));

        const char* filename = "Data/Model/pico/picoAnim.mdl";
        std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
        r->LoadModel(filename);
        r->SetShaderID(SHADER_ID::UnityChanToon);
        r->SetIsShadowFall(true);

        std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();

        //子にカメラをフォーカスさせる
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
        r->SetIsShadowDraw(true);
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

    //カメラを生成
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

    //メインカメラ設定
    std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("CameraTitle")->GetComponent<CameraCom>();
    mainCamera_ = camera;

    //ポストエフェクト
    {
        Graphics& graphics = Graphics::Instance();
        postEff_ = std::make_unique<PostEffect>(
            static_cast<UINT>(graphics.GetScreenWidth()),
            static_cast<UINT>(graphics.GetScreenHeight()));

        //ブルーム
        graphics.shaderParameter3D_.bloomLuminance.intensity = 5;
        graphics.shaderParameter3D_.bloomLuminance.threshold = 1;

        //太陽
        graphics.shaderParameter3D_.lightDirection = { -0.972198129f,-0.0744780228f,-0.222000003f,0 };
    }

    //フラグ設定
    firstFrameSkip_ = false;
    isSceneEndFlag_ = false;
    startFlag_ = false;
    isStageMove_ = true;
    isStopFlag_ = false;

    //ロリポップ設定
    candyID_ = 0;

    candyData_[0].nodeName = "Head";
    std::shared_ptr<GameObject> candy = GameObjectManager::Instance().Find("CandyTitle");
    candyData_[0].pos = { 0.18f,3.91f,2.37f };
    candyData_[0].angle = { -1.15f,0,0.15f };
    candyData_[0].scale = { 0.2f,0.2f,0.2f };
    candy->transform_->SetScale(candyData_[0].scale);

    candyData_[1].nodeName = "RightHandMiddle1";
    candyData_[1].pos = { -0.3f,-2.1f,6.5f };
    candyData_[1].angle = { 0,0,-3.478f };
    candyData_[1].scale = { 2,2,2 };

    candySizeCount_ = 1;

    //瞬き
    eyeTime_ = 3;

    //パーティクル遷移時間
    transitionOutTimer_ = 1.5f;
    transitionInTimer_ = -1.0f;

    //初期ビネット設定
    Vignette& vignette = Graphics::Instance().shaderParameter3D_.vignette;
    vignette.enabled = -1;

    std::shared_ptr<ParticleSystemCom> particle = SceneManager::Instance().GetParticleObj()->GetComponent<ParticleSystemCom>();
    if (particle->GetEnabled()) //パーティクル起動していたら
    {
        transitionInTimer_ = 2.5f;
        particle->GetSaveParticleData().particleData.isRoop = false;
        SceneManager::Instance().SetParticleUpdate(true);
    }

    //BGM
    BGM_->Stop();
    BGM_->Play(true);

    //スコアを初期化
    Score::Instance().ResetScore();
}

//終了化
void SceneTitle::Finalize()
{
    BGM_->AudioRelease();
    pushSE_->AudioRelease();
    //BGM_->Stop();
    //pushSE_->Stop();
}

//更新処理
void SceneTitle::Update(float elapsedTime)
{
    GamePad& gamePad = Input::Instance().GetGamePad();

    //何かボタンを押したらゲームシーンへ切りかえ
    //const GamePadButton anyButton =
    //    GamePad::BTN_UP |
    //    GamePad::BTN_RIGHT |
    //    GamePad::BTN_DOWN |
    //    GamePad::BTN_LEFT |
    //    GamePad::BTN_A |
    //    GamePad::BTN_B |
    //    GamePad::BTN_X |
    //    GamePad::BTN_Y |
    //    GamePad::BTN_START |
    //    GamePad::BTN_BACK |
    //    GamePad::BTN_LEFT_THUMB |
    //    GamePad::BTN_RIGHT_THUMB |
    //    GamePad::BTN_LEFT_SHOULDER |
    //    GamePad::BTN_RIGHT_SHOULDER |
    //    GamePad::BTN_LEFT_TRIGGER |
    //    GamePad::BTN_RIGHT_TRIGGER;
    const GamePadButton anyButton =GamePad::BTN_B ;

    if (gamePad.GetButtonDown() & anyButton)
    {
        std::shared_ptr<ParticleSystemCom> particle = SceneManager::Instance().GetParticleObj()->GetComponent<ParticleSystemCom>();
        if (!particle->GetEnabled())
        {
            startFlag_ = true;
            pushSE_->Play(false);
        }
    }

    if (isSceneEndFlag_)
    {
        //パーティクル遷移時間
        transitionOutTimer_ -= elapsedTime;
        if (transitionOutTimer_ < 0)
        {
            SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame));
            GameObjectManager::Instance().AllRemove();

            //シェイプ設定
            std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("picoTitle");
            std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
            res->ShapeReset();
            SceneManager::Instance().SetParticleUpdate(false);
        }
    }

    if (transitionInTimer_ >= 0)
    {
        transitionInTimer_ -= elapsedTime;
        if (transitionInTimer_ <= 0)
        {
            std::shared_ptr<ParticleSystemCom> particle = SceneManager::Instance().GetParticleObj()->GetComponent<ParticleSystemCom>();
            particle->SetEnabled(false);
            SceneManager::Instance().SetParticleUpdate(false);
        }
    }

    //Renderを遅らせる
    if (firstFrameSkip_)
        frameDelayRender_ = true;

    //ロリポップ行列設定
    if (firstFrameSkip_)
    {
        std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("picoTitle");
        std::shared_ptr<GameObject> candy = GameObjectManager::Instance().Find("CandyTitle");
        Model::Node* parentNode = pico->GetComponent<RendererCom>()->GetModel()->FindNode(candyData_[candyID_].nodeName.c_str());

        //子にする
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

        //行列入れる
        DirectX::XMFLOAT4X4 tra;
        DirectX::XMStoreFloat4x4(&tra, Tra);
        candy->transform_->SetWorldTransform(tra);
    }


    GameObjectManager::Instance().Update(elapsedTime);


    if (isSceneEndFlag_)return;

    //演出
    TitleProductionUpdate(elapsedTime);

    //ここで最初のアニメーション再生
    if (!firstFrameSkip_)
    {
        std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("picoTitle");
        std::shared_ptr<AnimationCom> picoAnim = pico->GetComponent<AnimationCom>();

        picoAnim->PlayAnimation(48, true, 0);
        firstFrameSkip_ = true;

        //シェイプ初期設定
        std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
        res->ShapeReset();
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[2].rate = 1;
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[3].rate = 1;
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[7].rate = 0;
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[8].rate = 0;
    }

}

//描画処理
void SceneTitle::Render(float elapsedTime)
{
    if (isSceneEndFlag_)return;
    if (!frameDelayRender_)return;

    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();
    ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
    ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

    //画面クリア＆レンダーターゲット設定
    FLOAT color[] = { 0.0f,0.0f,0.5f,1.0f };    //RGBA(0.0~1.0)
    dc->OMSetRenderTargets(1, &rtv, dsv);

    // 描画処理
    ShaderParameter3D& rc = graphics.shaderParameter3D_;
    rc.lightDirection = graphics.shaderParameter3D_.lightDirection;	// ライト方向

    //カメラパラメーター設定
    rc.view = mainCamera_->GetView();
    rc.projection = mainCamera_->GetProjection();
    DirectX::XMFLOAT3 cameraPos = mainCamera_->GetGameObject()->transform_->GetWorldPosition();
    rc.viewPosition = { cameraPos.x,cameraPos.y,cameraPos.z,1 };

    //影設定   
    std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("picoTitle");
    DirectX::XMFLOAT3  pPos = pico->transform_->GetWorldPosition();
    rc.shadowMapData.shadowCameraPos = { pPos.x,pPos.y,pPos.z,0 };
    rc.shadowMapData.shadowRect = 13;

    //3D描画
    {
        //バッファ避難
        Graphics::Instance().CacheRenderTargets();

        //ポストエフェクト用切り替え
        PostRenderTarget* ps = Graphics::Instance().GetPostEffectModelRenderTarget().get();
        PostDepthStencil* ds = Graphics::Instance().GetPostEffectModelDepthStencilView().get();

        // 画面クリア＆レンダーターゲット設定
        rtv = {};
        dsv = {};
        rtv = ps->renderTargetView.Get();
        dsv = ds->depthStencilView.Get();
        dc->ClearRenderTargetView(rtv, color);
        dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        dc->OMSetRenderTargets(1, &rtv, dsv);

        // ビューポートの設定
        D3D11_VIEWPORT	vp = {};
        vp.Width = static_cast<float>(ps->width);
        vp.Height = static_cast<float>(ps->height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        dc->RSSetViewports(1, &vp);

        //スカイマップ描画
        postEff_->SkymapRender(mainCamera_);

        GameObjectManager::Instance().UpdateTransform();

        GameObjectManager::Instance().Render(mainCamera_->GetView(), mainCamera_->GetProjection());

        // デバッグレンダラ描画実行
        graphics.GetDebugRenderer()->Render(dc, mainCamera_->GetView(), mainCamera_->GetProjection());

        //バッファ戻す
        Graphics::Instance().RestoreRenderTargets();

        postEff_->Render(mainCamera_);
        if (graphics.IsDebugGUI())
            postEff_->ImGuiRender();

    }

    //if (ImGui::Begin("A", nullptr, ImGuiWindowFlags_None))
    //    ImGui::DragFloat3("angleCandy", &candyData_[0].angle.x, 0.01f);
    //    ImGui::DragFloat3("posCandy", &candyData_[0].pos.x, 0.01f);
    //    ImGui::DragFloat3("angleCandy1", &candyData_[1].angle.x, 0.01f);

    //ImGui::End();
}

//演出
void SceneTitle::TitleProductionUpdate(float elapsedTime)
{
    std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("picoTitle");
    std::shared_ptr<AnimationCom> picoAnim = pico->GetComponent<AnimationCom>();
    std::shared_ptr<GameObject> stage = GameObjectManager::Instance().Find("DonutsTitle");
    std::shared_ptr<GameObject> logo = GameObjectManager::Instance().Find("Logo");
    std::shared_ptr<GameObject> pushButton = GameObjectManager::Instance().Find("PushButton");
    std::shared_ptr<GameObject> lookCamera = GameObjectManager::Instance().Find("lookCamera");

    //ステージ上下に動かす
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

    //ロリポップ角度設定
    if (!startFlag_)
    {
        //Xの往復をみて、Zを動かす
        static int roundCount = 0;
        static int roundRand = static_cast<int>(Mathf::RandomRange(2.0f, 6.0f));

        //Xの動き
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

        //Zの動き
        static bool moveZ = false;
        if (roundCount >= roundRand)    //往復カウントが設定した値になったら
        {
            roundCount = 0;
            roundRand = static_cast<int>(Mathf::RandomRange(2.0f, 6.0f));
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

    //瞬き
    {
        eyeTime_ -= elapsedTime;
        static bool eyeDir = true;  //瞬き用フラグ
        if (eyeTime_ < 0)
        {
            eyeTime_ = Mathf::RandomRange(3, 6);
            isShapeEye_ = true;
            eyeDir = true;
        }
        if (isShapeEye_)    //瞬き実行
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
            //シェイプ設定
            std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
            res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[0].rate = shapeEye_;
        }
    }

    //カメラをlookにフォーカス
    {
        DirectX::XMFLOAT3 lerpPos = lookCamera->transform_->GetWorldPosition();
        static  DirectX::XMFLOAT3 oldPos = lerpPos;

        DirectX::XMStoreFloat3(&lerpPos, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&oldPos), DirectX::XMLoadFloat3(&lerpPos), 0.1f));
        oldPos = lerpPos;

        mainCamera_->SetLookAt(lerpPos);

    }


    //スタート時処理
    if (startFlag_)
    {
        if (picoAnim->GetCurrentAnimationIndex() != 49)
            picoAnim->PlayAnimation(49, false);

        //回転
        if (picoAnim->GetCurrentAnimationEvent("turn", DirectX::XMFLOAT3()))
        {
            //回転
            DirectX::XMFLOAT3 picoEuler = pico->transform_->GetEulerRotation();
            picoEuler.y = Mathf::Lerp(picoEuler.y, startAfter_.playerEulerY, 0.1f);
            pico->transform_->SetEulerRotation(picoEuler);
        }
        //lookCamera設定
        if (picoAnim->GetCurrentAnimationEventIsEnd("headFocus"))
        {
            //lookCameraを動かす
            DirectX::XMFLOAT3 lookPos = lookCamera->transform_->GetLocalPosition();
            lookPos.y = Mathf::Lerp(lookPos.y, startAfter_.lookCameraPosY, 0.1f);
            lookCamera->transform_->SetLocalPosition(lookPos);

        }

        //step１顔アップ
        {
            DirectX::XMFLOAT3 headPos;
            if (picoAnim->GetCurrentAnimationEvent("upCamera", headPos))
            {
                //注視点
                lookCamera->transform_->SetWorldPosition(headPos);

                //カメラ動かす
                DirectX::XMFLOAT3 lookPos = { headPos.x + startAfter_.cameraPos1.x,headPos.y + startAfter_.cameraPos1.y,headPos.z + startAfter_.cameraPos1.z };
                DirectX::XMFLOAT3 cPos = mainCamera_->GetGameObject()->transform_->GetWorldPosition();
                DirectX::XMStoreFloat3(&cPos, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&cPos), DirectX::XMLoadFloat3(&lookPos), 0.08f));
                mainCamera_->GetGameObject()->transform_->SetWorldPosition(cPos);

                //ロゴ回転
                DirectX::XMFLOAT3 logoRotate = logo->transform_->GetEulerRotation();
                logoRotate.y += 100*elapsedTime;
                if (logoRotate.y > 200)
                    logoRotate.y = 200;
                logo->transform_->SetEulerRotation(logoRotate);
            }
            else if (picoAnim->GetCurrentAnimationEvent("headFocus", headPos))  //カメラをフォーカス
            {
                lookCamera->transform_->SetWorldPosition(headPos);
            }
        }

        //step２顔にアップ後処理
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

                //シェイプ設定
                std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
                res->ShapeReset();
            }

            {
                if (isStopFlag_)
                {
                    //シェイプ設定
                    std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
                    float rate7 = res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[7].rate;
                    float rate8 = res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[8].rate;
                    rate7 = Mathf::Lerp(rate7, 1, 0.1f);
                    rate8 = Mathf::Lerp(rate8, 0.5f, 0.1f);
                    res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[7].rate = rate7;
                    res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[8].rate = rate8;
                }
            }

            //動きとめる
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

        //step３顔アップから引く処理
        {
            if (picoAnim->GetCurrentAnimationEventIsEnd("headFocus"))
            {
                DirectX::XMFLOAT3 cPos = mainCamera_->GetGameObject()->transform_->GetWorldPosition();
                DirectX::XMStoreFloat3(&cPos, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&cPos), DirectX::XMLoadFloat3(&startAfter_.cameraPos2), 0.08f));
                mainCamera_->GetGameObject()->transform_->SetWorldPosition(cPos);

                //エフェクト流す
                std::shared_ptr<ParticleSystemCom> particle = SceneManager::Instance().GetParticleObj()->GetComponent<ParticleSystemCom>();
                if (!particle->GetEnabled())
                {
                    particle->SetEnabled(true);
                    particle->GetSaveParticleData().particleData.isRoop = true;
                    particle->IsRestart();
                    SceneManager::Instance().SetParticleUpdate(true);
                }

            }

        }

        //ロリポップサイズ設定
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


        //アニメーション終わり
        if (!picoAnim->IsPlayAnimation())
        {
            //シーン遷移フラグON
            isSceneEndFlag_ = true;

            //仮処理
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

                //シェイプ初期設定
                std::shared_ptr<FbxModelResource> res = pico->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
                res->ShapeReset();
                res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[2].rate = 1;
                res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[3].rate = 1;
                res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[7].rate = 0;
                res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[8].rate = 0;
            }
        }

        //瞬きをしなくする
        eyeTime_ = 100;
    }
}
