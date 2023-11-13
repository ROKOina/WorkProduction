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

    //�`��
    void Render(std::shared_ptr<CameraCom> camera);

    //ImGui�`��
    void ImGuiRender();

    //�X�J�C�}�b�v�`��
    void SkymapRender(std::shared_ptr<CameraCom> camera);

    //�����Y�t���A�L���ݒ�
    void SetSunEnabled(bool enabled) { sun_->SetEnabled(enabled); }

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
    std::unique_ptr<ShaderPost> bloomKawaseFilter_;  //�쐣��
    std::unique_ptr<ShaderPost> bloomBlur_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> bloomBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> bloomExBuffer_;

    //���z���
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


    //�X�J�C�}�b�v
    bool enabledSkyMap_ = true;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skymapSrv_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> skymapBuffer_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>			skyVertex_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>			skyPixel_;

    std::unique_ptr<ShaderPost> postEffect_;
    std::unique_ptr<ShaderPost> postRender_;

    std::unique_ptr<TextureFormat> drawTexture_;
    std::unique_ptr<PostRenderTarget> renderPost_[BloomCount];   //0:�P�x�A1�`:�u���[
    std::unique_ptr<PostRenderTarget> renderPostSun_;
    std::unique_ptr<PostRenderTarget> renderPostFull_;  //�t���X�N���[���p�̃����_�[�^�[�Q�b�g
};

