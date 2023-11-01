#pragma once

#include "System\Component.h"

#include <d3d11.h>
#include <wrl.h>

#define NUMTHREADS_X 16

//パーティクル一つの情報
struct Particle
{
	DirectX::XMFLOAT4 color{ 1, 1, 1, 1 };
	DirectX::XMFLOAT3 position{ 0, 0, 0 };
	DirectX::XMFLOAT3 emitPosition{0, 0, 0};	//出現位置を保存

	DirectX::XMFLOAT3 angle{ 0, 0, 0 };
	DirectX::XMFLOAT3 startAngle{ 0, 0, 0 };
	DirectX::XMFLOAT3 startAngleRand{0, 0, 0};

	DirectX::XMFLOAT3 size{ 1,1,1 };
	DirectX::XMFLOAT3 startSize{ 1,1,1 };
	DirectX::XMFLOAT3 randSizeCurve{0, 0, 0};	//乱数サイズカーブの補間値

	DirectX::XMFLOAT3 velocity{ 0, 0, 0 };
	float age{};
	float lifeTime{ 1 };
	int state{};
	//出現しているか
	int emmitterFlag = 0;

	DirectX::XMFLOAT4X4 saveModel;
};
struct ParticleScene
{
	DirectX::XMFLOAT4X4 modelMat;
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

	//パーティクルロード
	void Load(const char* filename);
	//テクスチャロード
	void LoadTexture(const char* filename);

public:

	//UV
	struct UV
	{
		DirectX::XMFLOAT2 texSize{1, 1};
		DirectX::XMFLOAT2 texPos{0, 0};

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	//出現形
	struct Shape
	{
		int shapeID = 0;	//0:cone 1:sphere
		DirectX::XMFLOAT3 shaDummy;

		float angle = 45;	//出現する角度
		float radius = 1;	//出現する半径
		float radiusThickness = 1;	//出現する半径のボリューム
		float arc = 1;	//形状を作る円弧

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	//動き
	struct VelocityLifeTime
	{
		DirectX::XMFLOAT4 linearVelocity{0, 0, 0, 0};
		DirectX::XMFLOAT4 orbitalVelocity{0, 0, 0, 0};

		float radial = 0;
		DirectX::XMFLOAT3 vDummy{0, 0, 0};

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	//消えるまでの時間でサイズを変更できる
#define scaleKeyCount 5
	struct ScaleLifeTime
	{
		float keyTime = -1;

		DirectX::XMFLOAT3 sDummy;

		DirectX::XMFLOAT4 value{1, 1, 1, 1};
		DirectX::XMFLOAT4 curvePower{0, 0, 0, 0};	//valueに行くまでのカーブ

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct RotationLifeTime
	{
		DirectX::XMFLOAT4 rotation{0, 0, 0, 0};
		//最後の値（ｗ）を1にすればランダムになる
		DirectX::XMFLOAT4 rotationRand{0, 0, 0, 0};

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	//パーティクルすべての情報
	struct ParticleConstants
	{
		//ワールドに合わせて動く
		int isWorld = 1;
		int isRoop = 1;	//0:off  1:on

		int rateTime = 100;	//一秒間に出現する数
		float gravity = 0;


		DirectX::XMFLOAT4 emitterPosition{};

		DirectX::XMFLOAT4 startAngle{0, 0, 0, 0};
		DirectX::XMFLOAT4 startAngleRand{0, 0, 0, 0};	//最後の値（ｗ）を1にすればランダムになる

		DirectX::XMFLOAT4 startSize{ 0.05f, 0.05f, 0, 0 };	
		DirectX::XMFLOAT4 startSizeRand{ 0, 0, 0, 0 };	//最後の値（ｗ）を1にすればランダムになる

		float startSpeed{ 1 };

		float time{ 0 };
		float elapsedTime{ 0 };
		float lifeTime = 3;


		UV uv;

		Shape shape;

		VelocityLifeTime velocityLifeTime;

		ScaleLifeTime scaleLifeTime[scaleKeyCount];	//並び替えてkey順にする
		ScaleLifeTime scaleLifeTimeRand[scaleKeyCount];

		RotationLifeTime rotationLifeTime;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct SaveParticleData
	{
		std::string particleTexture;
		ParticleConstants particleData;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

private:
	void DeleteScaleKey(int id, ScaleLifeTime(&scaleLife)[scaleKeyCount]);

	//シリアルでパーティクルを保存
	void SaveParticle();
	void LoadFromFileParticle();
	void LoadParticle(const char* filename);

private:
	const size_t maxParticleCount_;

	SaveParticleData particleData_;


	//画像
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleSprite_;

	//構造体バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleBuffer_;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleUAV_;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleSRV_;

	//シェーダー
	Microsoft::WRL::ComPtr<ID3D11VertexShader> particleVertex_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> particlePixel_;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> particleGeometry_;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleCompute_;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleInitializerCompute_;

	//コンスタントバッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> sceneBuffer_;


	//debug関係
	bool isRestart_ = false;
};