#include "Graphics/Graphics.h"
#include "Model.h"
#include "GameSource\Render\ResourceManager.h"

//コンストラクタで呼ぶイニシャライザ
void Model::ModelInitialize(std::shared_ptr<ModelResource> resource)
{
	//this->resource = resource;

	// ノード
	const std::vector<ModelResource::Node>& resNodes = resource->GetNodes();

	nodes.resize(resNodes.size());
	for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
	{
		auto&& src = resNodes.at(nodeIndex);
		auto&& dst = nodes.at(nodeIndex);

		dst.name = src.name.c_str();
		dst.parent = src.parentIndex >= 0 ? &nodes.at(src.parentIndex) : nullptr;
		dst.scale = src.scale;
		dst.rotate = src.rotate;
		dst.translate = src.translate;

		if (dst.parent != nullptr)
		{
			dst.parent->children.emplace_back(&dst);
		}
	}

	// 行列計算
	const DirectX::XMFLOAT4X4 transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	UpdateTransform(transform);
}

// コンストラクタ
Model::Model(const char* filename)
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	modelResource = std::make_shared<FbxModelResource>();
	modelResource->Load(device, filename);
	ModelInitialize(modelResource);
}

// 変換行列計算
void Model::UpdateTransform(const DirectX::XMFLOAT4X4& transform)
{
	DirectX::XMMATRIX Transform = DirectX::XMLoadFloat4x4(&transform);

	for (Node& node : nodes)
	{
		// ローカル行列算出
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotate));
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(node.translate.x, node.translate.y, node.translate.z);
		DirectX::XMMATRIX LocalTransform = S * R * T;

		// ワールド行列算出
		DirectX::XMMATRIX ParentTransform;
		if (node.parent != nullptr)
		{
			ParentTransform = DirectX::XMLoadFloat4x4(&node.parent->worldTransform);
		}
		else
		{
			ParentTransform = Transform;
		}
		DirectX::XMMATRIX WorldTransform = LocalTransform * ParentTransform;

		// 計算結果を格納
		DirectX::XMStoreFloat4x4(&node.localTransform, LocalTransform);
		DirectX::XMStoreFloat4x4(&node.worldTransform, WorldTransform);
	}
}

//ノード検索
Model::Node* Model::FindNode(const char* name)
{
	//全てのノードを総当たりで名前比較する
	for (auto& node : nodes)
	{
		if (strcmp(node.name, name) == 0)
		{
			return &node;
		}
	}

	//見つからなかった
	return nullptr;
}


// アニメーション入力
void Model::ImportFbxAnimation(const char* filename)
{
	if (modelResource != nullptr)
	{
		//fbx以外は弾く
		const char* ext = strrchr(filename, '.');
		if (::_stricmp(ext, ".fbx") == 0)
		{
			modelResource->AddAnimation(filename);
		}
	}
}
