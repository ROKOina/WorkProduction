#include "Graphics/Graphics.h"
#include "Input/Input.h"
#include "SceneLoading.h"
#include "SceneManager.h"

//初期化
void SceneLoading::Initialize()
{
    //スプライト初期化
    sprite_ = new Sprite("Data/Sprite/LoadingIcon.png");
    
    //スレッド開始
    thread_ = new std::thread(LoadingThread,this);
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
    constexpr float speed = 180;
    angle_ += speed * elapsedTime;

    //次のシーンの準備が完了したらシーンを切り替える
    if (nextScene_->IsReady())
    {
        SceneManager::Instance().ChangeScene(nextScene_);
    }
}


//描画処理
void SceneLoading::Render()
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
            positionX, positionY, textureWidth, textureHeight,
            0, 0, textureWidth, textureHeight,
            angle_,
            1, 1, 1, 1);
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
