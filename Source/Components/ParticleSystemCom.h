#pragma once

#include "System\Component.h"

#include <d3d11.h>
#include <wrl.h>

#define NUMTHREADS_X 16

//�p�[�e�B�N����̏��
struct Particle
{
	DirectX::XMFLOAT4 color{ 1, 1, 1, 1 };
	DirectX::XMFLOAT3 position{ 0, 0, 0 };
	DirectX::XMFLOAT3 emitPosition{0, 0, 0};	//�o���ʒu��ۑ�

	DirectX::XMFLOAT3 angle{ 0,0,0 };
	DirectX::XMFLOAT3 startAngle{ 0,0,0 };
	DirectX::XMFLOAT3 startAngleRand{ 0,0,0 };

	float size{ 1 };
	float startSize{ 1 };
	float randSizeCurve{0};	//�����T�C�Y�J�[�u�̕�Ԓl

	DirectX::XMFLOAT3 velocity{ 0, 0, 0 };
	float age{};
	float lifeTime{ 1 };
	int state{};
	//�o�����Ă��邩
	int emmitterFlag = 0;

	DirectX::XMFLOAT2 uvSize;
	DirectX::XMFLOAT2 uvPos;

	DirectX::XMFLOAT4 downPP;

	DirectX::XMFLOAT4X4 saveModel;
};
struct ParticleScene
{
	DirectX::XMFLOAT4X4 modelMat;
	DirectX::XMFLOAT4X4 viweProj;

	DirectX::XMFLOAT4X4 inverseModelMat;
	DirectX::XMFLOAT4X4 inverseViweProj;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 gProj;

	DirectX::XMFLOAT4 lightDir;
	DirectX::XMFLOAT4 cameraPos;
	DirectX::XMFLOAT4 downVec;
};

class ParticleSystemCom : public Component
{
	//�R���|�[�l���g�I�[�o�[���C�h
public:
	ParticleSystemCom(int particleCount, bool isAutoDeleteFlag = true);	
	~ParticleSystemCom() {}

	// ���O�擾
	const char* GetName() const override { return "ParticleSystem"; }

	// �J�n����
	void Start() override;

	// �X�V����
	void Update(float elapsedTime) override;

	// GUI�`��
	void OnGUI() override;


	//ParticleSystem�N���X
public:

	void Initialize();

	void Render();

	//�p�[�e�B�N�����[�h
	void Load(const char* filename);
	//�e�N�X�`�����[�h
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

	//�o���`
	struct Shape
	{
		int shapeID = 0;	//0:cone 1:sphere
		DirectX::XMFLOAT3 shaDummy;

		float angle = 45;	//�o������p�x
		float radius = 1;	//�o�����锼�a
		float radiusThickness = 1;	//�o�����锼�a�̃{�����[��
		float arc = 1;	//�`������~��

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	//����
	struct VelocityLifeTime
	{
		DirectX::XMFLOAT4 linearVelocity{0, 0, 0, 0};
		DirectX::XMFLOAT4 orbitalVelocity{0, 0, 0, 0};

		float radial = 0;
		DirectX::XMFLOAT3 vDummy{0, 0, 0};

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	//������܂ł̎��ԂŃT�C�Y��ύX�ł���
#define scaleKeyCount 5
	struct ScaleLifeTime
	{
		float keyTime = -1;

		float value{1};
		float curvePower{1};	//value�ɍs���܂ł̃J�[�u

		float sDummy;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	//������܂ł̎��ԂŐF��ύX�ł���
#define colorKeyCount 5
	struct ColorLifeTime
	{
		float keyTime = -1;
		DirectX::XMFLOAT3 cDummy;

