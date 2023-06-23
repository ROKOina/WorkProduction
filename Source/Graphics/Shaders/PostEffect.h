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

    //�`��
    void Render();

    //ImGui�`��
    void ImGuiRender();

private:
    //�|�X�g�G�t�F�N�g�p�\����
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
        bool isEnabled = true;  //�L���ɂ��邩
    };

private:
    //�J���[�O���[�f�B���O
    std::unique_ptr<ShaderPost> colorGrading_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> colorGradingBuffer_;

    //�u���[��
    std::unique_ptr<ShaderPost> bloomExtract_;
    std::unique_ptr<ShaderPost> bloomBlur_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> bloomBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> bloomExBuffer_;

    std::unique_ptr<ShaderPost> postEffect_;
    std::unique_ptr<ShaderPost> postRender_;

    std::unique_ptr<TextureFormat> drawTexture_;
    std::unique_ptr<PostRenderTarget> renderPost_[2];
    std::unique_ptr<PostRenderTarget> renderPostFull_;  //�t���X�N���[���p�̃����_�[�^�[�Q�b�g
};

