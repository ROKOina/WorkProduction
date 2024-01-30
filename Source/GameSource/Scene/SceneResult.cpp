#include "Graphics/Graphics.h"
#include "SceneResult.h"

#include "SceneTitle.h"
#include "SceneManager.h"
#include "Input/Input.h"
#include "SceneLoading.h"
#include "imgui.h"

#include "Components\ParticleSystemCom.h"
#include "../ScriptComponents/Score.h"

//初期化
void SceneResult::Initialize()
{
    resultEndFlag_ = false;
    resultStartFlag_ = true;
    transitionInTimer_ = 3;
    showIntervalTimer_ = 0;

    //遷移パーティクル
    std::shared_ptr<ParticleSystemCom> particle = SceneManager::Instance().GetParticleObj()->GetComponent<ParticleSystemCom>();
    particle->GetSaveParticleData().particleData.isRoop = false;
    SceneManager::Instance().SetParticleUpdate(true);

    //マックスコンボ
    //Score::Instance().maxCombo_ = 123;
    int maxCombo = Score::Instance().maxCombo_;

    //最大値を超えた場合
    if (maxCombo > 999)
    {
        maxCombo = 999;
        Score::Instance().maxCombo_ = maxCombo;
    }

    //コンボ数でスコア加算
    Score::Instance().score_ += maxCombo * 98712;

    //スコア
    //Score::Instance().score_ = 12345678912345;
    BIG_LONG score = Score::Instance().score_;

    //最大値を超えた場合
    if (score > 999999999999999)
    {
        score = 999999999999999;
        Score::Instance().score_ = score;
    }

    //桁数取得
    while (score > 0)
    {
        score /= 10;
        ketaCount_++;
    }

    //ライン初期化
    {
        //数字下
        lineData_.emplace_back(DirectX::XMFLOAT4(0, 626.7f, 755.7f, 4.0f));
        lineData_.emplace_back(DirectX::XMFLOAT4(0, 379.7f, 755.7f, 4.0f));

        //枠
        lineData_.emplace_back(DirectX::XMFLOAT4(0, 100, 1920, 5.0f), DirectX::XMFLOAT4(0, 0.5f, 1, 1));
        lineData_.emplace_back(DirectX::XMFLOAT4(0, 763, 1920, 5.0f), DirectX::XMFLOAT4(0, 0.5f, 1, 1));
        lineData_.emplace_back(DirectX::XMFLOAT4(765, 100, 5.0f, 667), DirectX::XMFLOAT4(0, 0.5f, 1, 1));
    }

    //BGM
    BGM_->Stop();
    BGM_->Play(true);
}

//終了化
void SceneResult::Finalize()
{
    BGM_->AudioRelease();
    pushSE_->AudioRelease();
}

//更新処理
void SceneResult::Update(float elapsedTime)
{
    GamePad& gamePad = Input::Instance().GetGamePad();
    const GamePadButton anyButton = GamePad::BTN_B;

    //シーン開始処理
    if (resultStartFlag_)
    {
        if (transitionInTimer_ > 0)
        {
            transitionInTimer_ -= elapsedTime;
        }
        if (transitionInTimer_ <= 0)
        {
            //遷移パーティクル終了
            std::shared_ptr<ParticleSystemCom> particle = SceneManager::Instance().GetParticleObj()->GetComponent<ParticleSystemCom>();
            particle->SetEnabled(false);
            SceneManager::Instance().SetParticleUpdate(false);
            resultStartFlag_ = false;
        }
        //シーン開始処理中はreturn
        return;
    }


    //シーン終了処理
    if (ketaCount_ <= showKeta_)
    {
        if (gamePad.GetButtonDown() & anyButton && !resultEndFlag_)
        {
            resultEndFlag_ = true;

            //遷移パーティクル開始
            std::shared_ptr<ParticleSystemCom> particle = SceneManager::Instance().GetParticleObj()->GetComponent<ParticleSystemCom>();
            particle->GetSaveParticleData().particleData.isRoop = true;
            particle->IsRestart();
            particle->SetEnabled(true);
            SceneManager::Instance().SetParticleUpdate(true);
            transitionOutTimer_ = 2.5f;

            pushSE_->Stop();
            pushSE_->Play(false);
        }
    }

    //終了演出
    if (resultEndFlag_)
    {
        transitionOutTimer_ -= elapsedTime;
        if (transitionOutTimer_ < 0)
        {
            //シーン切り替え
            SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTitle));
            SceneManager::Instance().SetParticleUpdate(false);
        }
    }

    //リザルト演出

    //表示桁を増やす
    if (ketaCount_ > showKeta_)
    {
        showIntervalTimer_ += elapsedTime;
        if (showIntervalTimer_ > showInterval_)
        {
            showIntervalTimer_ = 0;
            showKeta_++;
            Eff2D eff;
            eff.pos = { numScoreData_.x - showKeta_ * numScoreData_.w, numScoreData_.y };
            resultEffects_.emplace_back(eff);
            kiraSE_->Stop();
            kiraSE_->Play(false);
        }
    }

    //エフェクト更新
    for (auto& eff : resultEffects_)
    {
        //エフェクト次の画像
        eff.timer += elapsedTime;
        if (eff.timer > effSpeed_)
        {
            eff.timer = 0;
            eff.effID++;
        }
    }
    //エフェクト削除処理
    while (1)
    {
        if (resultEffects_.size() <= 0)
            break;

        if (resultEffects_[0].effID > effHeight_ * effWidth_ - 1)
        {
            resultEffects_.erase(resultEffects_.begin());
        }
        else
        {
            break;
        }
    }
}

