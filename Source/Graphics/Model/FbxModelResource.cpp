#include <fstream>
#include <algorithm>
#include "Graphics\Encoding.h"
#include "Misc.h"
#include "Logger.h"
#include "FbxModelResource.h"
#include <filesystem>


// FbxDouble2 → XMFLOAT2
inline DirectX::XMFLOAT2 FbxDouble2ToFloat2(const FbxDouble2& fbxValue)
{
	return DirectX::XMFLOAT2(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1])
	);
}

// FbxDouble3 → XMFLOAT3
inline DirectX::XMFLOAT3 FbxDouble3ToFloat3(const FbxDouble3& fbxValue)
{
	return DirectX::XMFLOAT3(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1]),
		static_cast<float>(fbxValue[2])
	);
}

// FbxDouble4 → XMFLOAT3
inline DirectX::XMFLOAT3 FbxDouble4ToFloat3(const FbxDouble4& fbxValue)
{
	return DirectX::XMFLOAT3(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1]),
		static_cast<float>(fbxValue[2])
	);
}

// FbxDouble4 → XMFLOAT4
inline DirectX::XMFLOAT4 FbxDouble4ToFloat4(const FbxDouble4& fbxValue)
{
	return DirectX::XMFLOAT4(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1]),
		static_cast<float>(fbxValue[2]),
		static_cast<float>(fbxValue[3])
	);
}

// FbxDouble4 → XMFLOAT4
inline DirectX::XMFLOAT4X4 FbxAMatrixToFloat4x4(const FbxAMatrix& fbxValue)
{
	return DirectX::XMFLOAT4X4(
		static_cast<float>(fbxValue[0][0]),
		static_cast<float>(fbxValue[0][1]),
		static_cast<float>(fbxValue[0][2]),
		static_cast<float>(fbxValue[0][3]),
		static_cast<float>(fbxValue[1][0]),
		static_cast<float>(fbxValue[1][1]),
		static_cast<float>(fbxValue[1][2]),
		static_cast<float>(fbxValue[1][3]),
		static_cast<float>(fbxValue[2][0]),
		static_cast<float>(fbxValue[2][1]),
		static_cast<float>(fbxValue[2][2]),
		static_cast<float>(fbxValue[2][3]),
		static_cast<float>(fbxValue[3][0]),
		static_cast<float>(fbxValue[3][1]),
		static_cast<float>(fbxValue[3][2]),
		static_cast<float>(fbxValue[3][3])
	);
}


// 読み込み
void FbxModelResource::Load(ID3D11Device* device, const char* filename, const char* ignoreRootMotionNodeName)
{		
	// ディレクトリパス取得
	char drive[32], dir[256], dirname[256];
	::_splitpath_s(filename, drive, sizeof(drive), dir, sizeof(dir), nullptr, 0, nullptr, 0);
	::_makepath_s(dirname, sizeof(dirname), drive, dir, nullptr, nullptr);

	//保存データあればそれでロード
	std::filesystem::path cereal_filename(filename);
	cereal_filename.replace_extension("mdl");
	std::string cereal_filenameString = cereal_filename.string<char>();
	if (std::filesystem::exists(cereal_filename.c_str()))
	{
		Deserialize(cereal_filenameString.c_str());
	}
	else
	{
		const char* ext = strrchr(filename, '.');
		if (::_stricmp(ext, ".fbx") == 0)
		{
			// FBXのファイルパスはUTF-8にする必要がある
			char fbxFilename[256];
			Encoding::StringToUTF8(filename, fbxFilename, sizeof(fbxFilename));

			FbxManager* fbxManager = FbxManager::Create();

			// FBXに対する入出力を定義する
			FbxIOSettings* fbxIOS = FbxIOSettings::Create(fbxManager, IOSROOT);	// 特別な理由がない限りIOSROOTを指定
			fbxManager->SetIOSettings(fbxIOS);

			// インポータを生成
			FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
			bool result = fbxImporter->Initialize(fbxFilename, -1, fbxManager->GetIOSettings());	// -1でファイルフォーマット自動判定
			_ASSERT_EXPR_A(result, "FbxImporter::Initialize() : Failed!!\n");

			// SceneオブジェクトにFBXファイル内の情報を流し込む
			FbxScene* fbxScene = FbxScene::Create(fbxManager, "scene");
			fbxImporter->Import(fbxScene);
			fbxImporter->Destroy();	// シーンを流し込んだらImporterは解放してOK

			// ジオメトリを三角形化しておく
			FbxGeometryConverter fbxGeometryConverter(fbxManager);
			fbxGeometryConverter.Triangulate(fbxScene, true);
			fbxGeometryConverter.RemoveBadPolygonsFromMeshes(fbxScene);

			// 軸がZ-Upの場合はY-Upに変換する
			FbxAxisSystem fbx_scene_axis_system = fbxScene->GetGlobalSettings().GetAxisSystem();
			if (fbx_scene_axis_system == FbxAxisSystem::eMayaZUp)
			{
				FbxAxisSystem::MayaYUp.ConvertScene(fbxScene);
			}

			// モデル読み込み
			std::vector<FbxNode*> fbxNodes;
			FbxNode* fbxRootNode = fbxScene->GetRootNode();
			LoadMaterials(device, dirname, fbxScene);
			LoadNodes(fbxRootNode, -1);
			LoadMeshes(device, fbxRootNode);

			// 無視するモーションノードを検索
			SetupIgnoreMotionNode(ignoreRootMotionNodeName);

			// アニメーション読み込み
			char name[256];
			::_splitpath_s(filename, nullptr, 0, nullptr, 0, name, 256, nullptr, 0);

			LoadAnimations(fbxScene, nullptr, false);

			// マネージャ解放
			fbxManager->Destroy();		// 関連するすべてのオブジェクトが解放される

			//.cerealに作成
			Export(cereal_filenameString.c_str());
		}
		else
		{
			// デシリアライズ
			Deserialize(filename);

			// 無視するモーションノードを検索
			SetupIgnoreMotionNode(ignoreRootMotionNodeName);
		}

	}
	// モデル構築
	BuildModel(device, dirname);
}

