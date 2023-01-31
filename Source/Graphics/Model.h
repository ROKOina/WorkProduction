#pragma once

#include <memory>
#include <vector>
#include <DirectXMath.h>
#include "Graphics/ModelResource.h"
#include "Graphics/FbxModelResource.h"

// モデル
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

	// 行列計算
	void UpdateTransform(const DirectX::XMFLOAT4X4& transform);

	// ノードリスト取得
	const std::vector<Node>& GetNodes() const { return nodes; }
	std::vector<Node>& GetNodes() { return nodes; }

	// リソース取得
	const ModelResource* GetResource() const { return resource.get(); }

	//アニメーション更新処理
	void UpdateAnimetion(float elapsedTime);

	//アニメーション再生
	void PlayAnimation(int index, bool loop, float blendSeconds = 0.2f);

	//アニメーション再生中か
	bool IsPlayAnimation()const;

	//ノード検索
	Node* FindNode(const char* name);

	//現在のアニメーション再生時間取得
	float GetCurrentAnimationSecoonds()const { return currentAnimationSeconds; }
private:
	std::shared_ptr<ModelResource>	resource;
	std::shared_ptr<FbxModelResource>	modelResource;	//fbx用
	std::vector<Node>				nodes;

	//アニメーション
	int currentAnimationIndex = -1;
	float currentAnimationSeconds = 0.0f;
	bool animationLoopFlag = false;
	bool animationEndFlag = false;
	float animationBlendTime = 0.0f;
	float animationBlendSeconds = 0.0f;
};
