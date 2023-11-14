#include "ParticleSystemCom.h"

#include "TransformCom.h"
#include "Graphics/Graphics.h"
#include "Graphics/Model/ResourceManager.h"
#include <imgui.h>

#include "Dialog.h"
#include "Logger.h"
#include <fstream>
#include <filesystem>
#include <shlwapi.h>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

CEREAL_CLASS_VERSION(ParticleSystemCom::UV, 1)
CEREAL_CLASS_VERSION(ParticleSystemCom::Shape, 1)
CEREAL_CLASS_VERSION(ParticleSystemCom::VelocityLifeTime, 1)
CEREAL_CLASS_VERSION(ParticleSystemCom::ScaleLifeTime, 1)
CEREAL_CLASS_VERSION(ParticleSystemCom::ColorLifeTime, 1)
CEREAL_CLASS_VERSION(ParticleSystemCom::RotationLifeTime, 1)
CEREAL_CLASS_VERSION(ParticleSystemCom::ParticleConstants, 1)
CEREAL_CLASS_VERSION(ParticleSystemCom::SaveParticleData, 1)

// シリアライズ
namespace DirectX
{
	template<class Archive>
	void serialize(Archive& archive, XMUINT4& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z),
			cereal::make_nvp("w", v.w)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT2& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT3& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT4& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z),
			cereal::make_nvp("w", v.w)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT4X4& m)
	{
		archive(
			cereal::make_nvp("_11", m._11), cereal::make_nvp("_12", m._12), cereal::make_nvp("_13", m._13), cereal::make_nvp("_14", m._14),
			cereal::make_nvp("_21", m._21), cereal::make_nvp("_22", m._22), cereal::make_nvp("_23", m._23), cereal::make_nvp("_24", m._24),
			cereal::make_nvp("_31", m._31), cereal::make_nvp("_32", m._32), cereal::make_nvp("_33", m._33), cereal::make_nvp("_34", m._34),
			cereal::make_nvp("_41", m._41), cereal::make_nvp("_42", m._42), cereal::make_nvp("_43", m._43), cereal::make_nvp("_44", m._44)
		);
	}
}

template<class Archive>
void ParticleSystemCom::UV::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(texSize),
		CEREAL_NVP(texPos)
	);
}

template<class Archive>
void ParticleSystemCom::Shape::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(shapeID),
		CEREAL_NVP(shaDummy),
		CEREAL_NVP(angle),
		CEREAL_NVP(radius),
		CEREAL_NVP(radiusThickness),
		CEREAL_NVP(arc)
	);
}

template<class Archive>
void ParticleSystemCom::VelocityLifeTime::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(linearVelocity),
		CEREAL_NVP(orbitalVelocity),
		CEREAL_NVP(radial),
		CEREAL_NVP(vDummy)
	);
}

template<class Archive>
void ParticleSystemCom::ScaleLifeTime::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(keyTime),
		CEREAL_NVP(sDummy),
		CEREAL_NVP(value),
		CEREAL_NVP(curvePower)
	);
}

template<class Archive>
void ParticleSystemCom::ColorLifeTime::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(keyTime),
		CEREAL_NVP(cDummy),
		CEREAL_NVP(value),
		CEREAL_NVP(curvePower)
	);
}

template<class Archive>
void ParticleSystemCom::RotationLifeTime::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(rotation),
		CEREAL_NVP(rotationRand)
	);
}

template<class Archive>
void ParticleSystemCom::ParticleConstants::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(isWorld),
		CEREAL_NVP(isRoop),
		CEREAL_NVP(rateTime),
		CEREAL_NVP(gravity),
		CEREAL_NVP(color),
		CEREAL_NVP(emitterPosition),
		CEREAL_NVP(startAngle),
		CEREAL_NVP(startAngleRand),
		CEREAL_NVP(startSize),
		CEREAL_NVP(startSpeed),
		CEREAL_NVP(lifeTime),
		CEREAL_NVP(uv),
		CEREAL_NVP(shape),
		CEREAL_NVP(velocityLifeTime),
		CEREAL_NVP(scaleLifeTime),
		CEREAL_NVP(scaleLifeTimeRand),
		CEREAL_NVP(colorLifeTime),
		CEREAL_NVP(rotationLifeTime)
	);
}

template<class Archive>
void ParticleSystemCom::SaveParticleData::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(particleTexture),
		CEREAL_NVP(particleData)
	);
}



UINT align(UINT num, UINT alignment)
{
	return (num + (alignment - 1)) & ~(alignment - 1);
}