		DirectX::XMFLOAT4 value{1, 1, 1, 1};
		DirectX::XMFLOAT4 curvePower{1, 1, 1, 1};	//value�ɍs���܂ł̃J�[�u

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct RotationLifeTime
	{
		DirectX::XMFLOAT4 rotation{0, 0, 0, 0};
		//�Ō�̒l�i���j��1�ɂ���΃����_���ɂȂ�
		DirectX::XMFLOAT4 rotationRand{0, 0, 0, 0};

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	//�p�[�e�B�N�����ׂĂ̏��
	struct ParticleConstants
	{
		//���[���h�ɍ��킹�ē���
		int isWorld = 1;
		int isRoop = 1;	//0:off  1:on

		int rateTime = 100;	//��b�Ԃɏo�����鐔
		float gravity = 0;

		DirectX::XMFLOAT4 color{1, 1, 1, 1};
		DirectX::XMFLOAT4 emitterPosition{};

		DirectX::XMFLOAT4 startAngle{0, 0, 0, 0};
		DirectX::XMFLOAT4 startAngleRand{0, 0, 0, 0};	//�Ō�̒l�i���j��1�ɂ���΃����_���ɂȂ�

		//�Ō�̒l�i���j��1�ɂ���΃����_���ɂȂ�AY�������_���l��
		DirectX::XMFLOAT4 startSize{ 0.5f, 0, 0, 0};

		float startSpeed{ 1 };

		float time{ 0 };
		float elapsedTime{ 0 };
		float lifeTime = 3;


		UV uv;

		Shape shape;

		VelocityLifeTime velocityLifeTime;

		ScaleLifeTime scaleLifeTime[scaleKeyCount];	//���ёւ���key���ɂ���
		ScaleLifeTime scaleLifeTimeRand[scaleKeyCount];

		ColorLifeTime colorLifeTime[colorKeyCount];

		RotationLifeTime rotationLifeTime;


		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	//�V���A��������
	struct SaveParticleData
	{
		std::string particleTexture;
		ParticleConstants particleData;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

public:
	//���[�v
	void SetRoop(bool roop) { particleData_.particleData.isRoop = roop; }
	bool GetRoop() { return particleData_.particleData.isRoop; }

	SaveParticleData& GetSaveParticleData() { return particleData_; }

	//���َq�p�[�e�B�N���ݒ�ɂ���
	void SetSweetsParticle(bool enable);

	//�폜����
	bool DeleteFlag();

public:
	//���َq�������p�[�e�B�N���p
	struct SweetsParticleData
	{
		//uv�X�N���[���ňꖇ�̉摜���炨�َq���o��
		int isEnable = false;

		int sweetsCount = 23;
		DirectX::XMFLOAT2 uvCount{8, 8};
	};

	//���̃Q�[���p�p�[�e�B�N���f�[�^(�V���A�����Ȃ�)
	struct GameParticleData
	{
		SweetsParticleData sweetsData;
	};

private:
	void DeleteScaleKey(int id, ScaleLifeTime(&scaleLife)[scaleKeyCount]);
	void DeleteColorKey(int id, ColorLifeTime(&colorLife)[colorKeyCount]);

	//�V���A���Ńp�[�e�B�N����ۑ�
	void SaveParticle();
	void LoadFromFileParticle();
	void LoadParticle(const char* filename);

private:
	//���[�v����Ȃ��ꍇ����ɍ폜����邩
	bool isAutoDeleteFlag_ = true;

	const size_t maxParticleCount_;

	SaveParticleData particleData_;
	GameParticleData gameData_;

	float lifeLimit_ = 0;	//�Q�[���I�u�W�F�N�g�������悤

	//�摜
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleSprite_;

	//�\���̃o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleBuffer_;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleUAV_;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleSRV_;

	//�V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11VertexShader> particleVertex_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> particlePixel_;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> particleGeometry_;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleCompute_;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleInitializerCompute_;

	//�R���X�^���g�o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> gameBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> sceneBuffer_;

	////�p�[�e�B�N���������X���b�h��
	//std::future<void> future;			

	//debug�֌W
	bool isRestart_ = false;
};