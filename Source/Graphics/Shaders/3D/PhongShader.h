#pragma once

#include <memory>
#include <wrl.h>
#include "../Shader.h"

class PhongShader :public Shader
{
public:
    PhongShader(ID3D11Device* device);
    ~PhongShader() override {}

    void Begin(ID3D11DeviceContext* dc, const ShaderParameter3D& rc)override;
    void Draw(ID3D11DeviceContext* dc,  const Model* model)override;
    void End(ID3D11DeviceContext* context)override;

private:

    static const int MaxBones = 128;

    struct CbScene
    {
        DirectX::XMFLOAT4		viewPosition;
        DirectX::XMFLOAT4X4     viewProjection;
        DirectX::XMFLOAT4		ambientLightColor;	
        DirectX::XMFLOAT4		lightDirection;
        DirectX::XMFLOAT4		lightColor;
    };

    struct CbMesh
    {
        DirectX::XMFLOAT4X4 boneTransforms[MaxBones];
    };

    struct CbSubset
    {
        DirectX::XMFLOAT4 materialColor;
    };

    struct CbShadowMap
    {
        DirectX::XMFLOAT4X4	lightViewProjection;	//	ライトビュープロジェクション行列
        DirectX::XMFLOAT3	shadowColor;			//	影の色
        float				shadowBias;				//	深度比較用のオフセット値
    };

    Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> meshConstantBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> subsetConstantBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> shadowMapConstantBuffer_;


    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
};