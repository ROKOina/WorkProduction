#include <stdlib.h>
#include <fstream>
#include <functional>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <WICTextureLoader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Misc.h"
#include "Logger.h"
#include "ModelResource.h"

// CEREALバージョン定義
CEREAL_CLASS_VERSION(UnityChanToonStruct, 1)

CEREAL_CLASS_VERSION(ModelResource::Node, 1)
CEREAL_CLASS_VERSION(ModelResource::Material, 1)
CEREAL_CLASS_VERSION(ModelResource::Subset, 1)
CEREAL_CLASS_VERSION(ModelResource::ShapeData, 1)
CEREAL_CLASS_VERSION(ModelResource::Vertex, 1)
CEREAL_CLASS_VERSION(ModelResource::Mesh, 1)
CEREAL_CLASS_VERSION(ModelResource::NodeKeyData, 1)
CEREAL_CLASS_VERSION(ModelResource::Keyframe, 1)
CEREAL_CLASS_VERSION(ModelResource::AnimationEvent, 1)
CEREAL_CLASS_VERSION(ModelResource::RootPosition, 1)
CEREAL_CLASS_VERSION(ModelResource::Animation, 1)
CEREAL_CLASS_VERSION(ModelResource, 1)

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
void UnityChanToonStruct::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(_Is_LightColor_Base),
		CEREAL_NVP(_Tweak_SystemShadowsLevel),
		CEREAL_NVP(_BaseColor_Step),
		CEREAL_NVP(_BaseShade_Feather),
		CEREAL_NVP(_BaseColor),
		CEREAL_NVP(_1st_ShadeColor),
		CEREAL_NVP(_Set_HighColorMask),
		CEREAL_NVP(_HighColor_Power),
		CEREAL_NVP(dummy),
		CEREAL_NVP(_RimLight_Power),
		CEREAL_NVP(_RimLight),
		CEREAL_NVP(_RimLightColor),
		CEREAL_NVP(_Emissive_Color)
	);
}

template<class Archive>
void ModelResource::Node::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(id),
		CEREAL_NVP(name),
		CEREAL_NVP(path),
		CEREAL_NVP(parentIndex),
		CEREAL_NVP(scale),
		CEREAL_NVP(rotate),
		CEREAL_NVP(translate)
	);
}

template<class Archive>
void ModelResource::Material::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(name),
		CEREAL_NVP(textureFilename),
		CEREAL_NVP(normalFilename),
		CEREAL_NVP(color),
		CEREAL_NVP(toonStruct)
	);
}

template<class Archive>
void ModelResource::Subset::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(startIndex),
		CEREAL_NVP(indexCount),
		CEREAL_NVP(materialIndex)
	);
}

template<class Archive>
void ModelResource::Vertex::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(position),
		CEREAL_NVP(normal),
		CEREAL_NVP(tangent),
		CEREAL_NVP(texcoord),
		CEREAL_NVP(color),
		CEREAL_NVP(boneWeight),
		CEREAL_NVP(boneIndex)
	);
}

template<class Archive>
void ModelResource::ShapeData::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(name),
		CEREAL_NVP(rate),
		CEREAL_NVP(shapeVertex)
	);
}

template<class Archive>
void ModelResource::Mesh::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(vertices),
		CEREAL_NVP(indices),
		CEREAL_NVP(subsets),
		CEREAL_NVP(nodeIndex),
		CEREAL_NVP(nodeIndices),
		CEREAL_NVP(offsetTransforms),
		CEREAL_NVP(boundsMin),
		CEREAL_NVP(boundsMax),
		CEREAL_NVP(shapeData)
	);
}

template<class Archive>
void ModelResource::NodeKeyData::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(scale),
		CEREAL_NVP(rotate),
		CEREAL_NVP(translate)
	);
}

template<class Archive>
void ModelResource::Keyframe::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(seconds),
		CEREAL_NVP(nodeKeys)
	);
}

template<class Archive>
void ModelResource::AnimationEvent::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(name),
		CEREAL_NVP(startFrame),
		CEREAL_NVP(endFrame),
		CEREAL_NVP(eventNode)
	);
}

template<class Archive>
void ModelResource::RootPosition::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(frame),
		CEREAL_NVP(pos)
	);
}

template<class Archive>
void ModelResource::Animation::serialize(Archive& archive, int version)
{
	archive(
		CEREAL_NVP(name),
		CEREAL_NVP(secondsLength),
		CEREAL_NVP(keyframes),
		CEREAL_NVP(animationEvents),
		CEREAL_NVP(rootPosition)
	);
}

