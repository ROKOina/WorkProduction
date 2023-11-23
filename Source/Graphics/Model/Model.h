#pragma once

#include <memory>
#include <vector>
#include <DirectXMath.h>
#include "Graphics/Model/FbxModelResource.h"

#include <future>

// モデル
class Model
{
public:
	Model(const char* filename);
	~Model() {}

	void ModelInitialize(const char* filename);

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
	const std::vector<Node>& GetNodes() const { return nodes_; }
	std::vector<Node>& GetNodes() { return nodes_; }

	// リソース取得
	const FbxModelResource* GetResource() const { return modelResource_.get(); }
	std::shared_ptr<FbxModelResource> GetResourceShared() const { return modelResource_; }

	//ノード検索
	Node* FindNode(const char* name);

	//カラー
	const DirectX::XMFLOAT4 GetMaterialColor() const { return materialColor_; }
	void SetMaterialColor(DirectX::XMFLOAT4 color) { materialColor_ = color; }

	void JoinThred() { future.get(); }

	//影を描画するか
	void SetIsShadowDraw(bool flag) { isShadowDraw_ = flag; }
	const bool GetIsShadowDraw() const { return isShadowDraw_; }

private:
	//std::shared_ptr<ModelResource>	resource;
	std::shared_ptr<FbxModelResource>	modelResource_;	//fbx用
	std::vector<Node>				nodes_;

	DirectX::XMFLOAT4 materialColor_ = { 1,1,1,1 };

	bool isShadowDraw_ = false;

	//モデル読み込みをスレッド化
	std::future<void> future;
};