// エクスポート
void FbxModelResource::Export(const char* filename)
{
	Serialize(filename);
}

// アニメーション追加
void FbxModelResource::AddAnimation(const char* filename)
{
	// FBXのファイルパスはUTF-8にする必要がある
	char fbxFilename[256];
	Encoding::StringToUTF8(filename, fbxFilename, sizeof(fbxFilename));

	FbxManager* fbxManager = FbxManager::Create();

	// FBXに対する入出力を定義する
	FbxIOSettings* fbxIOS = FbxIOSettings::Create(fbxManager, IOSROOT);	// 特別な理由がない限りIOSROOTを指定
	fbxManager->SetIOSettings(fbxIOS);

	// インポータを生成
	FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
	bool result = fbxImporter->Initialize(fbxFilename, -1, fbxManager->GetIOSettings());	// -1でファイルフォーマット自動判定
	_ASSERT_EXPR_A(result, "FbxImporter::Initialize() : Failed!!\n");

	// SceneオブジェクトにFBXファイル内の情報を流し込む
	FbxScene* fbxScene = FbxScene::Create(fbxManager, "scene");
	fbxImporter->Import(fbxScene);
	fbxImporter->Destroy();	// シーンを流し込んだらImporterは解放してOK

	// 軸がZ-Upの場合はY-Upに変換する
	FbxAxisSystem fbx_scene_axis_system = fbxScene->GetGlobalSettings().GetAxisSystem();
	if (fbx_scene_axis_system == FbxAxisSystem::eMayaZUp)
	{
		FbxAxisSystem::MayaYUp.ConvertScene(fbxScene);
	}

	// アニメーション読み込み
	char name[256];
	::_splitpath_s(filename, nullptr, 0, nullptr, 0, name, 256, nullptr, 0);

	char utf8[256];
	Encoding::StringToUTF8(name, utf8, sizeof(utf8));

	LoadAnimations(fbxScene, utf8, true);

	// マネージャ解放
	fbxManager->Destroy();		// 関連するすべてのオブジェクトが解放される
}

// FBXノードを再帰的に辿ってデータを読み込む
void FbxModelResource::LoadNodes(FbxNode* fbxNode, int parentNodeIndex)
{
	FbxNodeAttribute* fbxNodeAttribute = fbxNode->GetNodeAttribute();
	FbxNodeAttribute::EType fbxNodeAttributeType = FbxNodeAttribute::EType::eUnknown;

	if (fbxNodeAttribute != nullptr)
	{
		fbxNodeAttributeType = fbxNodeAttribute->GetAttributeType();
	}

	switch (fbxNodeAttributeType)
	{
	case FbxNodeAttribute::eUnknown:
	case FbxNodeAttribute::eNull:
	case FbxNodeAttribute::eMarker:
	case FbxNodeAttribute::eMesh:
	case FbxNodeAttribute::eSkeleton:
		LoadNode(fbxNode, parentNodeIndex);
		break;
	}

	// 再帰的に子ノードを処理する
	parentNodeIndex = static_cast<int>(nodes_.size() - 1);
	for (int i = 0; i < fbxNode->GetChildCount(); ++i)
	{
		LoadNodes(fbxNode->GetChild(i), parentNodeIndex);
	}
}

// FBXノードからノードデータを読み込み
void FbxModelResource::LoadNode(FbxNode* fbxNode, int parentNodeIndex)
{
	FbxAMatrix& fbxLocalTransform = fbxNode->EvaluateLocalTransform();

	Node node;
	node.id = GetNodeId(fbxNode);
	node.name = fbxNode->GetName();
	node.path = GetNodePath(fbxNode);
	node.parentIndex = parentNodeIndex;
	node.scale = FbxDouble4ToFloat3(fbxLocalTransform.GetS());
	node.rotate = FbxDouble4ToFloat4(fbxLocalTransform.GetQ());
	node.translate = FbxDouble4ToFloat3(fbxLocalTransform.GetT());

	// 座標軸変換
	ConvertTranslationFromRHtoLH(node.translate);
	ConvertRotationFromRHtoLH(node.rotate);

	nodes_.emplace_back(node);
}

// FBXノードを再帰的に辿ってメッシュデータを読み込み
void FbxModelResource::LoadMeshes(ID3D11Device* device, FbxNode* fbxNode)
{
	FbxNodeAttribute* fbxNodeAttribute = fbxNode->GetNodeAttribute();
	FbxNodeAttribute::EType fbxNodeAttributeType = FbxNodeAttribute::EType::eUnknown;

	if (fbxNodeAttribute != nullptr)
	{
		fbxNodeAttributeType = fbxNodeAttribute->GetAttributeType();
	}

	switch (fbxNodeAttributeType)
	{
	case FbxNodeAttribute::eMesh:
		LoadMesh(device, fbxNode, static_cast<FbxMesh*>(fbxNodeAttribute));
		break;
	}

	// 再帰的に子ノードを処理する
	for (int i = 0; i < fbxNode->GetChildCount(); ++i)
	{
		LoadMeshes(device, fbxNode->GetChild(i));
	}
}

