#pragma once

#include <memory>
#include <wrl.h>
#include <d3d11.h>
#include "TextureFormat.h"
#include "Graphics\Shaders\3D\ShaderParameter3D.h"

class PostEffect
{
public:
    PostEffect(UINT width, UINT height);
    ~PostEffect() {}

    //描画
    void Render();

    //ImGui描画
    void ImGuiRender();

    //スカイマップ描画
    void SkymapRender();

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

    //スカイマップ
    bool enabledSkyMap_ = true;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skymapSrv_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> skymapBuffer_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>			skyVertex_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>			skyPixel_;

    std::unique_ptr<ShaderPost> postEffect_;
    std::unique_ptr<ShaderPost> postRender_;

    std::unique_ptr<TextureFormat> drawTexture_;
    std::unique_ptr<PostRenderTarget> renderPost_[5];
    std::unique_ptr<PostRenderTarget> renderPostFull_;  //フルスクリーン用のレンダーターゲット
};

