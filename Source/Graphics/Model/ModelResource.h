#pragma once

#include <string>
#include <vector>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "../Shaders/3D/ShaderParameter3D.h"

class ModelResource
{
public:
	ModelResource() {}
	virtual ~ModelResource() {}

	using NodeId = UINT64;

	struct Node
	{
		NodeId				id;
		std::string			name;
		std::string			path;
		int					parentIndex;
		DirectX::XMFLOAT3	scale;
		DirectX::XMFLOAT4	rotate;
		DirectX::XMFLOAT3	translate;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct Material
	{
		std::string			name;
		std::string			textureFilename;
		std::string			normalFilename;
		DirectX::XMFLOAT4	color = { 0.8f, 0.8f, 0.8f, 1.0f };

		//トゥーンシェーダー用の構造体
		UnityChanToonStruct toonStruct;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMap;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct Subset
	{
		UINT		startIndex = 0;
		UINT		indexCount = 0;
		int			materialIndex = 0;

		Material* material = nullptr;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct Vertex
	{
		DirectX::XMFLOAT3	position = { 0, 0, 0 };
		DirectX::XMFLOAT3	normal = { 0, 0, 0 };
		DirectX::XMFLOAT3	tangent = { 0, 0, 0 };
		DirectX::XMFLOAT2	texcoord = { 0, 0 };
		DirectX::XMFLOAT4	color = { 1, 1, 1, 1 };
		DirectX::XMFLOAT4	boneWeight = { 1, 0, 0, 0 };
		DirectX::XMUINT4	boneIndex = { 0, 0, 0, 0 };

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	//シェイプデータ
	struct ShapeData
	{
		std::string name;
		float rate = 0;
		std::vector<DirectX::XMFLOAT3> shapeVertex;

		Microsoft::WRL::ComPtr<ID3D11Buffer>			sBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>			srvBuffer;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct Mesh
	{
		std::vector<Vertex>						vertices;
		std::vector<UINT>						indices;
		std::vector<Subset>						subsets;

		int										nodeIndex;
		std::vector<int>						nodeIndices;
		std::vector<DirectX::XMFLOAT4X4>		offsetTransforms;

		DirectX::XMFLOAT3						boundsMin;
		DirectX::XMFLOAT3						boundsMax;

		Microsoft::WRL::ComPtr<ID3D11Buffer>	vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>	indexBuffer;

		std::vector<ShapeData> shapeData;	//シェイプデータ

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct NodeKeyData
	{
		DirectX::XMFLOAT3	scale;
		DirectX::XMFLOAT4	rotate;
		DirectX::XMFLOAT3	translate;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct Keyframe
	{
		float						seconds;
		std::vector<NodeKeyData>	nodeKeys;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct AnimationEvent
	{
		std::string		name;
		float			startFrame;
		float			endFrame;
		Node			eventNode;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct RootPosition
	{
		float frame;
		DirectX::XMFLOAT3 pos;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct Animation
	{
		std::string					name;
		float						secondsLength;
		std::vector<Keyframe>		keyframes;
		std::vector<AnimationEvent>	animationEvents;
		std::vector<RootPosition>	rootPosition;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	// 各種データ取得
	const std::vector<Mesh>& GetMeshes() const { return meshes_; }
	const std::vector<Node>& GetNodes() const { return nodes_; }
	const std::vector<Animation>& GetAnimations() const { return animations_; }
	const std::vector<Material>& GetMaterials() const { return materials_; }
	std::vector<Mesh>& GetMeshesEdit() { return meshes_; }
	std::vector<Material>& GetMaterialsEdit()  { return materials_; }

	const int& GetShapeIndex() const { return shapeIndex_; }

	//シェイプ0クリア
	void ShapeReset();

	// 読み込み
	void Load(ID3D11Device* device, const char* filename);

protected:
	// モデルセットアップ
	void BuildModel(ID3D11Device* device, const char* dirname);

	// テクスチャ読み込み
	HRESULT LoadTexture(ID3D11Device* device, const char* filename, const char* suffix, bool dummy, ID3D11ShaderResourceView** srv, UINT dummy_color = 0xFFFFFFFF);

	// シリアライズ
	void Serialize(const char* filename);

	// デシリアライズ
	void Deserialize(const char* filename);

	// ノードインデックスを取得する
	int FindNodeIndex(NodeId nodeId) const;

protected:
	std::vector<Node>		nodes_;
	std::vector<Material>	materials_;
	std::vector<Mesh>		meshes_;
	std::vector<Animation>	animations_;

	int shapeIndex_ = 0;
};