// FBXメッシュからメッシュデータを読み込み
void FbxModelResource::LoadMesh(ID3D11Device* device, FbxNode* fbxNode, FbxMesh* fbxMesh)
{
	int fbxControlPointsCount = fbxMesh->GetControlPointsCount();
	//int fbxPolygonVertexCount = fbxMesh->GetPolygonVertexCount();
	//const int* fbxPolygonVertices = fbxMesh->GetPolygonVertices();

	int fbxMaterialCount = fbxNode->GetMaterialCount();
	int fbxPolygonCount = fbxMesh->GetPolygonCount();
	std::string& fbxNodePath = GetNodePath(fbxNode);

	meshes_.emplace_back(Mesh());
	Mesh& mesh = meshes_.back();
	NodeId nodeId = GetNodeId(fbxNode);
	mesh.nodeIndex = FindNodeIndex(nodeId);
	mesh.subsets.resize(fbxMaterialCount > 0 ? fbxMaterialCount : 1);
	mesh.boundsMin.x = mesh.boundsMin.y = mesh.boundsMin.z = FLT_MAX;
	mesh.boundsMax.x = mesh.boundsMax.y = mesh.boundsMax.z = -FLT_MAX;

	// サブセットのマテリアル設定
	for (int fbxMaterialIndex = 0; fbxMaterialIndex < fbxMaterialCount; ++fbxMaterialIndex)
	{
		const FbxSurfaceMaterial* fbxSurfaceMaterial = fbxNode->GetMaterial(fbxMaterialIndex);

		Subset& subset = mesh.subsets.at(fbxMaterialIndex);
		subset.materialIndex = FindMaterialIndex(fbxNode->GetScene(), fbxSurfaceMaterial);
	}

	// サブセットの頂点インデックス範囲設定
	if (fbxMaterialCount > 0)
	{
		for (int fbxPolygonIndex = 0; fbxPolygonIndex < fbxPolygonCount; ++fbxPolygonIndex)
		{
			int fbxMaterialIndex = fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(fbxPolygonIndex);
			mesh.subsets.at(fbxMaterialIndex).indexCount += 3;
		}

		int offset = 0;
		for (Subset& subset : mesh.subsets)
		{
			subset.startIndex = offset;
			offset += subset.indexCount;

			subset.indexCount = 0;
		}
	}

	// 頂点影響力データ
	struct BoneInfluence
	{
		int useCount = 0;
		int indices[4] = { 0, 0, 0, 0 };
		float weights[4] = { 1.0f, 0.0f, 0.0f, 0.0f };

		void Add(int index, float weight)
		{
			if (useCount < 4)
			{
				indices[useCount] = index;
				weights[useCount] = weight;
				useCount++;
			}
		}
	};
	// 頂点影響力データを抽出する
	std::vector<BoneInfluence> boneInfluences;
	{
		boneInfluences.resize(fbxControlPointsCount);

		// スキニングに必要な情報を取得する
		int fbxDeformerCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
		for (int fbxDeformerIndex = 0; fbxDeformerIndex < fbxDeformerCount; ++fbxDeformerIndex)
		{
			FbxSkin* fbxSkin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(fbxDeformerIndex, FbxDeformer::eSkin));

			int fbxClusterCount = fbxSkin->GetClusterCount();
			for (int fbxClusterIndex = 0; fbxClusterIndex < fbxClusterCount; ++fbxClusterIndex)
			{
				FbxCluster* fbxCluster = fbxSkin->GetCluster(fbxClusterIndex);

				// 頂点影響力データを抽出する
				{
					int fbxClusterControlPointIndicesCount = fbxCluster->GetControlPointIndicesCount();
					const int* fbxControlPointIndices = fbxCluster->GetControlPointIndices();
					const double* fbxControlPointWeights = fbxCluster->GetControlPointWeights();

					for (int i = 0; i < fbxClusterControlPointIndicesCount; ++i)
					{
						BoneInfluence& boneInfluence = boneInfluences.at(fbxControlPointIndices[i]);

						boneInfluence.Add(fbxClusterIndex, static_cast<float>(fbxControlPointWeights[i]));
					}
				}

				// メッシュ空間からボーン空間への変換行列の計算をする
				{
					// メッシュ空間からグローバル空間への変換行列
					FbxAMatrix fbxMeshToGlobalSpaceTransform;
					fbxCluster->GetTransformMatrix(fbxMeshToGlobalSpaceTransform);

					// ボーン空間からグローバル空間への変換行列
					FbxAMatrix fbxBoneToGlobalSpaceTransform;
					fbxCluster->GetTransformLinkMatrix(fbxBoneToGlobalSpaceTransform);

					// メッシュ空間からボーン空間への変換行列
					FbxAMatrix fbxMeshToBoneTransform = fbxBoneToGlobalSpaceTransform.Inverse() * fbxMeshToGlobalSpaceTransform;

					// 座標系変換
					DirectX::XMFLOAT4X4 offsetTransform = FbxAMatrixToFloat4x4(fbxMeshToBoneTransform);
					ConvertMatrixFromRHtoLH(offsetTransform);

					mesh.offsetTransforms.emplace_back(offsetTransform);

					// 影響するボーンインデックスを名前で検索する
					NodeId nodeId = GetNodeId(fbxCluster->GetLink());
					int nodeIndex = FindNodeIndex(nodeId);
					mesh.nodeIndices.emplace_back(nodeIndex);
				}
			}
		}
	}

	// ジオメトリ変換行列
	FbxAMatrix fbxGeometricTransform(
		fbxNode->GetGeometricTranslation(FbxNode::eSourcePivot),
		fbxNode->GetGeometricRotation(FbxNode::eSourcePivot),
		fbxNode->GetGeometricScaling(FbxNode::eSourcePivot)
		);
	DirectX::XMFLOAT4X4 geometricTransform = FbxAMatrixToFloat4x4(fbxGeometricTransform);
	DirectX::XMMATRIX GM = DirectX::XMLoadFloat4x4(&geometricTransform);
		
	// UVセット名
	FbxStringList fbxUVSetNames;
	fbxMesh->GetUVSetNames(fbxUVSetNames);

	// 頂点データ
	mesh.vertices.resize(fbxPolygonCount * 3);
	mesh.indices.resize(fbxPolygonCount * 3);

	int vertexCount = 0;
	const FbxVector4* fbxControlPoints = fbxMesh->GetControlPoints();
	for (int fbxPolygonIndex = 0; fbxPolygonIndex < fbxPolygonCount; ++fbxPolygonIndex)
	{
		// ポリゴンに適用されているマテリアルインデックスを取得する
		int fbxMaterialIndex = 0;
		if (fbxMaterialCount > 0)
		{
			fbxMaterialIndex = fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(fbxPolygonIndex);
		}

		Subset& subset = mesh.subsets.at(fbxMaterialIndex);
		const int indexOffset = subset.startIndex + subset.indexCount;

		for (int fbxVertexIndex = 0; fbxVertexIndex < 3; ++fbxVertexIndex)
		{
			Vertex vertex;

			int fbxControlPointIndex = fbxMesh->GetPolygonVertex(fbxPolygonIndex, fbxVertexIndex);
			// Position
			{
				DirectX::XMFLOAT3 position = FbxDouble4ToFloat3(fbxControlPoints[fbxControlPointIndex]);
				DirectX::XMVECTOR V = DirectX::XMLoadFloat3(&position);
				V = DirectX::XMVector3TransformCoord(V, GM);
				DirectX::XMStoreFloat3(&vertex.position, V);
			}

			// Weight
			{
				BoneInfluence& boneInfluence = boneInfluences.at(fbxControlPointIndex);
				vertex.boneIndex.x = boneInfluence.indices[0];
				vertex.boneIndex.y = boneInfluence.indices[1];
				vertex.boneIndex.z = boneInfluence.indices[2];
				vertex.boneIndex.w = boneInfluence.indices[3];
				vertex.boneWeight.x = boneInfluence.weights[0];
				vertex.boneWeight.y = boneInfluence.weights[1];
				vertex.boneWeight.z = boneInfluence.weights[2];
				//vertex.boneWeight.w = 1.0f - (vertex.boneWeight.x + vertex.boneWeight.y + vertex.boneWeight.z);
				vertex.boneWeight.w = boneInfluence.weights[3];
			}

			// Normal
			if (fbxMesh->GetElementNormalCount() > 0)
			{
				FbxVector4 fbxNormal;
				fbxMesh->GetPolygonVertexNormal(fbxPolygonIndex, fbxVertexIndex, fbxNormal);

				DirectX::XMFLOAT3 normal = FbxDouble4ToFloat3(fbxNormal);
				DirectX::XMVECTOR V = DirectX::XMLoadFloat3(&normal);
				V = DirectX::XMVector3TransformCoord(V, GM);
				V = DirectX::XMVector3Normalize(V);
				DirectX::XMStoreFloat3(&vertex.normal, V);

				vertex.normal = FbxDouble4ToFloat3(fbxNormal);
			}
			else
			{
				vertex.normal = DirectX::XMFLOAT3(0, 0, 0);
			}

			// Texcoord
			if (fbxMesh->GetElementUVCount() > 0)
			{
				bool fbxUnmappedUV;
				FbxVector2 fbxUV;
				fbxMesh->GetPolygonVertexUV(fbxPolygonIndex, fbxVertexIndex, fbxUVSetNames[0], fbxUV, fbxUnmappedUV);
				fbxUV[1] = 1.0 - fbxUV[1];
				vertex.texcoord = FbxDouble2ToFloat2(fbxUV);
			}
			else
			{
				vertex.texcoord = DirectX::XMFLOAT2(0, 0);
			}

			// 座標系変換
			ConvertTranslationFromRHtoLH(vertex.position);
			ConvertTranslationFromRHtoLH(vertex.normal);

			// 頂点＆インデックスデータ設定
			mesh.indices.at(indexOffset + fbxVertexIndex) = vertexCount;
			mesh.vertices.at(vertexCount) = vertex;
			vertexCount++;

			// バウンディングボックス
			mesh.boundsMin.x = std::fminf(mesh.boundsMin.x, vertex.position.x);
			mesh.boundsMin.y = std::fminf(mesh.boundsMin.y, vertex.position.y);
			mesh.boundsMin.z = std::fminf(mesh.boundsMin.z, vertex.position.z);
			mesh.boundsMax.x = std::fmaxf(mesh.boundsMax.x, vertex.position.x);
			mesh.boundsMax.y = std::fmaxf(mesh.boundsMax.y, vertex.position.y);
			mesh.boundsMax.z = std::fmaxf(mesh.boundsMax.z, vertex.position.z);
		}

		subset.indexCount += 3;
	}

	// タンジェント
	fbxMesh->GenerateTangentsData(0, true);
	fbxsdk::FbxGeometryElementTangent* fbxElementTangent = fbxMesh->GetElementTangent(0);
	if (fbxElementTangent != nullptr)
	{
		int fbxPolygonVertexCount = fbxMesh->GetPolygonVertexCount();
		for (int i = 0; i < fbxPolygonVertexCount; i++)
		{
			FbxVector4 fbxTangent = fbxElementTangent->GetDirectArray().GetAt(i);
			mesh.vertices[i].tangent.x = static_cast<float>(fbxTangent[0]);
			mesh.vertices[i].tangent.y = static_cast<float>(fbxTangent[1]);
			mesh.vertices[i].tangent.z = static_cast<float>(fbxTangent[2]);

			// 座標系変換
			ConvertTranslationFromRHtoLH(mesh.vertices[i].tangent);
		}
	}

	// 頂点カラー
	int fbxElementVertexColorLayerCount = fbxMesh->GetElementVertexColorCount();
	if (fbxElementVertexColorLayerCount > 0)
	{
		// 頂点カラーレイヤー取得
		FbxGeometryElementVertexColor* fbxElementVertexColor = fbxMesh->GetElementVertexColor(0);

		// 保存形式の取得
		FbxGeometryElement::EMappingMode fbxMappingMode = fbxElementVertexColor->GetMappingMode();
		FbxGeometryElement::EReferenceMode fbxReferenceMode = fbxElementVertexColor->GetReferenceMode();

		// ポリゴン頂点に対するインデックス参照形式のみ対応
		if (fbxMappingMode == FbxGeometryElement::eByPolygonVertex)
		{
			if (fbxReferenceMode == FbxGeometryElement::eIndexToDirect)
			{
				FbxLayerElementArrayTemplate<int>* indexArray = &fbxElementVertexColor->GetIndexArray();
				int indexCount = indexArray->GetCount();
				for (int i = 0; i < indexCount; i++)
				{
					FbxColor fbxColor = fbxElementVertexColor->GetDirectArray().GetAt(indexArray->GetAt(i));
					mesh.vertices[i].color.x = static_cast<float>(fbxColor.mRed);
					mesh.vertices[i].color.y = static_cast<float>(fbxColor.mGreen);
					mesh.vertices[i].color.z = static_cast<float>(fbxColor.mBlue);
					mesh.vertices[i].color.w = static_cast<float>(fbxColor.mAlpha);
				}
			}
		}
	}

