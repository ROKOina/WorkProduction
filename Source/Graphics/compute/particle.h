#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>
#include "Graphics\Graphics.h"
#include "Misc.h"


class Particle
{
public:
    Particle(DirectX::XMFLOAT4 initial_position);
    ~Particle() {
    }

    void integrate(float delta_time, DirectX::XMFLOAT4 eye_position,
        DirectX::XMFLOAT4X4	view, DirectX::XMFLOAT4X4	projection);
    void Render(const RenderContext& rc);

public:
    struct particle
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 velocity;
    };
    std::vector<particle> particles;

    struct particle_constants
    {
        DirectX::XMFLOAT4 current_eye_position;
        DirectX::XMFLOAT4 previous_eye_position;

        // Radius of outermost orbit 
        float outermost_radius;
        // Height of area
        float area_height;

        float particle_size;
        uint32_t particle_count;

        DirectX::XMFLOAT4X4	view;
        DirectX::XMFLOAT4X4	projection;
        float sceneTimer;
        float deltaTime;

        DirectX::XMFLOAT2	dummy;

    };
    particle_constants particle_data;

private:
    size_t particle_count{ 0 };
    Microsoft::WRL::ComPtr<ID3D11Buffer> particle_buffer;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particle_buffer_uav;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particle_buffer_srv;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometry_shader;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> compute_shader;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;

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