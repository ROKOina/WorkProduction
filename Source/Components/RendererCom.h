#pragma once

#include "System\Component.h"

class RenderderCom : public Component
{
    //コンポーネントオーバーライド
public: 
    RenderderCom() {}
    ~RenderderCom() {}

    // 名前取得
    const char* GetName() const override { return "Renderer"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;


    //Rendererクラス
public:
    // モデルの読み込み
    void LoadModel(const char* filename);

    // モデルの取得
    Model* GetModel() const { return model.get(); }

private:
    std::unique_ptr<Model>	model;

};