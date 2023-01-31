#include <fstream>
#include <algorithm>
#include "Encoding.h"
#include "Misc.h"
#include "Logger.h"
#include "FbxModelResource.h"
#include <filesystem>


// FbxDouble2 �� XMFLOAT2
inline DirectX::XMFLOAT2 FbxDouble2ToFloat2(const FbxDouble2& fbxValue)
{
	return DirectX::XMFLOAT2(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1])
	);
}

// FbxDouble3 �� XMFLOAT3
inline DirectX::XMFLOAT3 FbxDouble3ToFloat3(const FbxDouble3& fbxValue)
{
	return DirectX::XMFLOAT3(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1]),
		static_cast<float>(fbxValue[2])
	);
}

// FbxDouble4 �� XMFLOAT3
inline DirectX::XMFLOAT3 FbxDouble4ToFloat3(const FbxDouble4& fbxValue)
{
	return DirectX::XMFLOAT3(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1]),
		static_cast<float>(fbxValue[2])
	);
}

// FbxDouble4 �� XMFLOAT4
inline DirectX::XMFLOAT4 FbxDouble4ToFloat4(const FbxDouble4& fbxValue)
{
	return DirectX::XMFLOAT4(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1]),
		static_cast<float>(fbxValue[2]),
		static_cast<float>(fbxValue[3])
	);
}

// FbxDouble4 �� XMFLOAT4
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


// �ǂݍ���
void FbxModelResource::Load(ID3D11Device* device, const char* filename, const char* ignoreRootMotionNodeName)
{		
	// �f�B���N�g���p�X�擾
	char drive[32], dir[256], dirname[256];
	::_splitpath_s(filename, drive, sizeof(drive), dir, sizeof(dir), nullptr, 0, nullptr, 0);
	::_makepath_s(dirname, sizeof(dirname), drive, dir, nullptr, nullptr);

	//�ۑ��f�[�^����΂���Ń��[�h
	std::filesystem::path cereal_filename(filename);
	cereal_filename.replace_extension("cereal");
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
			// FBX�̃t�@�C���p�X��UTF-8�ɂ���K�v������
			char fbxFilename[256];
			Encoding::StringToUTF8(filename, fbxFilename, sizeof(fbxFilename));

			FbxManager* fbxManager = FbxManager::Create();

			// FBX�ɑ΂�����o�͂��`����
			FbxIOSettings* fbxIOS = FbxIOSettings::Create(fbxManager, IOSROOT);	// ���ʂȗ��R���Ȃ�����IOSROOT���w��
			fbxManager->SetIOSettings(fbxIOS);

			// �C���|�[�^�𐶐�
			FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
			bool result = fbxImporter->Initialize(fbxFilename, -1, fbxManager->GetIOSettings());	// -1�Ńt�@�C���t�H�[�}�b�g��������
			_ASSERT_EXPR_A(result, "FbxImporter::Initialize() : Failed!!\n");

			// Scene�I�u�W�F�N�g��FBX�t�@�C�����̏��𗬂�����
			FbxScene* fbxScene = FbxScene::Create(fbxManager, "scene");
			fbxImporter->Import(fbxScene);
			fbxImporter->Destroy();	// �V�[���𗬂����񂾂�Importer�͉������OK

									// �W�I���g�����O�p�`�����Ă���
			FbxGeometryConverter fbxGeometryConverter(fbxManager);
			fbxGeometryConverter.Triangulate(fbxScene, true);
			fbxGeometryConverter.RemoveBadPolygonsFromMeshes(fbxScene);

			// ����Z-Up�̏ꍇ��Y-Up�ɕϊ�����
			FbxAxisSystem fbx_scene_axis_system = fbxScene->GetGlobalSettings().GetAxisSystem();
			if (fbx_scene_axis_system == FbxAxisSystem::eMayaZUp)
			{
				FbxAxisSystem::MayaYUp.ConvertScene(fbxScene);
			}

			// ���f���ǂݍ���
			std::vector<FbxNode*> fbxNodes;
			FbxNode* fbxRootNode = fbxScene->GetRootNode();
			LoadMaterials(device, dirname, fbxScene);
			LoadNodes(fbxRootNode, -1);
			LoadMeshes(device, fbxRootNode);

			// �������郂�[�V�����m�[�h������
			SetupIgnoreMotionNode(ignoreRootMotionNodeName);

			// �A�j���[�V�����ǂݍ���
			char name[256];
			::_splitpath_s(filename, nullptr, 0, nullptr, 0, name, 256, nullptr, 0);

			LoadAnimations(fbxScene, nullptr, false);

			// �}�l�[�W�����
			fbxManager->Destroy();		// �֘A���邷�ׂẴI�u�W�F�N�g����������
		}
		else
		{
			// �f�V���A���C�Y
			Deserialize(filename);

			// �������郂�[�V�����m�[�h������
			SetupIgnoreMotionNode(ignoreRootMotionNodeName);
		}

		Export(cereal_filenameString.c_str());
	}
	// ���f���\�z
	BuildModel(device, dirname);
}

