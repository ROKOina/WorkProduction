#pragma once

#include "System\Component.h"

#include <d3d11.h>
#include <wrl.h>

#define NUMTHREADS_X 16

struct Particle
{
	DirectX::XMFLOAT4 color{ 1, 1, 1, 1 };
	DirectX::XMFLOAT3 position{ 0, 0, 0 };
	DirectX::XMFLOAT3 angle{ 0, 0, 0 };
	DirectX::XMFLOAT3 velocity{ 0, 0, 0 };
	float age{};
	float lifeTime{ 1 };
	int state{};
};
struct ParticleScene
{
	DirectX::XMFLOAT4X4 viweProj;
	DirectX::XMFLOAT4 lightDir;
	DirectX::XMFLOAT4 cameraPos;
};

class ParticleSystemCom : public Component
{
	//コンポーネントオーバーライド
public:
	ParticleSystemCom(int particleCount) 
		: maxParticleCount_(particleCount) {}
	~ParticleSystemCom() {}

	// 名前取得
	const char* GetName() const override { return "ParticleSystem"; }

	// 開始処理
	void Start() override;

	// 更新処理
	void Update(float elapsedTime) override;

	// GUI描画
	void OnGUI() override;


	//ParticleSystemクラス
public:

	void Initialize();

	void Render();

private:
	const size_t maxParticleCount_;

	struct ParticleConstants
	{
		DirectX::XMFLOAT3 emitterPosition{};
		float particleSize{ 0.02f * 30 };
		float time{};
		float elapsedTime{};

		DirectX::XMFLOAT2 texSize{1, 1};
		DirectX::XMFLOAT2 texPos{0, 0};

		DirectX::XMFLOAT2 dummy;
	};
	ParticleConstants particleData_;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleSprite_;

	Microsoft::WRL::ComPtr<ID3D11Buffer> particleBuffer_;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleUAV_;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleSRV_;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> particleVertex_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> particlePixel_;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> particleGeometry_;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleCompute_;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleInitializerCompute_;

	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> sceneBuffer_;

};