//描画処理
void SceneResult::Render(float elapsedTime)
{
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();
    ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
    ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

    //画面クリア＆レンダーターゲット設定
    FLOAT color[] = { 0.0f,0.0f,0.0f,1.0f };    //RGBA(0.0~1.0)
    dc->ClearRenderTargetView(rtv, color);
    dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    dc->OMSetRenderTargets(1, &rtv, dsv);

    //背景
    {
        backSprite_->Render(dc, backData_.x, backData_.y
            , backData_.z, backData_.w
            , 0, 0, static_cast<float>(backSprite_->GetTextureWidth()), static_cast<float>(backSprite_->GetTextureHeight())
            , 0, backColor_.x, backColor_.y, backColor_.z, backColor_.w);
        resultArtSprite_->Render(dc, resultArtPos_.x, resultArtPos_.y
            , resultArtSprite_->GetTextureWidth() * resultArtPos_.z, resultArtSprite_->GetTextureHeight() * resultArtPos_.z
            , 0, 0, static_cast<float>(resultArtSprite_->GetTextureWidth()), static_cast<float>(resultArtSprite_->GetTextureHeight())
            , 0, 1, 1, 1, 1);
    }

    //文字
    {
        //リザルト
        resultSprite_->Render(dc, resultPos_.x, resultPos_.y
            , resultSprite_->GetTextureWidth() * resultPos_.z, resultSprite_->GetTextureHeight() * resultPos_.z
            , 0, 0, static_cast<float>(resultSprite_->GetTextureWidth()), static_cast<float>(resultSprite_->GetTextureHeight())
            , 0, 1, 1, 1, 1);
        //スコア
        scoreSprite_->Render(dc, scorePos_.x, scorePos_.y
            , scoreSprite_->GetTextureWidth() * scorePos_.z, scoreSprite_->GetTextureHeight() * scorePos_.z
            , 0, 0, static_cast<float>(scoreSprite_->GetTextureWidth()), static_cast<float>(scoreSprite_->GetTextureHeight())
            , 0, 1, 1, 1, 1);
        //マックスコンボ
        maxComboSprite_->Render(dc, maxComboPos_.x, maxComboPos_.y
            , maxComboSprite_->GetTextureWidth() * maxComboPos_.z, maxComboSprite_->GetTextureHeight() * maxComboPos_.z
            , 0, 0, static_cast<float>(maxComboSprite_->GetTextureWidth()), static_cast<float>(maxComboSprite_->GetTextureHeight())
            , 0, 1, 1, 1, 1);

        if (ketaCount_ <= showKeta_)
        {
            //ボタン
            static float timer = 0;
            timer += elapsedTime;
            float sinPosY = sin(timer);
            buttonSprite_->Render(dc, buttonPos_.x, buttonPos_.y + sinPosY * 5
                , buttonSprite_->GetTextureWidth() * buttonPos_.z, buttonSprite_->GetTextureHeight() * buttonPos_.z
                , 0, 0, static_cast<float>(buttonSprite_->GetTextureWidth()), static_cast<float>(buttonSprite_->GetTextureHeight())
                , 0, 1, 1, 1, 1);
        }
    }

    //ライン
    for (auto& lineData : lineData_)
    {
        lineSprite_->Render(dc, lineData.line.x, lineData.line.y
            , lineData.line.z, lineData.line.w
            , 0, 0, static_cast<float>(lineSprite_->GetTextureWidth()), static_cast<float>(lineSprite_->GetTextureHeight())
            , 0, lineData.color.x, lineData.color.y, lineData.color.z, lineData.color.w);
    }
    lineSprite_->Render(dc, linePosData_.x, linePosData_.y
        , linePosData_.z, linePosData_.w
        , 0, 0, static_cast<float>(lineSprite_->GetTextureWidth()), static_cast<float>(lineSprite_->GetTextureHeight())
        , 0, 1, 1, 1, 1);

    //マックスコンボ表示
    {
        int maxCombo = Score::Instance().maxCombo_;
        for (int keta = 0; keta < 3; ++keta)
        {
            if (maxCombo <= 0)break;

            int num = maxCombo % 10;
            maxCombo /= 10;

            numSprite_->Render(dc, numMaxComboData_.x - keta * numMaxComboData_.w, numMaxComboData_.y
                , static_cast<float>(numSprite_->GetTextureWidth()) * 0.1f * numMaxComboData_.z, static_cast<float>(numSprite_->GetTextureHeight()) * numMaxComboData_.z
                , 170.0f * num, 0
                , static_cast<float>(numSprite_->GetTextureWidth()) * 0.1f, static_cast<float>(numSprite_->GetTextureHeight())
                , 0, numScoreColor_.x, numScoreColor_.y, numScoreColor_.z, numScoreColor_.w);
        }
    }

    //スコア表示
    {
        BIG_LONG score = Score::Instance().score_;

        //スコア
        for (int keta = 0; keta < 15; ++keta)
        {
            if (keta >= showKeta_)break;    //表示桁数
            if (score <= 0)break;

            int num = score % 10;
            score /= 10;

            numSprite_->Render(dc, numScoreData_.x - keta * numScoreData_.w, numScoreData_.y
                , static_cast<float>(numSprite_->GetTextureWidth()) * 0.1f * numScoreData_.z, static_cast<float>(numSprite_->GetTextureHeight()) * numScoreData_.z
                , 170.0f * num, 0
                , static_cast<float>(numSprite_->GetTextureWidth()) * 0.1f, static_cast<float>(numSprite_->GetTextureHeight())
                , 0, numScoreColor_.x, numScoreColor_.y, numScoreColor_.z, numScoreColor_.w);
        }

        //演出数字
        if (ketaCount_ > showKeta_)
        {
            for (int keta = 0; keta < 15; ++keta)
            {
                if (keta < showKeta_)continue;
                int ranNum = rand() % 10;

                numSprite_->Render(dc, numScoreData_.x - keta * numScoreData_.w, numScoreData_.y
                    , static_cast<float>(numSprite_->GetTextureWidth()) * 0.1f * numScoreData_.z, static_cast<float>(numSprite_->GetTextureHeight()) * numScoreData_.z
                    , 170.0f * ranNum, 0
                    , static_cast<float>(numSprite_->GetTextureWidth()) * 0.1f, static_cast<float>(numSprite_->GetTextureHeight())
                    , 0, numScoreColor_.x, numScoreColor_.y, numScoreColor_.z, numScoreColor_.w);
            }
        }

        //演出エフェクト
        DirectX::XMFLOAT2 effSize = { static_cast<float>(numEffSprite_->GetTextureWidth() / effWidth_) ,static_cast<float>(numEffSprite_->GetTextureHeight() / effHeight_) };
        for (auto& eff : resultEffects_)
        {
            numEffSprite_->Render(dc, eff.pos.x - 20, eff.pos.y - 20
                , effSize.x * 1.5f, effSize.y * 1.5f
                , effSize.x * (eff.effID % effWidth_), effSize.y * (eff.effID / effWidth_)
                , effSize.x, effSize.y
                , 0, 1, 1, 1, 1);
        }
    }


    //debug
    if (graphics.IsDebugGUI())
    {
        if (ImGui::Begin("Result", nullptr, ImGuiWindowFlags_None))
        {
            //int s = Score::Instance().score_;
            //ImGui::DragInt("score", &s);
            //Score::Instance().score_ = s;

            ImGui::DragInt("showKeta_", &showKeta_);
            ImGui::DragFloat4("num", &numScoreData_.x, 0.1f);
            ImGui::DragFloat4("numColor_", &numScoreColor_.x, 0.1f);
            ImGui::DragFloat4("numMaxComboData_", &numMaxComboData_.x, 0.1f);

            ImGui::DragFloat4("backData_", &backData_.x, 0.1f);
            ImGui::DragFloat4("backColor_", &backColor_.x, 0.1f);

            ImGui::DragFloat3("resultPos_", &resultPos_.x);
            ImGui::DragFloat3("scorePos_", &scorePos_.x);
            ImGui::DragFloat3("maxComboPos_", &maxComboPos_.x);
            ImGui::DragFloat3("buttonPos_", &buttonPos_.x);

            ImGui::DragFloat4("abc", &linePosData_.x, 0.1f);

        }
        ImGui::End();
    }

}