#if 0	// ミラーリングのモデルデータでうまくいかないときがある
	// 頂点最適化
	int currentVertexCount = 0;
	vertexCount = static_cast<int>(mesh.vertices.size());
	for (int i = 0; i < vertexCount; ++i)
	{
		int sameIndex = -1;

		Vertex& a = mesh.vertices[i];
		//	同一頂点検索
		for (int j = 0; j < currentVertexCount; j++)
		{
			Vertex& b = mesh.vertices[j];

			if (a.position.x != b.position.x) continue;
			if (a.position.y != b.position.y) continue;
			if (a.position.z != b.position.z) continue;
			if (a.normal.x != b.normal.x) continue;
			if (a.normal.y != b.normal.y) continue;
			if (a.normal.z != b.normal.z) continue;
			if (a.tangent.x != b.tangent.x) continue;
			if (a.tangent.y != b.tangent.y) continue;
			if (a.tangent.z != b.tangent.z) continue;
			if (a.texcoord.x != b.texcoord.x) continue;
			if (a.texcoord.y != b.texcoord.y) continue;
			if (a.color.x != b.color.x) continue;
			if (a.color.y != b.color.y) continue;
			if (a.color.z != b.color.z) continue;
			if (a.color.w != b.color.w) continue;
			if (a.boneWeight.x != b.boneWeight.x) continue;
			if (a.boneWeight.y != b.boneWeight.y) continue;
			if (a.boneWeight.z != b.boneWeight.z) continue;
			if (a.boneWeight.w != b.boneWeight.w) continue;
			if (a.boneIndex.x != b.boneIndex.x) continue;
			if (a.boneIndex.y != b.boneIndex.y) continue;
			if (a.boneIndex.z != b.boneIndex.z) continue;
			if (a.boneIndex.w != b.boneIndex.w) continue;

			sameIndex = j;
			break;
		}

		int index = sameIndex;

		//	新規頂点
		if (index == -1)
		{
			mesh.vertices[currentVertexCount] = a;
			index = currentVertexCount;
			currentVertexCount++;
		}
		//	インデックス更新
		for (int j = 0; j < vertexCount; ++j)
		{
			if (mesh.indices[j] == i)
			{
				mesh.indices[i] = index;
			}
		}
	}
	LOG("%s : vertex %d > %d\n", fbxNode->GetName(), mesh.vertices.size(), currentVertexCount);
	mesh.vertices.resize(currentVertexCount);
