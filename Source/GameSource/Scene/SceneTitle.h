#pragma once

#include "Graphics/Sprite/Sprite.h"
#include "Scene.h"

//タイトルシーン
class ScneTitle :public Scene
{
public:
    ScneTitle(){}
    ~ScneTitle()override{}

    //初期化
    void Initialize()override;

    //終了化
    void Finalize()override;

    //更新処理
    void Update(float elapsedTime)override;

    //描画処理
    void Render()override;

private:
    Sprite* sprite = nullptr;
    std::unique_ptr<Sprite> alphaS = std::make_unique<Sprite>("Data/Sprite/LoadingIcon.png");
};