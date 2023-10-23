#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <mutex>

class GameObject;

enum DIRECTION
{
	UP,
	UP_R,
	RIGHT,
	DOWN_R,
	DOWN,
	DOWN_L,
	LEFT,
	UP_L,

	NONE_DIR = -1,
};

struct EdgePath 
{

	int distnationNode = 0;//�ڑ���m�[�hID
	int originNode = 0;//���m�[�hID

	//	bool checkedFg = false;//�g�p����FG
	float cost = 1.0f;  //�R�X�g 


	//������
	void initSub(int node, int& buf1);//�������̊֐�
};


struct NodePath 
{
	//�R���X�g���N�^
	NodePath() {
		for (int i = 0; i < NodePath::EDGE_NO; i++) {
			auto o = std::make_shared<EdgePath>();
			edge.push_back(o);
		}
	}

	static const int EDGE_NO = 8;

	int NodeID = 0;
	//�R�X�g
	float cost = 1.0f;  //�R�X�g 
	float costFromStart = 100.0f;  //�X�^�[�g����̃R�X�g
	bool seachFg = false;//�T�[�`�ς�FG
	//���W
	DirectX::XMFLOAT3 pos;
	//���̃m�[�h�����L����G�b�W���
	std::vector<std::shared_ptr<EdgePath>> edge;

};


class SeachGraph
{
private:
	SeachGraph(){}
	~SeachGraph(){}

public:
	// �B��̃C���X�^���X�擾
	static SeachGraph& Instance()
	{
		static SeachGraph instance;
		return instance;
	}



	//enum P {
	//	//NON = 0,  /*NULL*/
	//	GROUND,  /*�n��*/
	//	START,  /*��Q���̈ʒu*/
	//	GOAL,  /*��Q���̈ʒu*/
	//	OBSTCLE,  /*��Q���̈ʒu*/
	//	END
	//};

	enum ObjNode
	{
		Player = 10,
		PlayerMawari,
		Enemy,
		EnemyMawari,
	};

	//�X�V
	void UpdatePath();
	void RenderPath();

	//������
	void InitSub(int _sizeX, int _sizeZ, float _centerX, float _centerZ, float _size, float  _thresholdY, std::shared_ptr<GameObject> player);
	bool setSub(int node, int buf1, int eNo);
	void makeMap(int _x, int _z, float _size);

	//�o�H�T��
	bool	DijkstraSseach();
	std::shared_ptr<EdgePath>  seachMinCostEdge(std::vector<std::shared_ptr<EdgePath>>& FNR, const std::shared_ptr<EdgePath> nowEdge);

	//�v���C���[�̎���ɔz�u���邽�߂̓�����Ԃ�
	std::vector<DirectX::XMFLOAT3> SearchEnemySetPos(DirectX::XMFLOAT3 enemyPos, DirectX::XMFLOAT3 goalPos, DIRECTION dirSet = NONE_DIR);

	//���[���h�ʒu����nodeVec�Q�b�g
	std::shared_ptr<NodePath> GetNodeVecFromWorldPos(DirectX::XMFLOAT3 pos);

	std::vector<std::shared_ptr<NodePath>> nodeVec_;//�S�m�[�h��ێ�����x�N�^�[
	int startId_ = 0;//�T�[�`�̃X�^�[�g�ʒu
	int goalId_ = 0;//�T�[�`�̏I���ʒu
	float thresholdY_ = 0;//������臒l
	int   cellNo_ = 0;
	int cellSizeX_ = 0;  /*�_�C�N�X�g���̏���*/
	int cellSizeZ_ = 0;  /*�}�X�ڐ�*/

	float meshSize_ = 0;  /*���ڂ̕�*/
	float meshOffsetX_ = 0;  /*�I�t�Z�b�g*/
	float meshOffsetZ_ = 0;  /*�I�t�Z�b�g*/
	float meshCenterX_ = 0;
	float meshCenterZ_ = 0;



	//�������[�g�̃G�b�W���L�����郏�[�N
	std::vector<int>  m_Answer;
	std::shared_ptr<EdgePath> nowEdge;

	std::vector<int>	costMap;


	//�v���C���[�ۑ�
	std::weak_ptr<GameObject> player_;

	//�f�o�b�O�\��
	bool debugEnabled_ = false;

	std::mutex mutex_;
};