ParticleSystemCom::ParticleSystemCom(int particleCount, bool isAutoDeleteFlag)
	: maxParticleCount_(particleCount), isAutoDeleteFlag_(isAutoDeleteFlag)
{
}

// 開始処理
void ParticleSystemCom::Start()
{
	//パーティクル初期化
	Initialize();
}

// 更新処理
void ParticleSystemCom::Update(float elapsedTime)
{
	float worldElapsedTime = Graphics::Instance().GetWorldSpeed() * elapsedTime;

	//リスタート
	if (isRestart_)
	{
		isRestart_ = false;
		lifeLimit_ = 0;
		Restart();
	}

	//削除判定
	if (!particleData_.particleData.isRoop && isAutoDeleteFlag_)
	{
		lifeLimit_ += worldElapsedTime;
	}
	else
	{
		lifeLimit_ = 0;
	}

	std::lock_guard<std::mutex> lock(Graphics::Instance().GetMutex());

	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();


	dc->CSSetUnorderedAccessViews(0, 1, particleUAV_.GetAddressOf(), NULL);

	particleData_.particleData.time += worldElapsedTime;
	particleData_.particleData.elapsedTime = worldElapsedTime;
	dc->UpdateSubresource(constantBuffer_.Get(), 0, 0, &particleData_.particleData, 0, 0);
	dc->CSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());

	dc->UpdateSubresource(gameBuffer_.Get(), 0, 0, &gameData_, 0, 0);
	dc->CSSetConstantBuffers(10, 1, gameBuffer_.GetAddressOf());

	dc->CSSetConstantBuffers(1, 1, sceneBuffer_.GetAddressOf());

	dc->CSSetShader(particleCompute_.Get(), NULL, 0);

	const UINT thread_group_count_x = align(static_cast<UINT>(maxParticleCount_), NUMTHREADS_X) / NUMTHREADS_X;
	dc->Dispatch(thread_group_count_x, 1, 1);

	ID3D11UnorderedAccessView* null_unordered_access_view{};
	dc->CSSetUnorderedAccessViews(0, 1, &null_unordered_access_view, NULL);
}