// �G�N�X�|�[�g
void FbxModelResource::Export(const char* filename)
{
	Serialize(filename);
}

// �A�j���[�V�����ǉ�
void FbxModelResource::AddAnimation(const char* filename)
{
	// FBX�̃t�@�C���p�X��UTF-8�ɂ���K�v������
	char fbxFilename[256];
	Encoding::StringToUTF8(filename, fbxFilename, sizeof(fbxFilename));

	FbxManager* fbxManager = FbxManager::Create();

	// FBX�ɑ΂�����o�͂��`����
	FbxIOSettings* fbxIOS = FbxIOSettings::Create(fbxManager, IOSROOT);	// ���ʂȗ��R���Ȃ�����IOSROOT���w��
	fbxManager->SetIOSettings(fbxIOS);

	// �C���|�[�^�𐶐�
	FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
	bool result = fbxImporter->Initialize(fbxFilename, -1, fbxManager->GetIOSettings());	// -1�Ńt�@�C���t�H�[�}�b�g��������
	_ASSERT_EXPR_A(result, "FbxImporter::Initialize() : Failed!!\n");

	// Scene�I�u�W�F�N�g��FBX�t�@�C�����̏��𗬂�����
	FbxScene* fbxScene = FbxScene::Create(fbxManager, "scene");
	fbxImporter->Import(fbxScene);
	fbxImporter->Destroy();	// �V�[���𗬂����񂾂�Importer�͉������OK

	// ����Z-Up�̏ꍇ��Y-Up�ɕϊ�����
	FbxAxisSystem fbx_scene_axis_system = fbxScene->GetGlobalSettings().GetAxisSystem();
	if (fbx_scene_axis_system == FbxAxisSystem::eMayaZUp)
	{
		FbxAxisSystem::MayaYUp.ConvertScene(fbxScene);
	}

	// �A�j���[�V�����ǂݍ���
	char name[256];
	::_splitpath_s(filename, nullptr, 0, nullptr, 0, name, 256, nullptr, 0);

	char utf8[256];
	Encoding::StringToUTF8(name, utf8, sizeof(utf8));

	LoadAnimations(fbxScene, utf8, true);

	// �}�l�[�W�����
	fbxManager->Destroy();		// �֘A���邷�ׂẴI�u�W�F�N�g����������
}

// FBX�m�[�h���ċA�I�ɒH���ăf�[�^��ǂݍ���
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

	// �ċA�I�Ɏq�m�[�h����������
	parentNodeIndex = static_cast<int>(nodes.size() - 1);
	for (int i = 0; i < fbxNode->GetChildCount(); ++i)
	{
		LoadNodes(fbxNode->GetChild(i), parentNodeIndex);
	}
}

// FBX�m�[�h����m�[�h�f�[�^��ǂݍ���
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

	// ���W���ϊ�
	ConvertTranslationFromRHtoLH(node.translate);
	ConvertRotationFromRHtoLH(node.rotate);

	nodes.emplace_back(node);
}