//シェイプ0クリア
void ModelResource::ShapeReset()
{
	for (auto& shape : meshes_[shapeIndex_].shapeData)
	{
		shape.rate = 0;
	}
}

// 読み込み
void ModelResource::Load(ID3D11Device* device, const char* filename)
{
	// ディレクトリパス取得
	char drive[32], dir[256], dirname[256];
	::_splitpath_s(filename, drive, sizeof(drive), dir, sizeof(dir), nullptr, 0, nullptr, 0);
	::_makepath_s(dirname, sizeof(dirname), drive, dir, nullptr, nullptr);

	// デシリアライズ
	Deserialize(filename);

	// モデル構築
	BuildModel(device, dirname);
}

// モデル構築
void ModelResource::BuildModel(ID3D11Device* device, const char* dirname)
{
	for (Material& material : materials_)
	{
		// 相対パスの解決
		char filename[256];
		::_makepath_s(filename, 256, nullptr, dirname, material.textureFilename.c_str(), nullptr);

		// ディフューズマップテクスチャ読み込み
		HRESULT hr = LoadTexture(device, filename, nullptr, true, material.diffuseMap.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		::_makepath_s(filename, 256, nullptr, dirname, material.normalFilename.c_str(), nullptr);
		// ノーマルテクスチャ読み込み
		hr = LoadTexture(device, filename, nullptr, true, material.normalMap.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	int meshCount = 0;
	for (Mesh& mesh : meshes_)
	{
		// サブセット
		for (Subset& subset : mesh.subsets)
		{
			subset.material = &materials_.at(subset.materialIndex);
		}

		// 頂点バッファ
		{
			D3D11_BUFFER_DESC bufferDesc = {};
			D3D11_SUBRESOURCE_DATA subresourceData = {};

			bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mesh.vertices.size());
			//bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;
			subresourceData.pSysMem = mesh.vertices.data();
			subresourceData.SysMemPitch = 0;
			subresourceData.SysMemSlicePitch = 0;

			HRESULT hr = device->CreateBuffer(&bufferDesc, &subresourceData, mesh.vertexBuffer.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		// インデックスバッファ
		{
			D3D11_BUFFER_DESC bufferDesc = {};
			D3D11_SUBRESOURCE_DATA subresourceData = {};

			bufferDesc.ByteWidth = static_cast<UINT>(sizeof(u_int) * mesh.indices.size());
			//bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;
			subresourceData.pSysMem = mesh.indices.data();
			subresourceData.SysMemPitch = 0; //Not use for index buffers.
			subresourceData.SysMemSlicePitch = 0; //Not use for index buffers.
			HRESULT hr = device->CreateBuffer(&bufferDesc, &subresourceData, mesh.indexBuffer.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		//シェイプデータ
		{
			for (int i = 0; i < mesh.shapeData.size(); ++i)
			{
				shapeIndex_ = meshCount;

				D3D11_BUFFER_DESC buffer_desc{};
				buffer_desc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * mesh.shapeData[i].shapeVertex.size());
				buffer_desc.StructureByteStride = sizeof(DirectX::XMFLOAT3);
				buffer_desc.Usage = D3D11_USAGE_DEFAULT;
				buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
				buffer_desc.CPUAccessFlags = 0;
				buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

				D3D11_SUBRESOURCE_DATA subresource_data{};

				subresource_data.pSysMem = mesh.shapeData[i].shapeVertex.data();
				subresource_data.SysMemPitch = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * mesh.vertices.size());		//配列のサイズ
				subresource_data.SysMemSlicePitch = sizeof(DirectX::XMFLOAT3);	//１要素
				HRESULT hr = device->CreateBuffer(&buffer_desc, &subresource_data, mesh.shapeData[i].sBuffer.GetAddressOf());
				_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = DXGI_FORMAT_UNKNOWN;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
				srvDesc.Buffer.FirstElement = 0;
				srvDesc.Buffer.NumElements = static_cast<UINT>(mesh.shapeData[i].shapeVertex.size());
				device->CreateShaderResourceView(mesh.shapeData[i].sBuffer.Get(), &srvDesc, mesh.shapeData[i].srvBuffer.GetAddressOf());
			}
		}

		meshCount++;
	}
}

// テクスチャ読み込み
HRESULT ModelResource::LoadTexture(ID3D11Device* device, const char* filename, const char* suffix, bool dummy, ID3D11ShaderResourceView** srv, UINT dummy_color)
{
	// パスを分解
	char drive[256], dirname[256], fname[256], ext[256];
	::_splitpath_s(filename, drive, sizeof(drive), dirname, sizeof(dirname), fname, sizeof(fname), ext, sizeof(ext));

	// 末尾文字を追加
	if (suffix != nullptr)
	{
		::strcat_s(fname, sizeof(fname), suffix);
	}
	// パスを結合
	char filepath[256];
	::_makepath_s(filepath, 256, drive, dirname, fname, ext);

	// マルチバイト文字からワイド文字へ変換
	wchar_t wfilepath[256];
	::MultiByteToWideChar(CP_ACP, 0, filepath, -1, wfilepath, 256);

	// テクスチャ読み込み
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	HRESULT hr = DirectX::CreateWICTextureFromFile(device, wfilepath, resource.GetAddressOf(), srv);
	if (FAILED(hr))
	{
		// WICでサポートされていないフォーマットの場合（TGAなど）は
		// STBで画像読み込みをしてテクスチャを生成する
		int width, height, bpp;
		unsigned char* pixels = stbi_load(filepath, &width, &height, &bpp, STBI_rgb_alpha);
		if (pixels != nullptr)
		{
			D3D11_TEXTURE2D_DESC desc = { 0 };
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
			D3D11_SUBRESOURCE_DATA data;
			::memset(&data, 0, sizeof(data));
			data.pSysMem = pixels;
			data.SysMemPitch = width * 4;

			Microsoft::WRL::ComPtr<ID3D11Texture2D>	texture;
			hr = device->CreateTexture2D(&desc, &data, texture.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

			hr = device->CreateShaderResourceView(texture.Get(), nullptr, srv);
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

			// 後始末
			stbi_image_free(pixels);
		}
		else if (dummy)
		{
			// 読み込み失敗したらダミーテクスチャを作る
			LOG("load failed : %s\n", filepath);

			const int width = 8;
			const int height = 8;
			UINT pixels[width * height];
			for (int yy = 0; yy < height; ++yy)
			{
				for (int xx = 0; xx < width; ++xx)
				{
					pixels[yy * width + xx] = dummy_color;
				}
			}

			D3D11_TEXTURE2D_DESC desc = { 0 };
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
			D3D11_SUBRESOURCE_DATA data;
			::memset(&data, 0, sizeof(data));
			data.pSysMem = pixels;
			data.SysMemPitch = width;

			Microsoft::WRL::ComPtr<ID3D11Texture2D>	texture;
			hr = device->CreateTexture2D(&desc, &data, texture.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

			hr = device->CreateShaderResourceView(texture.Get(), nullptr, srv);
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
	}
	return hr;
}

// シリアライズ
void ModelResource::Serialize(const char* filename)
{
	std::ofstream ostream(filename, std::ios::binary);
	if (ostream.is_open())
	{
		cereal::BinaryOutputArchive archive(ostream);

		try
		{
			archive(
				CEREAL_NVP(nodes_),
				CEREAL_NVP(materials_),
				CEREAL_NVP(meshes_),
				CEREAL_NVP(animations_)
			);
		}
		catch (...)
		{
			LOG("model deserialize failed.\n%s\n", filename);
			return;
		}
	}
}

// デシリアライズ
void ModelResource::Deserialize(const char* filename)
{
	std::ifstream istream(filename, std::ios::binary);
	if (istream.is_open())
	{
		cereal::BinaryInputArchive archive(istream);

		try
		{
			archive(
				CEREAL_NVP(nodes_),
				CEREAL_NVP(materials_),
				CEREAL_NVP(meshes_),
				CEREAL_NVP(animations_)
			);
		}
		catch (...)
		{
			LOG("model deserialize failed.\n%s\n", filename);
			return;
		}
	}
	else
	{
		char buffer[256];
		sprintf_s(buffer, sizeof(buffer), "File not found > %s", filename);
		_ASSERT_EXPR_A(false, buffer);
	}
}

// ノードインデックスを取得する
int ModelResource::FindNodeIndex(NodeId nodeId) const
{
	int nodeCount = static_cast<int>(nodes_.size());
	for (int i = 0; i < nodeCount; ++i)
	{
		if (nodes_[i].id == nodeId)
		{
			return i;
		}
	}
	return -1;
}