// GUI描画
void ParticleSystemCom::OnGUI()
{
	//シリアライザ
	if (ImGui::TreeNode("Cereal"))
	{
		if (ImGui::Button("Save"))
		{
			SaveParticle();
		}
		if (ImGui::Button("Load"))
		{
			LoadFromFileParticle();
		}

		Graphics& graphics = Graphics::Instance();
		ID3D11Device* device = graphics.GetDevice();
		Dx11StateLib* dx11State = graphics.GetDx11State().get();

		//テクスチャロード
		char textureFile[256];
		ZeroMemory(textureFile, sizeof(textureFile));
		::strncpy_s(textureFile, sizeof(textureFile), particleData_.particleTexture.c_str(), sizeof(textureFile));
		if (ImGui::Button("..."))
		{
			const char* filter = "Texture Files(*.dds;*.png;*.tga;*.jpg;*.tif)\0*.dds;*.png;*.tga;*.jpg;*.tif;\0All Files(*.*)\0*.*;\0\0";
			DialogResult result = Dialog::OpenFileName(textureFile, sizeof(textureFile), filter, nullptr, graphics.GetHwnd());
			if (result == DialogResult::OK)
			{
				std::filesystem::path path = std::filesystem::current_path();
				path.append("Data");

				char drive[32], dir[256], dirname[256];
				::_splitpath_s(path.string().c_str(), drive, sizeof(drive), dir, sizeof(dir), nullptr, 0, nullptr, 0);
				::_makepath_s(dirname, sizeof(dirname), drive, dir, nullptr, nullptr);
				dirname[strlen(dirname) - 1] = '\0';
				char relativeTextureFile[MAX_PATH];
				PathRelativePathToA(relativeTextureFile, dirname, FILE_ATTRIBUTE_DIRECTORY, textureFile, FILE_ATTRIBUTE_ARCHIVE);

				// 読み込み
				particleData_.particleTexture = relativeTextureFile;
				D3D11_TEXTURE2D_DESC texture2d_desc{};
				if (particleSprite_)
					particleSprite_.Get()->Release();	//解放
				dx11State->load_texture_from_file(device, particleData_.particleTexture.c_str(), particleSprite_.GetAddressOf(), &texture2d_desc);
			}
		}
		ImGui::SameLine();

		::strncpy_s(textureFile, sizeof(textureFile), particleData_.particleTexture.c_str(), sizeof(textureFile));
		if (ImGui::InputText("texture", textureFile, sizeof(textureFile), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			particleData_.particleTexture = textureFile;

			char drive[32], dir[256], fullPath[256];
			::_splitpath_s(textureFile, drive, sizeof(drive), dir, sizeof(dir), nullptr, 0, nullptr, 0);
			::_makepath_s(fullPath, sizeof(fullPath), drive, dir, textureFile, nullptr);
			D3D11_TEXTURE2D_DESC texture2d_desc{};
			if (particleSprite_)
				particleSprite_.Get()->Release();	//解放
			dx11State->load_texture_from_file(device, particleData_.particleTexture.c_str(), particleSprite_.GetAddressOf(), &texture2d_desc);
		}

		ImGui::TreePop();
	}

	//デバッグ用
	ImGui::Checkbox("Restart", &isRestart_);

	//出現設定
	bool roop = particleData_.particleData.isRoop;
	if (ImGui::Checkbox("isRoop", &roop))
	{
		if (roop) particleData_.particleData.isRoop = 1;
		else particleData_.particleData.isRoop = 0;
	}

	bool world = particleData_.particleData.isWorld;
	if (ImGui::Checkbox("isWorld", &world))
	{
		if (world)  particleData_.particleData.isWorld = 1;
		else  particleData_.particleData.isWorld = 0;
	}

	ImGui::DragInt("emmitCount", &particleData_.particleData.rateTime, 1, 0, 100000);


	ImGui::DragFloat4("color", &particleData_.particleData.color.x, 0.01f, 0, 5);
	ImGui::DragFloat3("emitterPosition", &particleData_.particleData.emitterPosition.x, 0.1f);

	ImGui::DragFloat("startSpeed", &particleData_.particleData.startSpeed, 0.01f);

	ImGui::DragFloat("gravity", &particleData_.particleData.gravity, 0.01f);
	ImGui::DragFloat("lifeTime", &particleData_.particleData.lifeTime, 0.01f, 0.01f, 100);

	//サイズ
	{
		bool isRan = false;
		if (particleData_.particleData.startSize.w > 0.01f)isRan = true;
		if (ImGui::Checkbox("ranSize", &isRan))
		{
			if (!isRan)
				particleData_.particleData.startSize.w = 0;
			else
				particleData_.particleData.startSize.w = 1;
		}

		ImGui::DragFloat("startSize", &particleData_.particleData.startSize.x, 0.01f, 0.01f, 100);
		if (isRan)
		{
			ImGui::DragFloat("startSizeRand", &particleData_.particleData.startSize.y, 0.01f, 0.01f, 100);
		}
	}

	//角度
	{
		bool isRan = false;
		if (particleData_.particleData.startAngleRand.w > 0.01f)isRan = true;
		if (ImGui::Checkbox("ranAngle", &isRan))
		{
			if (!isRan)
				particleData_.particleData.startAngleRand.w = 0;
			else
				particleData_.particleData.startAngleRand.w = 1;
		}

		ImGui::DragFloat3("startAngle", &particleData_.particleData.startAngle.x, 0.1f);
		if (isRan)
		{
			ImGui::DragFloat3("startAngleRand", &particleData_.particleData.startAngleRand.x, 0.1f);
		}
	}

	//UV
	if (ImGui::TreeNode("UV"))
	{
		ImGui::DragFloat2("texSize", &particleData_.particleData.uv.texSize.x, 0.01f);
		ImGui::DragFloat2("texPos", &particleData_.particleData.uv.texPos.x, 0.01f);

		ImGui::TreePop();
	}

	//shape
	if (ImGui::TreeNode("Shape"))
	{
		const char* shapeName[] = { "Cone", "Sphere" };
		ImGui::Combo("shapeID", &particleData_.particleData.shape.shapeID, shapeName, _countof(shapeName));

		ImGui::DragFloat("shapeAngle", &particleData_.particleData.shape.angle, 0.1f, 0.01f, 90);
		ImGui::DragFloat("shapeRadius", &particleData_.particleData.shape.radius, 0.01f, 0.01f, 100);
		ImGui::DragFloat("shapeRadiusThickness", &particleData_.particleData.shape.radiusThickness, 0.01f, 0, 1);
		ImGui::DragFloat("arc", &particleData_.particleData.shape.arc, 0.01f, 0, 1);

		ImGui::TreePop();
	}

	//velocity
	if (ImGui::TreeNode("VelocityLifeTime"))
	{
		ImGui::DragFloat3("linearVelo", &particleData_.particleData.velocityLifeTime.linearVelocity.x, 0.1f);
		ImGui::DragFloat3("orbVelo", &particleData_.particleData.velocityLifeTime.orbitalVelocity.x, 0.1f);
		ImGui::DragFloat("radialVelo", &particleData_.particleData.velocityLifeTime.radial, 0.1f);

		ImGui::TreePop();
	}

	//scaleLifeTime
	if (ImGui::TreeNode("ScaleOverLifeTime"))
	{
		//scale
		if (ImGui::TreeNode("ScaleLifeTime"))
		{
			if (ImGui::Button("AddKey"))	//キー追加
			{
				for (int i = 0; i < scaleKeyCount; ++i)
				{
					if (particleData_.particleData.scaleLifeTime[i].keyTime >= 0)continue;

					if (i == 0)
					{
						particleData_.particleData.scaleLifeTime[i].keyTime = 0;
						break;
					}
					else
					{
						//キーの時間か最大で入れれない
						if (particleData_.particleData.scaleLifeTime[i - 1].keyTime > 0.99f)
						{
							break;
						}

						//前のキーの次に打つ
						particleData_.particleData.scaleLifeTime[i].keyTime =
							particleData_.particleData.scaleLifeTime[i - 1].keyTime + 0.01f;
						break;
					}
				}
			}
			for (int i = 0; i < scaleKeyCount; ++i)
			{
				//キーがないならbreak
				if (particleData_.particleData.scaleLifeTime[i].keyTime < 0)break;
				//キー
				if (ImGui::TreeNode(std::to_string(i).c_str()))
				{
					if (ImGui::DragFloat("keyTime", &particleData_.particleData.scaleLifeTime[i].keyTime, 0.01f, 0, 1))
					{
						//下限処理
						if (i != 0)
						{
							if (particleData_.particleData.scaleLifeTime[i].keyTime <= particleData_.particleData.scaleLifeTime[i - 1].keyTime)
								particleData_.particleData.scaleLifeTime[i].keyTime = particleData_.particleData.scaleLifeTime[i - 1].keyTime + 0.01f;
						}
						//上限処理
						if (i != scaleKeyCount - 1)
						{
							if (particleData_.particleData.scaleLifeTime[i + 1].keyTime >= 0)
							{
								if (particleData_.particleData.scaleLifeTime[i].keyTime >= particleData_.particleData.scaleLifeTime[i + 1].keyTime)
									particleData_.particleData.scaleLifeTime[i].keyTime = particleData_.particleData.scaleLifeTime[i + 1].keyTime - 0.01f;
							}
						}
					}

					ImGui::DragFloat("velue", &particleData_.particleData.scaleLifeTime[i].value, 0.1f, 0, 100);
					ImGui::DragFloat("curvePower", &particleData_.particleData.scaleLifeTime[i].curvePower, 0.1f);

					//キー削除
					if (ImGui::Button("Delete"))
					{
						DeleteScaleKey(i, particleData_.particleData.scaleLifeTime);
					}

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}

		//scaleRand
		if (ImGui::TreeNode("ScaleLifeTimeRand"))
		{
			if (ImGui::Button("AddKey"))	//キー追加
			{
				for (int i = 0; i < scaleKeyCount; ++i)
				{
					if (particleData_.particleData.scaleLifeTimeRand[i].keyTime >= 0)continue;

					if (i == 0)
					{
						particleData_.particleData.scaleLifeTimeRand[i].keyTime = 0;
						break;
					}
					else
					{
						//キーの時間か最大で入れれない
						if (particleData_.particleData.scaleLifeTimeRand[i - 1].keyTime > 0.99f)
						{
							break;
						}

						//前のキーの次に打つ
						particleData_.particleData.scaleLifeTimeRand[i].keyTime =
							particleData_.particleData.scaleLifeTimeRand[i - 1].keyTime + 0.01f;
						break;
					}
				}
			}
			for (int i = 0; i < scaleKeyCount; ++i)
			{
				//キーがないならbreak
				if (particleData_.particleData.scaleLifeTimeRand[i].keyTime < 0)break;
				//キー
				if (ImGui::TreeNode(std::to_string(i).c_str()))
				{
					if (ImGui::DragFloat("keyTime", &particleData_.particleData.scaleLifeTimeRand[i].keyTime, 0.01f, 0, 1))
					{
						//下限処理
						if (i != 0)
						{
							if (particleData_.particleData.scaleLifeTimeRand[i].keyTime <= particleData_.particleData.scaleLifeTimeRand[i - 1].keyTime)
								particleData_.particleData.scaleLifeTimeRand[i].keyTime = particleData_.particleData.scaleLifeTimeRand[i - 1].keyTime + 0.01f;
						}
						//上限処理
						if (i != scaleKeyCount - 1)
						{
							if (particleData_.particleData.scaleLifeTimeRand[i + 1].keyTime >= 0)
							{
								if (particleData_.particleData.scaleLifeTimeRand[i].keyTime >= particleData_.particleData.scaleLifeTimeRand[i + 1].keyTime)
									particleData_.particleData.scaleLifeTimeRand[i].keyTime = particleData_.particleData.scaleLifeTimeRand[i + 1].keyTime - 0.01f;
							}
						}
					}

					ImGui::DragFloat("velue", &particleData_.particleData.scaleLifeTimeRand[i].value, 0.1f, 0, 100);
					ImGui::DragFloat("curvePower", &particleData_.particleData.scaleLifeTimeRand[i].curvePower, 0.1f);

					//キー削除
					if (ImGui::Button("Delete"))
					{
						DeleteScaleKey(i, particleData_.particleData.scaleLifeTimeRand);
					}

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	//colorLifeTime
	if (ImGui::TreeNode("ColorOverLifeTime"))
	{
		if (ImGui::Button("AddKey"))	//キー追加
		{
			for (int i = 0; i < colorKeyCount; ++i)
			{
				if (particleData_.particleData.colorLifeTime[i].keyTime >= 0)continue;

				if (i == 0)
				{
					particleData_.particleData.colorLifeTime[i].keyTime = 0;
					break;
				}
				else
				{
					//キーの時間か最大で入れれない
					if (particleData_.particleData.colorLifeTime[i - 1].keyTime > 0.99f)
					{
						break;
					}

					//前のキーの次に打つ
					particleData_.particleData.colorLifeTime[i].keyTime =
						particleData_.particleData.colorLifeTime[i - 1].keyTime + 0.01f;
					break;
				}
			}
		}
		for (int i = 0; i < colorKeyCount; ++i)
		{
			//キーがないならbreak
			if (particleData_.particleData.colorLifeTime[i].keyTime < 0)break;
			//キー
			if (ImGui::TreeNode(std::to_string(i).c_str()))
			{
				if (ImGui::DragFloat("keyTime", &particleData_.particleData.colorLifeTime[i].keyTime, 0.01f, 0, 1))
				{
					//下限処理
					if (i != 0)
					{
						if (particleData_.particleData.colorLifeTime[i].keyTime <= particleData_.particleData.colorLifeTime[i - 1].keyTime)
							particleData_.particleData.colorLifeTime[i].keyTime = particleData_.particleData.colorLifeTime[i - 1].keyTime + 0.01f;
					}
					//上限処理
					if (i != colorKeyCount - 1)
					{
						if (particleData_.particleData.colorLifeTime[i + 1].keyTime >= 0)
						{
							if (particleData_.particleData.colorLifeTime[i].keyTime >= particleData_.particleData.colorLifeTime[i + 1].keyTime)
								particleData_.particleData.colorLifeTime[i].keyTime = particleData_.particleData.colorLifeTime[i + 1].keyTime - 0.01f;
						}
					}
				}

				ImGui::DragFloat4("velue", &particleData_.particleData.colorLifeTime[i].value.x, 0.1f, 0, 100);
				ImGui::DragFloat("curvePower", &particleData_.particleData.colorLifeTime[i].curvePower.x, 0.1f);

				//キー削除
				if (ImGui::Button("Delete"))
				{
					DeleteColorKey(i, particleData_.particleData.colorLifeTime);
				}

				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	//rotation
	if (ImGui::TreeNode("RotationLifeTime"))
	{
		bool isRan = false;
		if (particleData_.particleData.rotationLifeTime.rotationRand.w > 0.01f)isRan = true;
		if (ImGui::Checkbox("ranRota", &isRan))
		{
			if (!isRan)
				particleData_.particleData.rotationLifeTime.rotationRand.w = 0;
			else
				particleData_.particleData.rotationLifeTime.rotationRand.w = 1;
		}

		ImGui::DragFloat3("rotation", &particleData_.particleData.rotationLifeTime.rotation.x);
		if (isRan)
		{
			ImGui::DragFloat3("rotationRand", &particleData_.particleData.rotationLifeTime.rotationRand.x);
		}

		ImGui::TreePop();
	}


	//ゲーム用パーティクル設定
	if (ImGui::TreeNode("GameParticle"))
	{
		bool sweets = false;
		if (gameData_.sweetsData.isEnable == 1)sweets = true;
		if (ImGui::Checkbox("Sweets", &sweets))
		{
			if (sweets)gameData_.sweetsData.isEnable = 1;
			else gameData_.sweetsData.isEnable = 0;
		}

		ImGui::TreePop();
	}
}

void ParticleSystemCom::Restart()
{
	std::lock_guard<std::mutex> lock(Graphics::Instance().GetMutex());

	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	dc->CSSetUnorderedAccessViews(0, 1, particleUAV_.GetAddressOf(), NULL);

	dc->UpdateSubresource(constantBuffer_.Get(), 0, 0, &particleData_.particleData, 0, 0);
	dc->CSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());

	dc->UpdateSubresource(gameBuffer_.Get(), 0, 0, &gameData_, 0, 0);
	dc->CSSetConstantBuffers(10, 1, gameBuffer_.GetAddressOf());

	dc->CSSetShader(particleInitializerCompute_.Get(), NULL, 0);

	const UINT thread_group_count_x = align(static_cast<UINT>(maxParticleCount_), NUMTHREADS_X) / NUMTHREADS_X;
	dc->Dispatch(thread_group_count_x, 1, 1);

	ID3D11UnorderedAccessView* null_unordered_access_view{};
	dc->CSSetUnorderedAccessViews(0, 1, &null_unordered_access_view, NULL);
}

void ParticleSystemCom::Initialize()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	ID3D11Device* device = graphics.GetDevice();

	HRESULT hr{ S_OK };
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Particle) * maxParticleCount_);
	buffer_desc.StructureByteStride = sizeof(Particle);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	hr = device->CreateBuffer(&buffer_desc, NULL, particleBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc;
	shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shader_resource_view_desc.Buffer.ElementOffset = 0;
	shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(maxParticleCount_);
	hr = device->CreateShaderResourceView(particleBuffer_.Get(), &shader_resource_view_desc, particleSRV_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_UNORDERED_ACCESS_VIEW_DESC unordered_access_view_desc;
	unordered_access_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	unordered_access_view_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	unordered_access_view_desc.Buffer.FirstElement = 0;
	unordered_access_view_desc.Buffer.NumElements = static_cast<UINT>(maxParticleCount_);
	unordered_access_view_desc.Buffer.Flags = 0;
	hr = device->CreateUnorderedAccessView(particleBuffer_.Get(), &unordered_access_view_desc, particleUAV_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	Dx11StateLib* dx11State = graphics.GetDx11State().get();

	//画像読み込み
	if (!particleSprite_)
	{
		D3D11_TEXTURE2D_DESC texture2d_desc{};
		if (particleData_.particleTexture.size() > 0)
		{
			dx11State->load_texture_from_file(device, particleData_.particleTexture.c_str(), particleSprite_.GetAddressOf(), &texture2d_desc);
		}
		else
		{
			dx11State->load_texture_from_file(device, "Data/Sprite/color.png", particleSprite_.GetAddressOf(), &texture2d_desc);
		}
	}

	//定数バッファ
	dx11State->createConstantBuffer(device, sizeof(ParticleConstants), constantBuffer_.GetAddressOf());
	dx11State->createConstantBuffer(device, sizeof(GameParticleData), gameBuffer_.GetAddressOf());
	dx11State->createConstantBuffer(device, sizeof(ParticleScene), sceneBuffer_.GetAddressOf());

	//シェーダー初期化
	dx11State->createVsFromCso(device, "Shader\\ParticleVS.cso", particleVertex_.ReleaseAndGetAddressOf(), NULL, NULL, 0);
	dx11State->createPsFromCso(device, "Shader\\ParticlePS.cso", particlePixel_.ReleaseAndGetAddressOf());
	dx11State->createGsFromCso(device, "Shader\\ParticleGS.cso", particleGeometry_.ReleaseAndGetAddressOf());
	dx11State->createCsFromCso(device, "Shader\\ParticleCS.cso", particleCompute_.ReleaseAndGetAddressOf());
	dx11State->createCsFromCso(device, "Shader\\ParticleInitializeCS.cso", particleInitializerCompute_.ReleaseAndGetAddressOf());


	dc->CSSetUnorderedAccessViews(0, 1, particleUAV_.GetAddressOf(), NULL);

	dc->UpdateSubresource(constantBuffer_.Get(), 0, 0, &particleData_.particleData, 0, 0);
	dc->CSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());

	dc->UpdateSubresource(gameBuffer_.Get(), 0, 0, &gameData_, 0, 0);
	dc->CSSetConstantBuffers(10, 1, gameBuffer_.GetAddressOf());

	dc->CSSetShader(particleInitializerCompute_.Get(), NULL, 0);

	const UINT thread_group_count_x = align(static_cast<UINT>(maxParticleCount_), NUMTHREADS_X) / NUMTHREADS_X;
	dc->Dispatch(thread_group_count_x, 1, 1);

	ID3D11UnorderedAccessView* null_unordered_access_view{};
	dc->CSSetUnorderedAccessViews(0, 1, &null_unordered_access_view, NULL);
}

void ParticleSystemCom::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	Dx11StateLib* dx11State = graphics.GetDx11State().get();

	//背面カリングオフに
	dc->RSSetState(dx11State->GetRasterizerState(Dx11StateLib::RASTERIZER_TYPE::FRONTCOUNTER_TRUE_CULLNONE).Get());


	//シェーダーセット
	dc->VSSetShader(particleVertex_.Get(), NULL, 0);
	dc->PSSetShader(particlePixel_.Get(), NULL, 0);
	dc->GSSetShader(particleGeometry_.Get(), NULL, 0);
	dc->GSSetShaderResources(9, 1, particleSRV_.GetAddressOf());

	//サンプラーステート
	ID3D11SamplerState* samplerState = dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_WRAP).Get();
	dc->PSSetSamplers(0, 1, &samplerState);

	//ブレンドステート
	const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	dc->OMSetBlendState(
		dx11State->GetBlendState(Dx11StateLib::BLEND_STATE_TYPE::ALPHA_ATC).Get(),
		blendFactor, 0xFFFFFFFF);

	//画像
	dc->PSSetShaderResources(0, 1, particleSprite_.GetAddressOf());
	
	//コンスタントバッファ
	ParticleScene scene;

	scene.cameraPos = graphics.shaderParameter3D_.viewPosition;

	//モデル情報
	DirectX::XMStoreFloat4x4(&scene.modelMat,
		DirectX::XMLoadFloat4x4(&GetGameObject()->transform_->GetWorldTransform()));

	//ビュープロジェクション
	DirectX::XMStoreFloat4x4(&scene.viweProj,
		DirectX::XMLoadFloat4x4(&graphics.shaderParameter3D_.view)
		* DirectX::XMLoadFloat4x4(&graphics.shaderParameter3D_.projection));

	DirectX::XMStoreFloat4x4(&scene.inverseModelMat,
		DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&GetGameObject()->transform_->GetWorldTransform())));

	//逆ビュープロジェクション
	DirectX::XMStoreFloat4x4(&scene.inverseViweProj,
		DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&graphics.shaderParameter3D_.projection))
		* DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&graphics.shaderParameter3D_.view)));


	DirectX::XMStoreFloat4x4(&scene.view, DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&graphics.shaderParameter3D_.view)));
	DirectX::XMStoreFloat4x4(&scene.gProj,  DirectX::XMLoadFloat4x4(&graphics.shaderParameter3D_.projection));

	scene.lightDir = graphics.shaderParameter3D_.lightDirection;


	DirectX::XMFLOAT3 up = GetGameObject()->transform_->GetWorldPosition();
	DirectX::XMFLOAT4 down = { up.x,up.y-1,up.z,1 };


	scene.downVec = down; 


	dc->UpdateSubresource(sceneBuffer_.Get(), 0, 0, &scene, 0, 0);
	dc->GSSetConstantBuffers(1, 1, sceneBuffer_.GetAddressOf());

	dc->UpdateSubresource(constantBuffer_.Get(), 0, 0, &particleData_.particleData, 0, 0);
	dc->VSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());
	dc->PSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());
	dc->GSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());

	//解放
	dc->IASetInputLayout(NULL);
	dc->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	dc->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	dc->Draw(static_cast<UINT>(maxParticleCount_), 0);

	ID3D11ShaderResourceView* null_shader_resource_view{};
	dc->GSSetShaderResources(9, 1, &null_shader_resource_view);
	dc->VSSetShader(NULL, NULL, 0);
	dc->PSSetShader(NULL, NULL, 0);
	dc->GSSetShader(NULL, NULL, 0);
}

