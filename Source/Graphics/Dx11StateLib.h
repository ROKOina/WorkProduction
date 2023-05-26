#pragma once

#include <wrl.h>
#include <d3d11.h>

//dx11�`�����̃X�e�[�g�����������Ă���
class Dx11StateLib
{
public:
    Dx11StateLib() {}
    ~Dx11StateLib() {}

    //Init
    void Dx11StateInit(ID3D11Device* device);


    //shader�Z�b�g
public:
    HRESULT createVsFromCso(ID3D11Device* device, const char* cso_name, ID3D11VertexShader** vertex_shader,
        ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements);

    HRESULT createPsFromCso(ID3D11Device* device, const char* cso_name, ID3D11PixelShader** pixel_shader);

    //�R���X�^���g�o�b�t�@����
public:
    HRESULT createConstantBuffer(ID3D11Device* device, UINT ByteWidth, ID3D11Buffer** buffer);

    //�u�����h�X�e�[�g
public:
    enum class BLEND_STATE_TYPE {
        ALPHA,          //����
        ADDITION,       //���Z
        ADDITION_ALPHA, //���Z�i���߁j
        SUBTRACTION,    //���Z
        SCREEN,         //�X�N���[��
        PARTICLE,       //�p�[�e�B�N��
        DEFALT,           //�����Ȃ��i�ی��j
    };
    Microsoft::WRL::ComPtr<ID3D11BlendState> GetBlendState(BLEND_STATE_TYPE blendType) { return blendState[static_cast<int>(blendType)].Get(); }
private:
    Microsoft::WRL::ComPtr<ID3D11BlendState> blendState[static_cast<int>(BLEND_STATE_TYPE::DEFALT) + 1];

    //�[�x�X�e���V���X�e�[�g
public:
    enum class DEPTHSTENCIL_STATE_TYPE {
        DEPTH_ON_3D,   //�[�x�e�X�gON 3D�ɐݒ肳��Ă���
        DEPTH_ON_2D,   //�[�x�e�X�gON 2D�ɐݒ肳��Ă���
        DEPTH_OFF,   //�[�x�e�X�gOFF
        PARTICLE,  //�p�[�e�B�N��
        DEFALT,           //�����Ȃ��i�ی��j
    };
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> GetDepthStencilState(DEPTHSTENCIL_STATE_TYPE depthStencilType) { return depthStencilState[static_cast<int>(depthStencilType)].Get(); }
private:
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::DEFALT) + 1];

    //���X�^���C�U�[�X�e�[�g
public:
    enum class RASTERIZER_TYPE {
        FRONTCOUNTER_FALSE_CULLBACK,  //FrontCounterClockwise�ƃJ�����[�h�ݒ�
        FRONTCOUNTER_FALSE_CULLNONE,  
        FRONTCOUNTER_TRUE_CULLNONE,
        PARTICLE,   //�p�[�e�B�N��
        DEFALT,           //�����Ȃ��i�ی��j
    };
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> GetRasterizerState(RASTERIZER_TYPE rasterizerType) { return rasterizerState[static_cast<int>(rasterizerType)].Get(); }
private:
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState[static_cast<int>(RASTERIZER_TYPE::DEFALT) + 1];

    //�T���v���[�X�e�[�g
public:
    enum class SAMPLER_TYPE {
        TEXTURE_ADDRESS_WRAP,
        DEFALT,           //�����Ȃ��i�ی��j
    };
    Microsoft::WRL::ComPtr<ID3D11SamplerState> GetSamplerState(SAMPLER_TYPE samplerType) { return samplerState[static_cast<int>(samplerType)].Get(); }
private:
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState[static_cast<int>(SAMPLER_TYPE::DEFALT) + 1];


};

