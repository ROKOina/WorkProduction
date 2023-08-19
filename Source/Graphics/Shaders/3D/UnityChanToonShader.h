#pragma once

#include <memory>
#include <wrl.h>
#include "../Shader.h"

class UnityChanToonShader :public Shader
{
public:
    UnityChanToonShader(ID3D11Device* device);
    ~UnityChanToonShader() override {}

    void Begin(ID3D11DeviceContext* dc, const ShaderParameter3D& rc)override;
    void Draw(ID3D11DeviceContext* dc, const Model* model)override;
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
        DirectX::XMFLOAT4X4	lightViewProjection;	//	���C�g�r���[�v���W�F�N�V�����s��
        DirectX::XMFLOAT3	shadowColor;			//	�e�̐F
        float				shadowBias;				//	�[�x��r�p�̃I�t�Z�b�g�l
    };

    Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> meshConstantBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> subsetConstantBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> shadowMapConstantBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> unityChanToonConstantBuffer_;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Emissive;


    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
};