//ファイルネームでロード
ParticleSystemCom::SaveParticleData ParticleSystemCom::Load(const char* filename)
{
	particleData_= *ResourceManagerParticle::Instance().LoadParticleResource(filename);
	return particleData_;
	//return LoadParticle(filename);
}

void ParticleSystemCom::LoadTexture(const char* filename)
{
	particleData_.particleTexture = filename;
}

void ParticleSystemCom::SetSweetsParticle(bool enable)
{
	gameData_.sweetsData.isEnable = enable;
}

bool ParticleSystemCom::DeleteFlag()
{
	//ライフタイムの２倍でパーティクルは全て消えている
	if (particleData_.particleData.lifeTime * 2 < lifeLimit_)
		return true;
	return false;
}

//スケールキーを削除する
void ParticleSystemCom::DeleteScaleKey(int id, ScaleLifeTime(&scaleLife)[scaleKeyCount])
{
	//削除
	scaleLife[id].keyTime = -1;
	scaleLife[id].value = 1;
	scaleLife[id].curvePower = 1;

	//配列ずらす
	for (int i = id; i < scaleKeyCount - 1; ++i)
	{
		//次のキーがない場合
		if (scaleLife[i + 1].keyTime < 0)
		{
			scaleLife[i].keyTime = -1;
			scaleLife[i].value = 1;
			scaleLife[i].curvePower = 1;
		
			break;
		}

		scaleLife[i].keyTime =
			scaleLife[i + 1].keyTime;
		scaleLife[i].value =
			scaleLife[i + 1].value;
		scaleLife[i].curvePower =
			scaleLife[i + 1].curvePower;

		//配列の最後のひとつ前に場合
		if (id == scaleKeyCount - 2)
		{
			scaleLife[i + 1].keyTime = -1;
			scaleLife[i + 1].value = 1;
			scaleLife[i + 1].curvePower = 1;

			break;
		}
	}
}