// FBX�m�[�h���ċA�I�ɒH���ă��b�V���f�[�^��ǂݍ���
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

	// �ċA�I�Ɏq�m�[�h����������
	for (int i = 0; i < fbxNode->GetChildCount(); ++i)
	{
		LoadMeshes(device, fbxNode->GetChild(i));
	}
}

// FBX���b�V�����烁�b�V���f�[�^��ǂݍ���
void FbxModelResource::LoadMesh(ID3D11Device* device, FbxNode* fbxNode, FbxMesh* fbxMesh)
{
	int fbxControlPointsCount = fbxMesh->GetControlPointsCount();
	//int fbxPolygonVertexCount = fbxMesh->GetPolygonVertexCount();
	//const int* fbxPolygonVertices = fbxMesh->GetPolygonVertices();

	int fbxMaterialCount = fbxNode->GetMaterialCount();
	int fbxPolygonCount = fbxMesh->GetPolygonCount();
	std::string& fbxNodePath = GetNodePath(fbxNode);

	meshes.emplace_back(Mesh());
	Mesh& mesh = meshes.back();
	NodeId nodeId = GetNodeId(fbxNode);
	mesh.nodeIndex = FindNodeIndex(nodeId);
	mesh.subsets.resize(fbxMaterialCount > 0 ? fbxMaterialCount : 1);
	mesh.boundsMin.x = mesh.boundsMin.y = mesh.boundsMin.z = FLT_MAX;
	mesh.boundsMax.x = mesh.boundsMax.y = mesh.boundsMax.z = -FLT_MAX;

	// �T�u�Z�b�g�̃}�e���A���ݒ�
	for (int fbxMaterialIndex = 0; fbxMaterialIndex < fbxMaterialCount; ++fbxMaterialIndex)
	{
		const FbxSurfaceMaterial* fbxSurfaceMaterial = fbxNode->GetMaterial(fbxMaterialIndex);

		Subset& subset = mesh.subsets.at(fbxMaterialIndex);
		subset.materialIndex = FindMaterialIndex(fbxNode->GetScene(), fbxSurfaceMaterial);
	}

	// �T�u�Z�b�g�̒��_�C���f�b�N�X�͈͐ݒ�
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

	// ���_�e���̓f�[�^
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
	// ���_�e���̓f�[�^�𒊏o����
	std::vector<BoneInfluence> boneInfluences;
	{
		boneInfluences.resize(fbxControlPointsCount);

		// �X�L�j���O�ɕK�v�ȏ����擾����
		int fbxDeformerCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
		for (int fbxDeformerIndex = 0; fbxDeformerIndex < fbxDeformerCount; ++fbxDeformerIndex)
		{
			FbxSkin* fbxSkin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(fbxDeformerIndex, FbxDeformer::eSkin));

			int fbxClusterCount = fbxSkin->GetClusterCount();
			for (int fbxClusterIndex = 0; fbxClusterIndex < fbxClusterCount; ++fbxClusterIndex)
			{
				FbxCluster* fbxCluster = fbxSkin->GetCluster(fbxClusterIndex);

				// ���_�e���̓f�[�^�𒊏o����
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

				// ���b�V����Ԃ���{�[����Ԃւ̕ϊ��s��̌v�Z������
				{
					// ���b�V����Ԃ���O���[�o����Ԃւ̕ϊ��s��
					FbxAMatrix fbxMeshToGlobalSpaceTransform;
					fbxCluster->GetTransformMatrix(fbxMeshToGlobalSpaceTransform);

					// �{�[����Ԃ���O���[�o����Ԃւ̕ϊ��s��
					FbxAMatrix fbxBoneToGlobalSpaceTransform;
					fbxCluster->GetTransformLinkMatrix(fbxBoneToGlobalSpaceTransform);

					// ���b�V����Ԃ���{�[����Ԃւ̕ϊ��s��
					FbxAMatrix fbxMeshToBoneTransform = fbxBoneToGlobalSpaceTransform.Inverse() * fbxMeshToGlobalSpaceTransform;

					// ���W�n�ϊ�
					DirectX::XMFLOAT4X4 offsetTransform = FbxAMatrixToFloat4x4(fbxMeshToBoneTransform);
					ConvertMatrixFromRHtoLH(offsetTransform);

					mesh.offsetTransforms.emplace_back(offsetTransform);

					// �e������{�[���C���f�b�N�X�𖼑O�Ō�������
					NodeId nodeId = GetNodeId(fbxCluster->GetLink());
					int nodeIndex = FindNodeIndex(nodeId);
					mesh.nodeIndices.emplace_back(nodeIndex);
				}
			}
		}
	}

	// �W�I���g���ϊ��s��
	FbxAMatrix fbxGeometricTransform(
		fbxNode->GetGeometricTranslation(FbxNode::eSourcePivot),
		fbxNode->GetGeometricRotation(FbxNode::eSourcePivot),
		fbxNode->GetGeometricScaling(FbxNode::eSourcePivot)
		);
	DirectX::XMFLOAT4X4 geometricTransform = FbxAMatrixToFloat4x4(fbxGeometricTransform);
	DirectX::XMMATRIX GM = DirectX::XMLoadFloat4x4(&geometricTransform);
		
	// UV�Z�b�g��
	FbxStringList fbxUVSetNames;
	fbxMesh->GetUVSetNames(fbxUVSetNames);

	// ���_�f�[�^
	mesh.vertices.resize(fbxPolygonCount * 3);
	mesh.indices.resize(fbxPolygonCount * 3);

	int vertexCount = 0;
	const FbxVector4* fbxControlPoints = fbxMesh->GetControlPoints();
	for (int fbxPolygonIndex = 0; fbxPolygonIndex < fbxPolygonCount; ++fbxPolygonIndex)
	{
		// �|���S���ɓK�p����Ă���}�e���A���C���f�b�N�X���擾����
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

			// ���W�n�ϊ�
			ConvertTranslationFromRHtoLH(vertex.position);
			ConvertTranslationFromRHtoLH(vertex.normal);

			// ���_���C���f�b�N�X�f�[�^�ݒ�
			mesh.indices.at(indexOffset + fbxVertexIndex) = vertexCount;
			mesh.vertices.at(vertexCount) = vertex;
			vertexCount++;

			// �o�E���f�B���O�{�b�N�X
			mesh.boundsMin.x = std::fminf(mesh.boundsMin.x, vertex.position.x);
			mesh.boundsMin.y = std::fminf(mesh.boundsMin.y, vertex.position.y);
			mesh.boundsMin.z = std::fminf(mesh.boundsMin.z, vertex.position.z);
			mesh.boundsMax.x = std::fmaxf(mesh.boundsMax.x, vertex.position.x);
			mesh.boundsMax.y = std::fmaxf(mesh.boundsMax.y, vertex.position.y);
			mesh.boundsMax.z = std::fmaxf(mesh.boundsMax.z, vertex.position.z);
		}

		subset.indexCount += 3;
	}

	// �^���W�F���g
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

			// ���W�n�ϊ�
			ConvertTranslationFromRHtoLH(mesh.vertices[i].tangent);
		}
	}

	// ���_�J���[
	int fbxElementVertexColorLayerCount = fbxMesh->GetElementVertexColorCount();
	if (fbxElementVertexColorLayerCount > 0)
	{
		// ���_�J���[���C���[�擾
		FbxGeometryElementVertexColor* fbxElementVertexColor = fbxMesh->GetElementVertexColor(0);

		// �ۑ��`���̎擾
		FbxGeometryElement::EMappingMode fbxMappingMode = fbxElementVertexColor->GetMappingMode();
		FbxGeometryElement::EReferenceMode fbxReferenceMode = fbxElementVertexColor->GetReferenceMode();

		// �|���S�����_�ɑ΂���C���f�b�N�X�Q�ƌ`���̂ݑΉ�
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

#if 0	// �~���[�����O�̃��f���f�[�^�ł��܂������Ȃ��Ƃ�������
	// ���_�œK��
	int currentVertexCount = 0;
	vertexCount = static_cast<int>(mesh.vertices.size());
	for (int i = 0; i < vertexCount; ++i)
	{
		int sameIndex = -1;

		Vertex& a = mesh.vertices[i];
		//	���꒸�_����
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

		//	�V�K���_
		if (index == -1)
		{
			mesh.vertices[currentVertexCount] = a;
			index = currentVertexCount;
			currentVertexCount++;
		}
		//	�C���f�b�N�X�X�V
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

	// ���W�n�ϊ�
	ConvertIndexBufferFromRHtoLH(mesh.indices);
}

// FBX�V�[������FBX�}�e���A������}�e���A���f�[�^��ǂݍ���
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
		materials.emplace_back(material);
	}
}