#endif

	// 座標系変換
	ConvertIndexBufferFromRHtoLH(mesh.indices);
}

// FBXシーン内のFBXマテリアルからマテリアルデータを読み込み
void FbxModelResource::LoadMaterials(ID3D11Device* device, const char* dirname, FbxScene* fbxScene)
{
	int fbxMaterialCount = fbxScene->GetMaterialCount();

	if (fbxMaterialCount > 0)
	{
		for (int fbxMaterialIndex = 0; fbxMaterialIndex < fbxMaterialCount; ++fbxMaterialIndex)
		{
			FbxSurfaceMaterial* fbxSurfaceMaterial = fbxScene->GetMaterial(fbxMaterialIndex);

			LoadMaterial(device, dirname, fbxSurfaceMaterial);
		}
	}
	else
	{
		Material material;
		material.name = "Dummy";
		material.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		materials_.emplace_back(material);
	}
}

// FBXマテリアルからマテリアルデータを読み込み
void FbxModelResource::LoadMaterial(ID3D11Device* device, const char* dirname, FbxSurfaceMaterial* fbxSurfaceMaterial)
{
	bool ret = false;

	Material material;

	material.name = fbxSurfaceMaterial->GetName();

	// ディフューズカラー
	FbxProperty fbxDiffuseProperty = fbxSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
	FbxProperty fbxDiffuseFactorProperty = fbxSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuseFactor);
	if (fbxDiffuseProperty.IsValid() && fbxDiffuseFactorProperty.IsValid())
	{
		FbxDouble fbxFactor = fbxDiffuseFactorProperty.Get<FbxDouble>();
		FbxDouble3 fbxColor = fbxDiffuseProperty.Get<FbxDouble3>();

		material.color.x = static_cast<float>(fbxColor[0] * fbxFactor);
		material.color.y = static_cast<float>(fbxColor[1] * fbxFactor);
		material.color.z = static_cast<float>(fbxColor[2] * fbxFactor);
		material.color.w = 1.0f;
	}