//カラーキーを削除する
void ParticleSystemCom::DeleteColorKey(int id, ColorLifeTime(&colorLife)[colorKeyCount])
{
	//削除
	colorLife[id].keyTime = -1;
	colorLife[id].value = { 1, 1, 1, 1 };
	colorLife[id].curvePower = { 0, 0, 0, 0 };

	//配列ずらす
	for (int i = id; i < colorKeyCount - 1; ++i)
	{
		//次のキーがない場合
		if (colorLife[i + 1].keyTime < 0)
		{
			colorLife[i].keyTime = -1;
			colorLife[i].value = { 1, 1, 1, 1 };
			colorLife[i].curvePower = { 0, 0, 0, 0 };

			break;
		}

		colorLife[i].keyTime =
			colorLife[i + 1].keyTime;
		colorLife[i].value =
			colorLife[i + 1].value;
		colorLife[i].curvePower =
			colorLife[i + 1].curvePower;

		//配列の最後のひとつ前に場合
		if (id == scaleKeyCount - 2)
		{
			colorLife[i + 1].keyTime = -1;
			colorLife[i + 1].value = { 1, 1, 1, 1 };
			colorLife[i + 1].curvePower = { 0, 0, 0, 0 };

			break;
		}
	}
}

//シリアルでパーティクルを保存
void ParticleSystemCom::SaveParticle()
{
	static const char* filter = "Model Files(*.ipff)\0*.ipff;\0All Files(*.*)\0*.*;\0\0";

	char filename[256] = { 0 };
	DialogResult result = Dialog::SaveFileName(filename, sizeof(filename), filter, nullptr, "ipff", Graphics::Instance().GetHwnd());
	if (result == DialogResult::OK)
	{
		std::ofstream ostream(filename, std::ios::binary);
		if (ostream.is_open())
		{
			cereal::BinaryOutputArchive archive(ostream);

			try
			{
				archive(
					CEREAL_NVP(particleData_)
				);
			}
			catch (...)
			{
				LOG("particle deserialize failed.\n%s\n", filename);
				return;
			}
		}
	}
}

