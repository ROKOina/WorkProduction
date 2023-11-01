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
        ID3D11InputLayout** inputLayout_, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements);
    HRESULT createPsFromCso(ID3D11Device* device, const char* cso_name, ID3D11PixelShader** pixel_shader);
    HRESULT createGsFromCso(ID3D11Device* device, const char* cso_name, ID3D11GeometryShader** geometry_shader);
    HRESULT createCsFromCso(ID3D11Device* device, const char* cso_name, ID3D11ComputeShader** compute_shader);

    HRESULT load_texture_from_file(ID3D11Device* device, const char* filename, 
        ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc);

    //�R���X�^���g�o�b�t�@����
public:
    HRESULT createConstantBuffer(ID3D11Device* device, UINT ByteWidth, ID3D11Buffer** buffer);

    //�u�����h�X�e�[�g
public:
    enum class BLEND_STATE_TYPE {
        ALPHA,          //����
        ALPHA_ATC,      //����_AlphaToCoverageEnable
        ADDITION,       //���Z
        ADDITION_ALPHA, //���Z�i���߁j
        SUBTRACTION,    //���Z
        SCREEN,         //�X�N���[��
        PARTICLE,       //�p�[�e�B�N��
        DEFALT,         //�����Ȃ��i�ی��j
    };
    Microsoft::WRL::ComPtr<ID3D11BlendState> GetBlendState(BLEND_STATE_TYPE blendType) { return blendState_[static_cast<int>(blendType)].Get(); }
private:
    Microsoft::WRL::ComPtr<ID3D11BlendState> blendState_[static_cast<int>(BLEND_STATE_TYPE::DEFALT) + 1];

    //�[�x�X�e���V���X�e�[�g
public:
    enum class DEPTHSTENCIL_STATE_TYPE {
        DEPTH_ON_3D,   //�[�x�e�X�gON 3D�ɐݒ肳��Ă���
        DEPTH_ON_2D,   //�[�x�e�X�gON 2D�ɐݒ肳��Ă���
        DEPTH_OFF,   //�[�x�e�X�gOFF
        SKYMAP,   //�X�J�C�}�b�v
        PARTICLE,  //�p�[�e�B�N��
        DEFALT,           //�����Ȃ��i�ی��j
    };
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> GetDepthStencilState(DEPTHSTENCIL_STATE_TYPE depthStencilType) { return depthStencilState_[static_cast<int>(depthStencilType)].Get(); }
private:
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState_[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::DEFALT) + 1];

    //���X�^���C�U�[�X�e�[�g
public:
    enum class RASTERIZER_TYPE {
        FRONTCOUNTER_FALSE_CULLBACK,  //FrontCounterClockwise�ƃJ�����[�h�ݒ�
        FRONTCOUNTER_FALSE_CULLNONE,  
        FRONTCOUNTER_TRUE_CULLNONE,
        FRONTCOUNTER_FALSE_CULLNONE_WIREFRAME,  //���C���t���[��
        SKYMAP,   //�X�J�C�}�b�v
        PARTICLE,   //�p�[�e�B�N��
        DEFALT,           //�����Ȃ��i�ی��j
    };
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> GetRasterizerState(RASTERIZER_TYPE rasterizerType) { return rasterizerState_[static_cast<int>(rasterizerType)].Get(); }
private:
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_[static_cast<int>(RASTERIZER_TYPE::DEFALT) + 1];

    //�T���v���[�X�e�[�g
public:
    enum class SAMPLER_TYPE {
        TEXTURE_ADDRESS_WRAP,
        TEXTURE_ADDRESS_CLAMP,
        TEXTURE_ADDRESS_BORDER_LINER,
        TEXTURE_ADDRESS_WRAP_ANISO,
        TEXTURE_ADDRESS_BORDER_POINT, //�e�p
        TEXTURE_TRAIL, //�g���C���p
        DEFALT,           //�����Ȃ��i�ی��j
    };
    Microsoft::WRL::ComPtr<ID3D11SamplerState> GetSamplerState(SAMPLER_TYPE samplerType) { return samplerState_[static_cast<int>(samplerType)].Get(); }
private:
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_[static_cast<int>(SAMPLER_TYPE::DEFALT) + 1];


};