// FBX�}�e���A������}�e���A���f�[�^��ǂݍ���
void FbxModelResource::LoadMaterial(ID3D11Device* device, const char* dirname, FbxSurfaceMaterial* fbxSurfaceMaterial)
{
	bool ret = false;

	Material material;

	material.name = fbxSurfaceMaterial->GetName();

	// �f�B�t���[�Y�J���[
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
	// �f�B�t���[�Y�e�N�X�`��
	if (fbxDiffuseProperty.IsValid())
	{
		int fbxTextureCount = fbxDiffuseProperty.GetSrcObjectCount<FbxFileTexture>();
		if (fbxTextureCount > 0)
		{
			FbxFileTexture* fbxTexture = fbxDiffuseProperty.GetSrcObject<FbxFileTexture>();

			const char* relativeFileName = fbxTexture->GetRelativeFileName();

			// �e�N�X�`���t�@�C�������݂��Ă��邩�m�F
			char filename[256];
			::_makepath_s(filename, sizeof(filename), nullptr, dirname, relativeFileName, nullptr);
			std::ifstream istream(filename, std::ios::binary);
			if (istream.is_open())
			{
				// ���������̂ł��̂܂ܐݒ肷��
				material.textureFilename = relativeFileName;
			}
			else
			{
				// ������Ȃ������ꍇ�̓��f���Ɠ��f�B���N�g���ɂ���Γǂݍ��߂�悤�ɂ���
				char fname[256], ext[32];
				::_splitpath_s(relativeFileName, nullptr, 0, nullptr, 0, fname, sizeof(fname), ext, sizeof(ext));
				::_makepath_s(filename, sizeof(filename), nullptr, nullptr, fname, ext);
				material.textureFilename = filename;
			}
		}
	}

	materials.emplace_back(material);
}

