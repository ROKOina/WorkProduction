#include "Graphics/Graphics.h"
#include "Input/Input.h"
#include "SceneLoading.h"
#include "SceneManager.h"

//初期化
void SceneLoading::Initialize()
{
    //スプライト初期化
    sprite_ = new Sprite("Data/Sprite/yellow.png");
    
    //スレッド開始
    thread_ = new std::thread(LoadingThread,this);

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
        postEff_->SetSunEnabled(false);
    }

}

//終了化
void SceneLoading::Finalize()
{
    //スレッド終了化
    if (thread_ != nullptr)
    {
        thread_->detach();
        delete thread_;
        thread_ = nullptr;
    }

    //スプライト終了化
    if (sprite_ != nullptr)
    {
        delete sprite_;
        sprite_ = nullptr;
    }
}

//更新処理
void SceneLoading::Update(float elapsedTime)
{
    //次のシーンの準備が完了したらシーンを切り替える
    if (nextScene_->IsReady())
    {
        SceneManager::Instance().ChangeScene(nextScene_);
    }
}


//描画処理
void SceneLoading::Render(float elapsedTime)
{
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();
    ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
    ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

    //画面クリア＆レンダーターゲット設定
    FLOAT color[] = { 0.0f,0.0f,0.5f,1.0f };    //RGBA(0.0~1.0)
    dc->ClearRenderTargetView(rtv, color);
    dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    dc->OMSetRenderTargets(1, &rtv, dsv);


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

        //2Dスプライト描画
        {
            //画面下にローでイングアイコンを描画
            float screenWidth = static_cast<float>(graphics.GetScreenWidth());
            float screenHeight = static_cast<float>(graphics.GetScreenHeight());
            float textureWidth = static_cast<float>(sprite_->GetTextureWidth());
            float textureHeight = static_cast<float>(sprite_->GetTextureHeight());
            float positionX = screenWidth - textureWidth;
            float positionY = screenHeight - textureHeight;

            sprite_->Render(dc,
                0, 0, screenWidth, screenHeight,
                0, 0, textureWidth, textureHeight,
                0,
                1, 1, 1, 1);
        }

        //バッファ戻す
        graphics.RestoreRenderTargets();

        postEff_->Render(mainCamera_);
        if (graphics.IsDebugGUI())
            postEff_->ImGuiRender();
    }
}


//ローディングスレッド
void SceneLoading::LoadingThread(SceneLoading* scene)
{
    //COM関連の初期化でスレッド毎に呼ぶ必要がある
    CoInitialize(nullptr);

    //次のシーンの初期化を行う
    scene->nextScene_->Initialize();

    //スレッドが終わる前にCOM関連の終了化
    CoUninitialize();

    //次のシーンの準備完了設定
    scene->nextScene_->SetReady();
}
