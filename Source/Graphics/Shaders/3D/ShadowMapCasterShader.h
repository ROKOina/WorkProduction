#pragma once

#include <memory>
#include <wrl.h>
#include "../Shader.h"

class ShadowMapCasterShader : public Shader
{
public:
	ShadowMapCasterShader(ID3D11Device* device);
	~ShadowMapCasterShader() override {}

	void Begin(ID3D11DeviceContext* dc, const ShaderParameter3D& rc)override;
	void Draw(ID3D11DeviceContext* dc, const Model* model)override;
	void End(ID3D11DeviceContext* context)override;

private:
	static const int MaxBones = 128;

	struct CbScene
	{
		DirectX::XMFLOAT4X4		viewProjection;
	};

	struct CbMesh
	{
		DirectX::XMFLOAT4X4	boneTransforms[MaxBones];
	};


	Microsoft::WRL::ComPtr<ID3D11Buffer>			sceneConstantBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			meshConstantBuffer_;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>		vertexShader_;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>		inputLayout_;
};