// �A�j���[�V�����f�[�^��ǂݍ���
void FbxModelResource::LoadAnimations(FbxScene* fbxScene, const char* name, bool append)
{
	// ���ׂẴA�j���[�V���������擾
	FbxArray<FbxString*> fbxAnimStackNames;
	fbxScene->FillAnimStackNameArray(fbxAnimStackNames);

	int fbxAnimationCount = fbxAnimStackNames.Size();
	for (int fbxAnimationIndex = 0; fbxAnimationIndex < fbxAnimationCount; ++fbxAnimationIndex)
	{
		//if (0 != ::strncmp(fbxAnimStackNames.GetAt(fbxAnimationIndex)->Buffer(), u8"��{", strlen(u8"��{"))) continue;
		
		animations.emplace_back(Animation());
		Animation& animation = animations.back();

		// �A�j���[�V�����f�[�^�̃T���v�����O�ݒ�
		FbxTime::EMode fbxTimeMode = fbxScene->GetGlobalSettings().GetTimeMode();
		FbxTime fbxFrameTime;
		fbxFrameTime.SetTime(0, 0, 0, 1, 0, fbxTimeMode);

		float samplingRate = static_cast<float>(fbxFrameTime.GetFrameRate(fbxTimeMode));
		float samplingTime = 1.0f / samplingRate;

		FbxString* fbxAnimStackName = fbxAnimStackNames.GetAt(fbxAnimationIndex);
		FbxAnimStack* fbxAnimStack = fbxScene->FindMember<FbxAnimStack>(fbxAnimStackName->Buffer());

		// �Đ�����A�j���[�V�������w�肷��B
		fbxScene->SetCurrentAnimationStack(fbxAnimStack);

		// �A�j���[�V�����̍Đ��J�n���ԂƍĐ��I�����Ԃ��擾����
		FbxTakeInfo* fbxTakeInfo = fbxScene->GetTakeInfo(fbxAnimStackName->Buffer());
		FbxTime fbxStartTime = fbxTakeInfo->mLocalTimeSpan.GetStart();
		FbxTime fbxEndTime = fbxTakeInfo->mLocalTimeSpan.GetStop();

		// ���o����f�[�^��60�t���[����ŃT���v�����O����
		FbxTime fbxSamplingStep;
		fbxSamplingStep.SetTime(0, 0, 1, 0, 0, fbxTimeMode);
		fbxSamplingStep = static_cast<FbxLongLong>(fbxSamplingStep.Get() * samplingTime);

		int startFrame = static_cast<int>(fbxStartTime.Get() / fbxSamplingStep.Get());
		int endFrame = static_cast<int>(fbxEndTime.Get() / fbxSamplingStep.Get());
		int frameCount = static_cast<int>((fbxEndTime.Get() - fbxStartTime.Get()) / fbxSamplingStep.Get());

		// �A�j���[�V�����̑ΏۂƂȂ�m�[�h��񋓂���
		std::vector<FbxNode*> fbxNodes;
		if (append)
		{
			// �m�[�h�����r���đΏۃm�[�h��񋓂���
			// �����O���d�����Ă���Ǝ��s����ꍇ������
			FbxNode* fbxRootNode = fbxScene->GetRootNode();
			for (Node& node : nodes)
			{
				FbxNode* fbxNode = fbxRootNode->FindChild(node.name.c_str(), true, true);
				fbxNodes.emplace_back(fbxNode);
			}
		}
		else
		{
			// �m�[�h�̊��S�p�X���r���đΏۃm�[�h��񋓂���i�d���j
			// ���K�����f���ƃA�j���[�V������FBX�̃m�[�h�c���[�\������v���Ă���K�v������
			for (Node& node : nodes)
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

		// �A�j���[�V������
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

		// �A�j���[�V�����f�[�^�𒊏o����
		animation.secondsLength = frameCount * samplingTime;
		animation.keyframes.resize(frameCount + 1);

		float seconds = 0.0f;
		Keyframe* keyframe = animation.keyframes.data();
		size_t fbxNodeCount = fbxNodes.size();
		FbxTime fbxCurrentTime = fbxStartTime;
		for (FbxTime fbxCurrentTime = fbxStartTime; fbxCurrentTime < fbxEndTime; fbxCurrentTime += fbxSamplingStep, ++keyframe)
		{
			// �L�[�t���[�����̎p���f�[�^�����o���B
			keyframe->seconds = seconds;
			keyframe->nodeKeys.resize(fbxNodeCount);
			for (size_t fbxNodeIndex = 0; fbxNodeIndex < fbxNodeCount; ++fbxNodeIndex)
			{
				NodeKeyData& keyData = keyframe->nodeKeys.at(fbxNodeIndex);
				FbxNode* fbxNode = fbxNodes.at(fbxNodeIndex);
				if (fbxNode == nullptr)
				{
					// �A�j���[�V�����Ώۂ̃m�[�h���Ȃ������̂Ń_�~�[�f�[�^��ݒ�
					Node& node = nodes.at(fbxNodeIndex);
					keyData.scale = node.scale;
					keyData.rotate = node.rotate;
					keyData.translate = node.translate;
				}
				else if (fbxNodeIndex == rootMotionNodeIndex)
				{
					// ���[�g���[�V�����͖�������
					Node& node = nodes.at(fbxNodeIndex);
					keyData.scale = DirectX::XMFLOAT3(1, 1, 1);
					keyData.rotate = DirectX::XMFLOAT4(0, 0, 0, 1);
					keyData.translate = DirectX::XMFLOAT3(0, 0, 0);
				}
				else
				{
					// �w�莞�Ԃ̃��[�J���s�񂩂�X�P�[���l�A��]�l�A�ړ��l�����o���B
					const FbxAMatrix& fbxLocalTransform = fbxNode->EvaluateLocalTransform(fbxCurrentTime);

					keyData.scale = FbxDouble4ToFloat3(fbxLocalTransform.GetS());
					keyData.rotate = FbxDouble4ToFloat4(fbxLocalTransform.GetQ());
					keyData.translate = FbxDouble4ToFloat3(fbxLocalTransform.GetT());
				}
				// ���W�n�ϊ�
				ConvertTranslationFromRHtoLH(keyData.translate);
				ConvertRotationFromRHtoLH(keyData.rotate);
			}
			seconds += samplingTime;
		}
	}

	// ��n��
	for (int i = 0; i < fbxAnimationCount; i++)
	{
		delete fbxAnimStackNames[i];
	}

	//std::sort(animations.begin(), animations.end(), [](auto& a, auto& b) { return strcmp(a.name.c_str(), b.name.c_str()); });
}

// �E����W�n���獶����W�n�֕ϊ�
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

// �m�[�h�p�X�擾
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

// �m�[�hID�擾
ModelResource::NodeId FbxModelResource::GetNodeId(FbxNode* fbxNode)
{
	return reinterpret_cast<NodeId>(fbxNode);
}

// �}�e���A���C���f�b�N�X���擾����
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

// �������郂�[�V�����m�[�h
void FbxModelResource::SetupIgnoreMotionNode(const char* ignoreRootMotionNodeName)
{
	// �������郋�[�g���[�V����������
	if (ignoreRootMotionNodeName != nullptr)
	{
		rootMotionNodeIndex = -1;
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			if (nodes.at(i).name == ignoreRootMotionNodeName)
			{
				rootMotionNodeIndex = static_cast<int>(i);
				break;
			}
		}
	}
}