#if 0
	FbxProperty fbx_transparency_factor_property = fbxSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
	if (fbx_transparency_factor_property.IsValid())
	{
		FbxDouble fbxFactor = fbx_transparency_factor_property.Get<FbxDouble>();

		material.color.w = static_cast<float>(fbxFactor);
	}
#endif
	// ディフューズテクスチャ
	if (fbxDiffuseProperty.IsValid())
	{
		int fbxTextureCount = fbxDiffuseProperty.GetSrcObjectCount<FbxFileTexture>();
		if (fbxTextureCount > 0)
		{
			FbxFileTexture* fbxTexture = fbxDiffuseProperty.GetSrcObject<FbxFileTexture>();

			const char* relativeFileName = fbxTexture->GetRelativeFileName();

			// テクスチャファイルが存在しているか確認
			char filename[256];
			::_makepath_s(filename, sizeof(filename), nullptr, dirname, relativeFileName, nullptr);
			std::ifstream istream(filename, std::ios::binary);
			if (istream.is_open())
			{
				// 見つかったのでそのまま設定する
				material.textureFilename = relativeFileName;
			}
			else
			{
				// 見つからなかった場合はモデルと同ディレクトリにあれば読み込めるようにする
				char fname[256], ext[32];
				::_splitpath_s(relativeFileName, nullptr, 0, nullptr, 0, fname, sizeof(fname), ext, sizeof(ext));
				::_makepath_s(filename, sizeof(filename), nullptr, nullptr, fname, ext);
				material.textureFilename = filename;
			}
		}
	}

	materials_.emplace_back(material);
}

