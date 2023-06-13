#pragma once

#include "System\Component.h"
#include "Graphics\Model\Model.h"
#include "Graphics\Graphics.h"

class RendererCom : public Component
{
    //コンポーネントオーバーライド
public: 
    RendererCom() {}
    ~RendererCom() {}

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
    Model* GetModel() const { return model_.get(); }

    //シェーダーIDゲット
    const int GetShaderID() const { return shaderID_; }
    void SetShaderID(SHADER_ID shaderID) { shaderID_ = shaderID; }

private:
    std::unique_ptr<Model>	model_;

    int shaderID_ = SHADER_ID::Phong;  //シェーダーを決める
};