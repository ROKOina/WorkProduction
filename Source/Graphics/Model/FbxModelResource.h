#pragma once

#include <fbxsdk.h>
#include "ModelResource.h"

class FbxModelResource : public ModelResource
{
public:
	FbxModelResource() {}
	~FbxModelResource() override {}

	// 読み込み
	void Load(ID3D11Device* device, const char* filename, const char* ignoreRootMotionNodeName = nullptr);

	// エクスポート
	void Export(const char* filename);

	// アニメーション追加読み込み
	void AddAnimation(const char* filename);

private:
	// ノードデータを構築
	void LoadNodes(FbxNode* fbxNode, int parentNodeIndex);
	void LoadNode(FbxNode* fbxNode, int parentNodeIndex);

	// メッシュデータを読み込み
	void LoadMeshes(ID3D11Device* device, FbxNode* fbxNode);
	void LoadMesh(ID3D11Device* device, FbxNode* fbxNode, FbxMesh* fbxMesh);

	// マテリアルデータを読み込み
	void LoadMaterials(ID3D11Device* device, const char* dirname, FbxScene* fbxScene);
	void LoadMaterial(ID3D11Device* device, const char* dirname, FbxSurfaceMaterial* fbxSurfaceMaterial);

	// アニメーションデータを読み込み
	void LoadAnimations(FbxScene* fbxScene, const char* name, bool append);

	// 右手座標系から左手座標系へ変換
	void ConvertAxisSystemFromRHtoLH();

	// ノードパス取得
	std::string GetNodePath(FbxNode* fbxNode) const;

	// ノードID取得
	NodeId GetNodeId(FbxNode* fbxNode);

	// マテリアルインデックスを取得する
	int FindMaterialIndex(FbxScene* fbxScene, const FbxSurfaceMaterial* fbxSurfaceMaterial);

	// 無視するモーションノード
	void SetupIgnoreMotionNode(const char* ignoreRootMotionNodeName);

	// 移動値を右手座標系から左手座標系へ変換する
	void ConvertTranslationFromRHtoLH(DirectX::XMFLOAT3& translate);

	// 回転値を右手座標系から左手座標系へ変換する
	void ConvertRotationFromRHtoLH(DirectX::XMFLOAT4& rotate);

	// 行列値を右手座標系から左手座標系へ変換する
	void ConvertMatrixFromRHtoLH(DirectX::XMFLOAT4X4& matrix);

	// インデックスバッファを右手座標系から左手座標系へ変換する
	void ConvertIndexBufferFromRHtoLH(std::vector<UINT>& indices);

private:
	int						rootMotionNodeIndex_ = -1;
};