void ParticleSystemCom::LoadFromFileParticle()
{
	static const char* filter = "Model Files(*.ipff)\0*.ipff;\0All Files(*.*)\0*.*;\0\0";

	char filename[256] = { 0 };
	DialogResult result = Dialog::OpenFileName(filename, sizeof(filename), filter, nullptr, Graphics::Instance().GetHwnd());
	if (result == DialogResult::OK)
	{
		LoadParticle(filename);
	}
}

ParticleSystemCom::SaveParticleData ParticleSystemCom::LoadParticle(const char* filename)
{
	std::ifstream istream(filename, std::ios::binary);
	if (istream.is_open())
	{
		cereal::BinaryInputArchive archive(istream);

		try
		{
			archive(
				CEREAL_NVP(particleData_)
			);
		}
		catch (...)
		{
			LOG("particle deserialize failed.\n%s\n", filename);
			return {};
		}
	}

	Graphics& graphics = Graphics::Instance();
	ID3D11Device* device = graphics.GetDevice();
	Dx11StateLib* dx11State = graphics.GetDx11State().get();

	//画像読み込み
	if (particleSprite_)
		particleSprite_.Get()->Release();	//解放
	D3D11_TEXTURE2D_DESC texture2d_desc{};
	if (particleData_.particleTexture.size() > 0)
	{
		dx11State->load_texture_from_file(device, particleData_.particleTexture.c_str(), particleSprite_.GetAddressOf(), &texture2d_desc);
	}
	else
	{
		dx11State->load_texture_from_file(device, "Data/Sprite/color.png", particleSprite_.GetAddressOf(), &texture2d_desc);
	}

	return particleData_;

}
