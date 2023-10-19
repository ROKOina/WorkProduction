#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>

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

	int distnationNode = 0;//接続先ノードID
	int originNode = 0;//元ノードID

	//	bool checkedFg = false;//使用したFG
	float cost = 1.0f;  //コスト 


	//初期化
	void initSub(int node, int& buf1);//初期化の関数
};


struct NodePath 
{
	//コンストラクタ
	NodePath() {
		for (int i = 0; i < NodePath::EDGE_NO; i++) {
			auto o = std::make_shared<EdgePath>();
			edge.push_back(o);
		}
	}

	static const int EDGE_NO = 8;

	int NodeID = 0;
	//コスト
	float cost = 1.0f;  //コスト 
	float costFromStart = 100.0f;  //スタートからのコスト
	bool seachFg = false;//サーチ済のFG
	//座標
	DirectX::XMFLOAT3 pos;
	//このノードが所有するエッジ情報
	std::vector<std::shared_ptr<EdgePath>> edge;

};


class SeachGraph
{
private:
	SeachGraph(){}
	~SeachGraph(){}

public:
	// 唯一のインスタンス取得
	static SeachGraph& Instance()
	{
		static SeachGraph instance;
		return instance;
	}



	//enum P {
	//	//NON = 0,  /*NULL*/
	//	GROUND,  /*地面*/
	//	START,  /*障害物の位置*/
	//	GOAL,  /*障害物の位置*/
	//	OBSTCLE,  /*障害物の位置*/
	//	END
	//};

	enum ObjNode
	{
		Player = 10,
		PlayerMawari,
		Enemy,
	};

	//更新
	void UpdatePath();
	void RenderPath();

	//初期化
	void InitSub(int _sizeX, int _sizeZ, float _centerX, float _centerZ, float _size, float  _thresholdY, std::shared_ptr<GameObject> player);
	bool setSub(int node, int buf1, int eNo);
	void makeMap(int _x, int _z, float _size);

	//経路探査
	bool	DijkstraSseach();
	std::shared_ptr<EdgePath>  seachMinCostEdge(std::vector<std::shared_ptr<EdgePath>>& FNR, const std::shared_ptr<EdgePath> nowEdge);

	//プレイヤーの周りに配置するための道順を返す
	std::vector<DirectX::XMFLOAT3> SearchEnemySetPos(DirectX::XMFLOAT3 enemyPos, DirectX::XMFLOAT3 goalPos, DIRECTION dirSet = NONE_DIR);

	//ワールド位置からnodeVecゲット
	std::shared_ptr<NodePath> GetNodeVecFromWorldPos(DirectX::XMFLOAT3 pos);

	std::vector<std::shared_ptr<NodePath>> nodeVec;//全ノードを保持するベクター
	int startId = 0;//サーチのスタート位置
	int goalId = 0;//サーチの終了位置
	float thresholdY = 0;//高さの閾値
	int   cellNo = 0;
	int cellSizeX = 0;  /*ダイクストラの升目*/
	int cellSizeZ = 0;  /*マス目数*/

	float meshSize = 0;  /*升目の幅*/
	float meshOffsetX = 0;  /*オフセット*/
	float meshOffsetZ = 0;  /*オフセット*/
	float meshCenterX = 0;
	float meshCenterZ = 0;



	//答えルートのエッジを記憶するワーク
	std::vector<int>  m_Answer;
	std::shared_ptr<EdgePath> nowEdge;

	std::vector<int>	costMap;


	//プレイヤー保存
	std::weak_ptr<GameObject> player_;

	//デバッグ表示
	bool debugEnabled_ = true;
};
