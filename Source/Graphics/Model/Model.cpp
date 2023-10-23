#include "Graphics/Graphics.h"
#include "Model.h"
#include "ResourceManager.h"
#include "Components/System/GameObject.h"

//�R���X�g���N�^�ŌĂԃC�j�V�����C�U
void Model::ModelInitialize( const char* filename)
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	modelResource_ = std::make_shared<FbxModelResource>();

	//���\�[�X�}�l�[�W���[�ɓo�^����Ă��邩
	if (!ResourceManager::Instance().JudgeModelFilename(filename))
	{
		modelResource_->Load(device, filename);
		ResourceManager::Instance().RegisterModel(filename, modelResource_);	//���\�[�X�}�l�[�W���[�ɒǉ�����
	}
	else
	{
		modelResource_ = ResourceManager::Instance().LoadModelResource(filename);	//���[�h����
	}

	// �m�[�h
	const std::vector<ModelResource::Node>& resNodes = modelResource_->GetNodes();

	nodes_.resize(resNodes.size());
	for (size_t nodeIndex = 0; nodeIndex < nodes_.size(); ++nodeIndex)
	{
		auto&& src = resNodes.at(nodeIndex);
		auto&& dst = nodes_.at(nodeIndex);

		dst.name = src.name.c_str();
		dst.parent = src.parentIndex >= 0 ? &nodes_.at(src.parentIndex) : nullptr;
		dst.scale = src.scale;
		dst.rotate = src.rotate;
		dst.translate = src.translate;

		if (dst.parent != nullptr)
		{
			dst.parent->children.emplace_back(&dst);
		}
	}

	// �s��v�Z
	const DirectX::XMFLOAT4X4 transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	UpdateTransform(transform);
}

// �R���X�g���N�^
Model::Model(const char* filename)
{
	//ModelInitialize(filename);

	//th = std::thread(&Model::ModelInitialize, this, filename);
		future = Graphics::Instance().GetThreadPool()->submit([&](auto filename) { return ModelInitialize(filename); }, filename);
}

// �ϊ��s��v�Z
void Model::UpdateTransform(const DirectX::XMFLOAT4X4& transform)
{
	DirectX::XMMATRIX Transform = DirectX::XMLoadFloat4x4(&transform);

	for (Node& node : nodes_)
	{
		// ���[�J���s��Z�o
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotate));
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(node.translate.x, node.translate.y, node.translate.z);
		DirectX::XMMATRIX LocalTransform = S * R * T;

		// ���[���h�s��Z�o
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

		// �v�Z���ʂ��i�[
		DirectX::XMStoreFloat4x4(&node.localTransform, LocalTransform);
		DirectX::XMStoreFloat4x4(&node.worldTransform, WorldTransform);
	}
}

//�m�[�h����
Model::Node* Model::FindNode(const char* name)
{
	//�S�Ẵm�[�h�𑍓�����Ŗ��O��r����
	for (auto& node : nodes_)
	{
		if (strcmp(node.name, name) == 0)
		{
			return &node;
		}
	}

	//������Ȃ�����
	return nullptr;
}