// �ړ��l���E����W�n���獶����W�n�֕ϊ�����
void FbxModelResource::ConvertTranslationFromRHtoLH(DirectX::XMFLOAT3& translate)
{
	translate.x = -translate.x;
}

// ��]�l���E����W�n���獶����W�n�֕ϊ�����
void FbxModelResource::ConvertRotationFromRHtoLH(DirectX::XMFLOAT4& rotate)
{
	rotate.x = -rotate.x;
	rotate.w = -rotate.w;
}

// �s��l���E����W�n���獶����W�n�֕ϊ�����
void FbxModelResource::ConvertMatrixFromRHtoLH(DirectX::XMFLOAT4X4& matrix)
{
#if 0
	DirectX::XMFLOAT4X4 temp = matrix;

	DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&matrix);

	// �X�P�[���l�����o��
	DirectX::XMVECTOR SX = DirectX::XMVector3Length(M.r[0]);
	DirectX::XMVECTOR SY = DirectX::XMVector3Length(M.r[1]);
	DirectX::XMVECTOR SZ = DirectX::XMVector3Length(M.r[2]);
	// �X�P�[���l�̂Ȃ��s��ɕϊ�
	M.r[0] = DirectX::XMVector3Normalize(M.r[0]);
	M.r[1] = DirectX::XMVector3Normalize(M.r[1]);
	M.r[2] = DirectX::XMVector3Normalize(M.r[2]);
	// ��]�s����N�H�[�^�j�I���֕ϊ�
	DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationMatrix(M);
	DirectX::XMFLOAT4 q;
	DirectX::XMStoreFloat4(&q, Q);
	// �N�H�[�^�j�I����������W�n�֕ϊ�
	q.x = -q.x;
	q.w = -q.w;
	// �N�H�[�^�j�I������]�s��֕ϊ�
	Q = DirectX::XMLoadFloat4(&q);
	M = DirectX::XMMatrixRotationQuaternion(Q);
	// �X�P�[���l��߂�
	M.r[0] = DirectX::XMVectorMultiply(M.r[0], SX);
	M.r[1] = DirectX::XMVectorMultiply(M.r[1], SY);
	M.r[2] = DirectX::XMVectorMultiply(M.r[2], SZ);
	// ��]���X�P�[���s������o��
	DirectX::XMStoreFloat4x4(&matrix, M);
	// �ړ��l��������W�n�֕ϊ����ݒ�
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

// �C���f�b�N�X�o�b�t�@���E����W�n���獶����W�n�֕ϊ�����
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