// アニメーションデータを読み込み
void FbxModelResource::LoadAnimations(FbxScene* fbxScene, const char* name, bool append)
{
	// すべてのアニメーション名を取得
	FbxArray<FbxString*> fbxAnimStackNames;
	fbxScene->FillAnimStackNameArray(fbxAnimStackNames);

	int fbxAnimationCount = fbxAnimStackNames.Size();
	for (int fbxAnimationIndex = 0; fbxAnimationIndex < fbxAnimationCount; ++fbxAnimationIndex)
	{
		//if (0 != ::strncmp(fbxAnimStackNames.GetAt(fbxAnimationIndex)->Buffer(), u8"基本", strlen(u8"基本"))) continue;
		
		animations_.emplace_back(Animation());
		Animation& animation = animations_.back();

		// アニメーションデータのサンプリング設定
		FbxTime::EMode fbxTimeMode = fbxScene->GetGlobalSettings().GetTimeMode();
		FbxTime fbxFrameTime;
		fbxFrameTime.SetTime(0, 0, 0, 1, 0, fbxTimeMode);

		float samplingRate = static_cast<float>(fbxFrameTime.GetFrameRate(fbxTimeMode));
		float samplingTime = 1.0f / samplingRate;

		FbxString* fbxAnimStackName = fbxAnimStackNames.GetAt(fbxAnimationIndex);
		FbxAnimStack* fbxAnimStack = fbxScene->FindMember<FbxAnimStack>(fbxAnimStackName->Buffer());

		// 再生するアニメーションを指定する。
		fbxScene->SetCurrentAnimationStack(fbxAnimStack);

		// アニメーションの再生開始時間と再生終了時間を取得する
		FbxTakeInfo* fbxTakeInfo = fbxScene->GetTakeInfo(fbxAnimStackName->Buffer());
		FbxTime fbxStartTime = fbxTakeInfo->mLocalTimeSpan.GetStart();
		FbxTime fbxEndTime = fbxTakeInfo->mLocalTimeSpan.GetStop();

		// 抽出するデータは60フレーム基準でサンプリングする
		FbxTime fbxSamplingStep;
		fbxSamplingStep.SetTime(0, 0, 1, 0, 0, fbxTimeMode);
		fbxSamplingStep = static_cast<FbxLongLong>(fbxSamplingStep.Get() * samplingTime);

		int startFrame = static_cast<int>(fbxStartTime.Get() / fbxSamplingStep.Get());
		int endFrame = static_cast<int>(fbxEndTime.Get() / fbxSamplingStep.Get());
		int frameCount = static_cast<int>((fbxEndTime.Get() - fbxStartTime.Get()) / fbxSamplingStep.Get());

		// アニメーションの対象となるノードを列挙する
		std::vector<FbxNode*> fbxNodes;
		if (append)
		{
			// ノード名を比較して対象ノードを列挙する
			// ※名前が重複していると失敗する場合がある
			FbxNode* fbxRootNode = fbxScene->GetRootNode();
			for (Node& node : nodes_)
			{
				FbxNode* fbxNode = fbxRootNode->FindChild(node.name.c_str(), true, true);
				fbxNodes.emplace_back(fbxNode);
			}
		}
		else
		{
			// ノードの完全パスを比較して対象ノードを列挙する（重い）
			// ※必ずモデルとアニメーションのFBXのノードツリー構成が一致している必要がある
			for (Node& node : nodes_)
			{
				FbxNode* fbxAnimationNode = nullptr;
				for (int fbxNodeIndex = 0; fbxNodeIndex < fbxScene->GetNodeCount(); ++fbxNodeIndex)
				{
					FbxNode* fbxNode = fbxScene->GetNode(fbxNodeIndex);
					NodeId nodeId = GetNodeId(fbxNode);
					if (node.id == nodeId)
					{
						fbxAnimationNode = fbxNode;
						break;
					}
				}
				fbxNodes.emplace_back(fbxAnimationNode);
			}
		}

		// アニメーション名
		char animationName[256];
		if (name != nullptr && fbxAnimationCount == 1)
		{
			::sprintf_s(animationName, "%s", name);
		}
		else
		{
			::sprintf_s(animationName, "%s", fbxAnimStackName->Buffer());
		}

		animation.name = animationName;

		// アニメーションデータを抽出する
		animation.secondsLength = frameCount * samplingTime;
		animation.keyframes.resize(frameCount + 1);

		float seconds = 0.0f;
		Keyframe* keyframe = animation.keyframes.data();
		size_t fbxNodeCount = fbxNodes.size();
		FbxTime fbxCurrentTime = fbxStartTime;
		for (FbxTime fbxCurrentTime = fbxStartTime; fbxCurrentTime < fbxEndTime; fbxCurrentTime += fbxSamplingStep, ++keyframe)
		{
			// キーフレーム毎の姿勢データを取り出す。
			keyframe->seconds = seconds;
			keyframe->nodeKeys.resize(fbxNodeCount);
			for (size_t fbxNodeIndex = 0; fbxNodeIndex < fbxNodeCount; ++fbxNodeIndex)
			{
				NodeKeyData& keyData = keyframe->nodeKeys.at(fbxNodeIndex);
				FbxNode* fbxNode = fbxNodes.at(fbxNodeIndex);
				if (fbxNode == nullptr)
				{
					// アニメーション対象のノードがなかったのでダミーデータを設定
					Node& node = nodes_.at(fbxNodeIndex);
					keyData.scale = node.scale;
					keyData.rotate = node.rotate;
					keyData.translate = node.translate;
				}
				else if (fbxNodeIndex == rootMotionNodeIndex_)
				{
					// ルートモーションは無視する
					Node& node = nodes_.at(fbxNodeIndex);
					keyData.scale = DirectX::XMFLOAT3(1, 1, 1);
					keyData.rotate = DirectX::XMFLOAT4(0, 0, 0, 1);
					keyData.translate = DirectX::XMFLOAT3(0, 0, 0);
				}
				else
				{
					// 指定時間のローカル行列からスケール値、回転値、移動値を取り出す。
					const FbxAMatrix& fbxLocalTransform = fbxNode->EvaluateLocalTransform(fbxCurrentTime);

					keyData.scale = FbxDouble4ToFloat3(fbxLocalTransform.GetS());
					keyData.rotate = FbxDouble4ToFloat4(fbxLocalTransform.GetQ());
					keyData.translate = FbxDouble4ToFloat3(fbxLocalTransform.GetT());
				}
				// 座標系変換
				ConvertTranslationFromRHtoLH(keyData.translate);
				ConvertRotationFromRHtoLH(keyData.rotate);
			}
			seconds += samplingTime;
		}
	}

	// 後始末
	for (int i = 0; i < fbxAnimationCount; i++)
	{
		delete fbxAnimStackNames[i];
	}

	//std::sort(animations_.begin(), animations_.end(), [](auto& a, auto& b) { return strcmp(a.name.c_str(), b.name.c_str()); });
}

// 右手座標系から左手座標系へ変換
void FbxModelResource::ConvertAxisSystemFromRHtoLH()
{
#if 0
	for (Node& node : nodes)
	{
		node.translate.x = -node.translate.x;

		node.rotate.x = -node.rotate.x;
		node.rotate.w = -node.rotate.w;
	}

	for (Mesh& mesh : meshes)
	{
		for (Vertex& vertex : mesh.vertices)
		{
			vertex.position.x = -vertex.position.x;
		}
		for (DirectX::XMFLOAT4X4& offsetTransform : mesh.offsetTransforms)
		{
			DirectX::XMFLOAT4X4 tempTransform = offsetTransform;
			DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&offsetTransform);
			DirectX::XMVECTOR X = DirectX::XMVector3Length(M.r[0]);
			DirectX::XMVECTOR Y = DirectX::XMVector3Length(M.r[1]);
			DirectX::XMVECTOR Z = DirectX::XMVector3Length(M.r[2]);
			M.r[0] = DirectX::XMVector3Normalize(M.r[0]);
			M.r[1] = DirectX::XMVector3Normalize(M.r[1]);
			M.r[2] = DirectX::XMVector3Normalize(M.r[2]);
			DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationMatrix(M);
			DirectX::XMFLOAT4 q;
			DirectX::XMStoreFloat4(&q, Q);
			q.x = -q.x;
			q.w = -q.w;
			Q = DirectX::XMLoadFloat4(&q);
			M = DirectX::XMMatrixRotationQuaternion(Q);
			M.r[0] = DirectX::XMVectorMultiply(M.r[0], X);
			M.r[1] = DirectX::XMVectorMultiply(M.r[1], Y);
			M.r[2] = DirectX::XMVectorMultiply(M.r[2], Z);
			DirectX::XMStoreFloat4x4(&offsetTransform, M);

		//	offsetTransform._12 = tempTransform._21;
		//	offsetTransform._13 = tempTransform._31;
		//	offsetTransform._21 = tempTransform._12;
		//	offsetTransform._23 = tempTransform._32;
		//	offsetTransform._31 = tempTransform._13;
		//	offsetTransform._32 = tempTransform._23;
			offsetTransform._41 = -tempTransform._41;
			offsetTransform._42 = tempTransform._42;
			offsetTransform._43 = tempTransform._43;
		}
	}
