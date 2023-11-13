#pragma once

#include <memory>
#include <wrl.h>
#include <d3d11.h>
#include "TextureFormat.h"
#include "Graphics\Shaders\3D\ShaderParameter3D.h"

class CameraCom;

#define BlurCount 6
#define BloomCount BlurCount + 1

class PostEffect
{
public:
    PostEffect(UINT width, UINT height);
    ~PostEffect() {}

    //描画
    void Render(std::shared_ptr<CameraCom> camera);

    //ImGui描画
    void ImGuiRender();

    //スカイマップ描画
    void SkymapRender(std::shared_ptr<CameraCom> camera);

    //レンズフレア有効設定
    void SetSunEnabled(bool enabled) { sun_->SetEnabled(enabled); }

private:
    //ポストエフェクト用構造体
    struct ShaderPost
    {
    public:
        ShaderPost(const char* PS);

        void Draw(TextureFormat* renderTexture);
        bool IsEnabled() { return isEnabled; }
        void SetEnabled(bool enabled) { isEnabled = enabled; }

    private:
        Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexShaderPost;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelShaderPost;
        Microsoft::WRL::ComPtr<ID3D11InputLayout>			inputLayoutPost;
        bool isEnabled = true;  //有効にするか
    };

private:
    //カラーグレーディング
    std::unique_ptr<ShaderPost> colorGrading_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> colorGradingBuffer_;

    //ブルーム
    std::unique_ptr<ShaderPost> bloomExtract_;
    std::unique_ptr<ShaderPost> bloomKawaseFilter_;  //川瀬式
    std::unique_ptr<ShaderPost> bloomBlur_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> bloomBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> bloomExBuffer_;

    //太陽回り
    std::unique_ptr<ShaderPost> sun_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> sunBuffer_;

    struct CbScene
    {
        DirectX::XMFLOAT4		viewPosition;
        DirectX::XMFLOAT4X4     viewProjection;
        DirectX::XMFLOAT4		ambientLightColor;
        DirectX::XMFLOAT4		lightDirection;
        DirectX::XMFLOAT4		lightColor;
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> sceneBuffer_;


    //スカイマップ
    bool enabledSkyMap_ = true;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skymapSrv_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> skymapBuffer_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>			skyVertex_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>			skyPixel_;

    std::unique_ptr<ShaderPost> postEffect_;
    std::unique_ptr<ShaderPost> postRender_;

    std::unique_ptr<TextureFormat> drawTexture_;
    std::unique_ptr<PostRenderTarget> renderPost_[BloomCount];   //0:輝度、1～:ブラー
    std::unique_ptr<PostRenderTarget> renderPostSun_;
    std::unique_ptr<PostRenderTarget> renderPostFull_;  //フルスクリーン用のレンダーターゲット
};

