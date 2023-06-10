#pragma once

#include <memory>
#include <vector>
#include <DirectXMath.h>
#include "Graphics/Model/FbxModelResource.h"

// ���f��
class Model
{
public:
	Model(const char* filename);
	~Model() {}
	void ModelInitialize(std::shared_ptr<ModelResource> resource);

	struct Node
	{
		const char*			name;
		Node*				parent;
		DirectX::XMFLOAT3	scale;
		DirectX::XMFLOAT4	rotate;
		DirectX::XMFLOAT3	translate;
		DirectX::XMFLOAT4X4	localTransform;
		DirectX::XMFLOAT4X4	worldTransform;

		std::vector<Node*>	children;
	};

	// �s��v�Z
	void UpdateTransform(const DirectX::XMFLOAT4X4& transform);

	// �m�[�h���X�g�擾
	const std::vector<Node>& GetNodes() const { return nodes_; }
	std::vector<Node>& GetNodes() { return nodes_; }

	// ���\�[�X�擾
	const FbxModelResource* GetResource() const { return modelResource_.get(); }
	std::shared_ptr<FbxModelResource> GetResourceShared() const { return modelResource_; }

	//�m�[�h����
	Node* FindNode(const char* name);

private:
	//std::shared_ptr<ModelResource>	resource;
	std::shared_ptr<FbxModelResource>	modelResource_;	//fbx�p
	std::vector<Node>				nodes_;
};
