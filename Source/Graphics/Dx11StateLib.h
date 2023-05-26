#pragma once

#include <wrl.h>
#include <d3d11.h>

//dx11描画周りのステートを初期化しておく
class Dx11StateLib
{
public:
    Dx11StateLib() {}
    ~Dx11StateLib() {}

    //Init
    void Dx11StateInit(ID3D11Device* device);


    //shaderセット
public:
    HRESULT createVsFromCso(ID3D11Device* device, const char* cso_name, ID3D11VertexShader** vertex_shader,
        ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements);

    HRESULT createPsFromCso(ID3D11Device* device, const char* cso_name, ID3D11PixelShader** pixel_shader);

    //コンスタントバッファ生成
public:
    HRESULT createConstantBuffer(ID3D11Device* device, UINT ByteWidth, ID3D11Buffer** buffer);

    //ブレンドステート
public:
    enum class BLEND_STATE_TYPE {
        ALPHA,          //透過
        ADDITION,       //加算
        ADDITION_ALPHA, //加算（透過）
        SUBTRACTION,    //減算
        SCREEN,         //スクリーン
        PARTICLE,       //パーティクル
        DEFALT,           //何もなし（保険）
    };
    Microsoft::WRL::ComPtr<ID3D11BlendState> GetBlendState(BLEND_STATE_TYPE blendType) { return blendState[static_cast<int>(blendType)].Get(); }
private:
    Microsoft::WRL::ComPtr<ID3D11BlendState> blendState[static_cast<int>(BLEND_STATE_TYPE::DEFALT) + 1];

    //深度ステンシルステート
public:
    enum class DEPTHSTENCIL_STATE_TYPE {
        DEPTH_ON_3D,   //深度テストON 3Dに設定されていた
        DEPTH_ON_2D,   //深度テストON 2Dに設定されていた
        DEPTH_OFF,   //深度テストOFF
        PARTICLE,  //パーティクル
        DEFALT,           //何もなし（保険）
    };
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> GetDepthStencilState(DEPTHSTENCIL_STATE_TYPE depthStencilType) { return depthStencilState[static_cast<int>(depthStencilType)].Get(); }
private:
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::DEFALT) + 1];

    //ラスタライザーステート
public:
    enum class RASTERIZER_TYPE {
        FRONTCOUNTER_FALSE_CULLBACK,  //FrontCounterClockwiseとカルモード設定
        FRONTCOUNTER_FALSE_CULLNONE,  
        FRONTCOUNTER_TRUE_CULLNONE,
        PARTICLE,   //パーティクル
        DEFALT,           //何もなし（保険）
    };
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> GetRasterizerState(RASTERIZER_TYPE rasterizerType) { return rasterizerState[static_cast<int>(rasterizerType)].Get(); }
private:
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState[static_cast<int>(RASTERIZER_TYPE::DEFALT) + 1];

    //サンプラーステート
public:
    enum class SAMPLER_TYPE {
        TEXTURE_ADDRESS_WRAP,
        DEFALT,           //何もなし（保険）
    };
    Microsoft::WRL::ComPtr<ID3D11SamplerState> GetSamplerState(SAMPLER_TYPE samplerType) { return samplerState[static_cast<int>(samplerType)].Get(); }
private:
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState[static_cast<int>(SAMPLER_TYPE::DEFALT) + 1];


};

