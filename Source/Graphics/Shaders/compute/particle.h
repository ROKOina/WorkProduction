#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>
#include "Graphics\Graphics.h"
#include "Misc.h"


class Particle
{
public:
    Particle(DirectX::XMFLOAT4 initialPosition);
    ~Particle() {
    }

    void integrate(float deltaTime, DirectX::XMFLOAT4 eyePosition,
        DirectX::XMFLOAT4X4	view, DirectX::XMFLOAT4X4	projection);
    void Render(const RenderContext& rc);

public:
    struct particle
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 velocity;
    };
    std::vector<particle> particles_;

    struct particle_constants
    {
        DirectX::XMFLOAT4 currentEyePosition;
        DirectX::XMFLOAT4 previousEyePosition;

        // Radius of outermost orbit 
        float outermostRadius;
        // Height of area
        float areaHeight;

        float particleSize;
        uint32_t particleCount;

        DirectX::XMFLOAT4X4	view;
        DirectX::XMFLOAT4X4	projection;
        float sceneTimer;
        float deltaTime;

        DirectX::XMFLOAT2	dummy;

    };
    particle_constants particleData_;

private:
    size_t particleCount_{ 0 };
    Microsoft::WRL::ComPtr<ID3D11Buffer> particleBuffer_;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleBufferUav_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleBufferSrv_;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader_;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;

//private:
//    //パーティクル用頂点レイアウト
//    struct VERTEX_3D_PARTICLE
//    {
//        DirectX::XMFLOAT3 Position;
//        DirectX::XMFLOAT2 TexCoord;
//    };
//
//    //パーティクル資料
//    struct ParticleCompute
//    {
//        DirectX::XMFLOAT3 pos;
//        DirectX::XMFLOAT3 vel;
//        float life;
//    };
//
//    //コンピュートシェーダー
//    Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeShader;
//    // パーティクル
//    ParticleCompute* mParticle;
//    // バッファ
//    Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
//    Microsoft::WRL::ComPtr<ID3D11Buffer> mpParticleBuffer;
//    Microsoft::WRL::ComPtr<ID3D11Buffer> mpResultBuffer;
//    Microsoft::WRL::ComPtr<ID3D11Buffer> mpPositionBuffer;
//
//    // SRV
//    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mpParticleSRV;
//    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mpPositionSRV;
//    // UAV
//    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> mpResultUAV;
//
//    //コンスタントバッファ
//    struct WVP
//    {
//        DirectX::XMFLOAT4X4 world;
//        DirectX::XMFLOAT4X4 view;
//        DirectX::XMFLOAT4X4 projection;
//    };
//    Microsoft::WRL::ComPtr<ID3D11Buffer> wvpBuffer;
//
//    //パーティクル
//    //サイズ
//    float pSize = 2;
//    //最大数
//    int pAmount = 50;

};