#endif
}

// ノードパス取得
std::string FbxModelResource::GetNodePath(FbxNode* fbxNode) const
{
	std::string parentNodeName;

	FbxNode* fbxParentNode = fbxNode->GetParent();
	if (fbxParentNode != nullptr)
	{
		parentNodeName = GetNodePath(fbxParentNode);
		return parentNodeName + "/" + fbxNode->GetName();
	}

	return fbxNode->GetName();
}

// ノードID取得
ModelResource::NodeId FbxModelResource::GetNodeId(FbxNode* fbxNode)
{
	return reinterpret_cast<NodeId>(fbxNode);
}

// マテリアルインデックスを取得する
int FbxModelResource::FindMaterialIndex(FbxScene* fbxScene, const FbxSurfaceMaterial* fbxSurfaceMaterial)
{
	int fbxMaterialCount = fbxScene->GetMaterialCount();

	for (int i = 0; i < fbxMaterialCount; ++i)
	{
		if (fbxScene->GetMaterial(i) == fbxSurfaceMaterial)
		{
			return i;
		}
	}
	return -1;
}

// 無視するモーションノード
void FbxModelResource::SetupIgnoreMotionNode(const char* ignoreRootMotionNodeName)
{
	// 無視するルートモーションを検索
	if (ignoreRootMotionNodeName != nullptr)
	{
		rootMotionNodeIndex_ = -1;
		for (size_t i = 0; i < nodes_.size(); ++i)
		{
			if (nodes_.at(i).name == ignoreRootMotionNodeName)
			{
				rootMotionNodeIndex_ = static_cast<int>(i);
				break;
			}
		}
	}
}

// 移動値を右手座標系から左手座標系へ変換する
void FbxModelResource::ConvertTranslationFromRHtoLH(DirectX::XMFLOAT3& translate)
{
	translate.x = -translate.x;
}

// 回転値を右手座標系から左手座標系へ変換する
void FbxModelResource::ConvertRotationFromRHtoLH(DirectX::XMFLOAT4& rotate)
{
	rotate.x = -rotate.x;
	rotate.w = -rotate.w;
}

// 行列値を右手座標系から左手座標系へ変換する
void FbxModelResource::ConvertMatrixFromRHtoLH(DirectX::XMFLOAT4X4& matrix)
{
#if 0
	DirectX::XMFLOAT4X4 temp = matrix;

	DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&matrix);

	// スケール値を取り出す
	DirectX::XMVECTOR SX = DirectX::XMVector3Length(M.r[0]);
	DirectX::XMVECTOR SY = DirectX::XMVector3Length(M.r[1]);
	DirectX::XMVECTOR SZ = DirectX::XMVector3Length(M.r[2]);
	// スケール値のない行列に変換
	M.r[0] = DirectX::XMVector3Normalize(M.r[0]);
	M.r[1] = DirectX::XMVector3Normalize(M.r[1]);
	M.r[2] = DirectX::XMVector3Normalize(M.r[2]);
	// 回転行列をクォータニオンへ変換
	DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationMatrix(M);
	DirectX::XMFLOAT4 q;
	DirectX::XMStoreFloat4(&q, Q);
	// クォータニオンを左手座標系へ変換
	q.x = -q.x;
	q.w = -q.w;
	// クォータニオンを回転行列へ変換
	Q = DirectX::XMLoadFloat4(&q);
	M = DirectX::XMMatrixRotationQuaternion(Q);
	// スケール値を戻す
	M.r[0] = DirectX::XMVectorMultiply(M.r[0], SX);
	M.r[1] = DirectX::XMVectorMultiply(M.r[1], SY);
	M.r[2] = DirectX::XMVectorMultiply(M.r[2], SZ);
	// 回転＆スケール行列を取り出す
	DirectX::XMStoreFloat4x4(&matrix, M);
	// 移動値を左手座標系へ変換しつつ設定
	matrix._41 = -temp._41;
	matrix._42 = temp._42;
	matrix._43 = temp._43;
#else
	matrix._12 = -matrix._12;
	matrix._13 = -matrix._13;
	matrix._21 = -matrix._21;
	matrix._31 = -matrix._31;
	matrix._41 = -matrix._41;
#endif
}

// インデックスバッファを右手座標系から左手座標系へ変換する
void FbxModelResource::ConvertIndexBufferFromRHtoLH(std::vector<UINT>& indices)
{
	size_t size = indices.size();
	UINT* p = indices.data();
	for (size_t i = 0; i < size; i += 3)
	{
		UINT temp = p[1];
		p[1] = p[2];
		p[2] = temp;

		p += 3;
	}
}
