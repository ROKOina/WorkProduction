#pragma once

#include <memory>
#include <wrl.h>
#include "../Shader.h"

class LambertShader : public Shader
{
public:
	LambertShader(ID3D11Device* device);
	~LambertShader() override {}

	void Begin(ID3D11DeviceContext* dc, const ShaderParameter3D& rc) override;
	void Draw(ID3D11DeviceContext* dc, const Model* model) override;
	void End(ID3D11DeviceContext* dc) override;

private:
	static const int MaxBones = 128;

	struct CbScene
	{
		DirectX::XMFLOAT4X4	viewProjection;
		DirectX::XMFLOAT4	lightDirection;
	};
	CbScene cbScene_;

	struct CbMesh
	{
		DirectX::XMFLOAT4X4	boneTransforms[MaxBones];
	};

	struct CbSubset
	{
		DirectX::XMFLOAT4	materialColor;
	};


	Microsoft::WRL::ComPtr<ID3D11Buffer>			sceneConstantBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			meshConstantBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			subsetConstantBuffer_;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>		vertexShader_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		pixelShader_;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>		inputLayout_;
};
