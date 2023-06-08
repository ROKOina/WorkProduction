#pragma once

#include <fbxsdk.h>
#include "ModelResource.h"

class FbxModelResource : public ModelResource
{
public:
	FbxModelResource() {}
	~FbxModelResource() override {}

	// �ǂݍ���
	void Load(ID3D11Device* device, const char* filename, const char* ignoreRootMotionNodeName = nullptr);

	// �G�N�X�|�[�g
	void Export(const char* filename);

	// �A�j���[�V�����ǉ��ǂݍ���
	void AddAnimation(const char* filename);

private:
	// �m�[�h�f�[�^���\�z
	void LoadNodes(FbxNode* fbxNode, int parentNodeIndex);
	void LoadNode(FbxNode* fbxNode, int parentNodeIndex);

	// ���b�V���f�[�^��ǂݍ���
	void LoadMeshes(ID3D11Device* device, FbxNode* fbxNode);
	void LoadMesh(ID3D11Device* device, FbxNode* fbxNode, FbxMesh* fbxMesh);

	// �}�e���A���f�[�^��ǂݍ���
	void LoadMaterials(ID3D11Device* device, const char* dirname, FbxScene* fbxScene);
	void LoadMaterial(ID3D11Device* device, const char* dirname, FbxSurfaceMaterial* fbxSurfaceMaterial);

	// �A�j���[�V�����f�[�^��ǂݍ���
	void LoadAnimations(FbxScene* fbxScene, const char* name, bool append);

	// �E����W�n���獶����W�n�֕ϊ�
	void ConvertAxisSystemFromRHtoLH();

	// �m�[�h�p�X�擾
	std::string GetNodePath(FbxNode* fbxNode) const;

	// �m�[�hID�擾
	NodeId GetNodeId(FbxNode* fbxNode);

	// �}�e���A���C���f�b�N�X���擾����
	int FindMaterialIndex(FbxScene* fbxScene, const FbxSurfaceMaterial* fbxSurfaceMaterial);

	// �������郂�[�V�����m�[�h
	void SetupIgnoreMotionNode(const char* ignoreRootMotionNodeName);

	// �ړ��l���E����W�n���獶����W�n�֕ϊ�����
	void ConvertTranslationFromRHtoLH(DirectX::XMFLOAT3& translate);

	// ��]�l���E����W�n���獶����W�n�֕ϊ�����
	void ConvertRotationFromRHtoLH(DirectX::XMFLOAT4& rotate);

	// �s��l���E����W�n���獶����W�n�֕ϊ�����
	void ConvertMatrixFromRHtoLH(DirectX::XMFLOAT4X4& matrix);

	// �C���f�b�N�X�o�b�t�@���E����W�n���獶����W�n�֕ϊ�����
	void ConvertIndexBufferFromRHtoLH(std::vector<UINT>& indices);

private:
	int						rootMotionNodeIndex = -1;
};
