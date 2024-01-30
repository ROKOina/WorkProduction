#pragma once

#include "Graphics/Sprite/Sprite.h"
#include "Scene.h"

#include "Audio\AudioSource.h"
#include "Audio\Audio.h"

//リザルトシーン
class SceneResult :public Scene
{
public:
    SceneResult() {}
    ~SceneResult()override
    { }

    //初期化
    void Initialize()override;

    //終了化
    void Finalize()override;

    //更新処理
    void Update(float elapsedTime)override;

    //描画処理
    void Render(float elapsedTime)override;

private:

private:    
    //画像
    std::unique_ptr<Sprite> resultArtSprite_ = std::make_unique<Sprite>("./Data/Sprite/result/resultArt.png");
    DirectX::XMFLOAT3 resultArtPos_ = { 768,100,1 };
    
    //文字
    std::unique_ptr<Sprite> resultSprite_ = std::make_unique<Sprite>("./Data/Sprite/result/RESULT.png");
    DirectX::XMFLOAT3 resultPos_ = { 9,96,0.3f };
    std::unique_ptr<Sprite> scoreSprite_ = std::make_unique<Sprite>("./Data/Sprite/result/SCORE.png");
    DirectX::XMFLOAT3 scorePos_ = { 38,487,0.23f };
    std::unique_ptr<Sprite> maxComboSprite_ = std::make_unique<Sprite>("./Data/Sprite/result/MAXCOMBO.png");
    DirectX::XMFLOAT3 maxComboPos_ = { 38,240,0.23f };
    std::unique_ptr<Sprite> buttonSprite_ = std::make_unique<Sprite>("./Data/Sprite/result/PushButton.png");
    DirectX::XMFLOAT3 buttonPos_ = { 1097,703,0.23f };
    
    //背景
    std::unique_ptr<Sprite> backSprite_ = std::make_unique<Sprite>("./Data/Sprite/result/resultBack.png");
    DirectX::XMFLOAT4 backData_ = { 0,100,768,664.2f };  //xy:pos zw:size
    DirectX::XMFLOAT4 backColor_ = { 1,1.7f,0,1 };
    //ライン用
    std::unique_ptr<Sprite> lineSprite_ = std::make_unique<Sprite>("./Data/Sprite/rect.png");
    struct LineData
    {
        LineData(DirectX::XMFLOAT4 line, DirectX::XMFLOAT4 color = {1,1,1,1})
            :line(line), color(color) {}
        DirectX::XMFLOAT4 line;
        DirectX::XMFLOAT4 color;
    };
    std::vector<LineData> lineData_;
    DirectX::XMFLOAT4 linePosData_ = { 0,626.7f,755.7f,4 };
    //数字
    int ketaCount_; //桁数
    std::unique_ptr<Sprite> numSprite_ = std::make_unique<Sprite>("./Data/Sprite/number.png");
    DirectX::XMFLOAT4 numMaxComboData_ = { 697,308,0.3f,43 };   //xy:pos z:size w:間隔
    DirectX::XMFLOAT4 numScoreData_ = { 697,555,0.3f,43 };   //xy:pos z:size w:間隔
    DirectX::XMFLOAT4 numScoreColor_ = { 3,0.3f,0.2f,1 };

    //演出
    int showKeta_ = 0;  //演出用表示桁
    float showInterval_ = 0.1f;
    float showIntervalTimer_ = 0.0f;
    //表示時エフェクト
    int effHeight_ = 3;
    int effWidth_ = 4;
    std::unique_ptr<Sprite> numEffSprite_ = std::make_unique<Sprite>("./Data/Sprite/result/resultEff.png");
    struct Eff2D
    {
        DirectX::XMFLOAT2 pos;
        float timer = 0;
        int effID = 0;
    };
    float effSpeed_ = 0.05f;
    std::vector<Eff2D> resultEffects_;

    bool resultEndFlag_ = false;
    bool resultStartFlag_ = false;
    float transitionOutTimer_;
    float transitionInTimer_;


    //BGM
    std::unique_ptr<AudioSource> BGM_ = Audio::Instance().LoadAudioSource("Data/Audio/resultBGM.wav");
    //SE
    std::unique_ptr<AudioSource> pushSE_ = Audio::Instance().LoadAudioSource("Data/Audio/push.wav");
    std::unique_ptr<AudioSource> kiraSE_ = Audio::Instance().LoadAudioSource("Data/Audio/kira.wav");
};