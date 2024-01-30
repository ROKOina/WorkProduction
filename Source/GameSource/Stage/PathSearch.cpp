#include "PathSearch.h"

#include "Graphics/Graphics.h"
#include "Components/TransformCom.h"
#include "GameSource/ScriptComponents/Enemy/EnemyManager.h"
#include "GameSource/ScriptComponents/Enemy/EnemyNearCom.h"

#include <imgui.h>

/// EdgePath ///
void EdgePath::initSub(int node, int& buf1)
{
	buf1 = -1;
	originNode = node;
}



/// SeachGraph ///
void SeachGraph::UpdatePath()
{
	//動的にコストを変えるので最初は0初期化する
	for (auto& c : costMap_)
	{
		c = 0;
	}


	//敵の場所を登録
	for (auto& enemy : EnemyManager::Instance().GetNearEnemies())
	{
		DirectX::XMFLOAT3 ePos = enemy.enemy.lock()->transform_->GetWorldPosition();
		std::shared_ptr<NodePath> enemyNode = GetNodeVecFromWorldPos(ePos);
		costMap_[enemyNode->NodeID] = Enemy;

		if (enemy.enemy.lock()->GetComponent<EnemyNearCom>()->GetIsPathFlag())continue;

		//敵回りを登録
		for (auto& e : enemyNode->edge)
		{
			if (e->distnationNode < 0)continue;
			costMap_[e->distnationNode] = EnemyMawari;
		}
	}

	//プレイヤーの場所の登録
	DirectX::XMFLOAT3 pPos = player_.lock()->transform_->GetWorldPosition();
	std::shared_ptr<NodePath> node = GetNodeVecFromWorldPos(pPos);
	costMap_[node->NodeID] = Player;

	//プレイヤー周りを登録
	for (auto& e : node->edge)
	{
		if (e->distnationNode < 0)continue;
		costMap_[e->distnationNode] = PlayerMawari;
	}
}

void SeachGraph::RenderPath()
{
	//デバッグでブロック表示
	if (debugEnabled_)
	{
		int i = 0;
		for (auto& n : nodeVec_)
		{
			if (i == goalId_)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 0,0,0,1 });
			else if (costMap_[i] == EnemyMawari)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 0,1,0,1 });
			else if (costMap_[i] == PlayerMawari)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 0,1,0,1 });
			else if (costMap_[i] == Enemy)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 0,1,1,1 });
			else if (costMap_[i] == Player)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 1,0,0,1 });
			else
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 1,1,1,1 });
			i++;
		}

		//ゴールまでの道順
		if (goalId_ > 0)
		{
			int startNo = goalId_;
			int endNo = -1;

			while (endNo != startId_)
			{
				endNo = m_Answer_[startNo];

				if (endNo == -1)break;

				Graphics::Instance().GetDebugRenderer()->DrawBox(nodeVec_[endNo]->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 1,0,1,1 });

				startNo = endNo;
			}
		}
	}

	{
		ImGui::SetNextWindowPos(ImVec2(950, 50), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		ImGui::Begin("PathSearch", nullptr, ImGuiWindowFlags_None);

		ImGui::Checkbox("debugEnabled", &debugEnabled_);

		ImGui::End();
	}
}


#define NANAME 1.414f;

void SeachGraph::InitSub(int _sizeX, int _sizeZ, float _centerX, float _centerZ, float _size, float _thresholdY, std::shared_ptr<GameObject> player)
{
	player_ = player;
	thresholdY_ = _thresholdY;//高さの閾値

	cellSizeX_ = _sizeX;  /*ダイクストラの升目*/
	cellSizeZ_ = _sizeZ;
	meshCenterX_ = _centerX;
	meshCenterZ_ = _centerZ;
	meshSize_ = _size;  /*メッシュ単位の幅*/

	meshOffsetX_ = meshSize_ * cellSizeX_ / 2;  /*オフセット*/
	meshOffsetZ_ = meshSize_ * cellSizeZ_ / 2;  /*オフセット*/
	cellNo_ = cellSizeX_ * cellSizeZ_;
	nodeVec_.clear();//地形

	//描画用OBJECTをセット
	for (int i = 0; i < cellNo_; i++) {
		auto piece = std::make_shared<NodePath>();
		piece->NodeID = i;

		float culms = (float)cellSizeX_;
		piece->pos.x = (meshOffsetX_ * -1.0f + (float)(i % cellSizeX_) * meshSize_ + (meshSize_ / 2.0f)) + meshCenterX_;
		piece->pos.z = (meshOffsetZ_ * -1.0f + (float)(i / cellSizeX_) * meshSize_ +
			+(meshSize_ / 2.0f)) + meshCenterZ_;
		nodeVec_.push_back(std::move(piece));

		m_Answer_.push_back(-1);
		costMap_.push_back(0);

	}

	//ナビメッシュを作成(今のところy固定)
	makeMap(cellSizeX_, cellSizeZ_, 1);

	int buf1 = 0;
	for (int z = 0; z < cellSizeZ_; z++) {

		for (int x = 0; x < cellSizeX_; x++) {
			int node = cellSizeX_ * z + x;
			//P_EDGE0,  /*エッジの表示　12時の方向*/
			std::shared_ptr<EdgePath> e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::UP]);
			e->initSub(node, buf1);

			if (
				!(z <= 0)
				) {
				buf1 = node - cellSizeX_;
			}
			setSub(node, buf1, 0);

			/*エッジの表示　1.5時の方向*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::UP_R]);
			e->initSub(node, buf1);
			if (
				!(z <= 0) && !(x >= cellSizeX_ - 1)
				) {
				buf1 = node - cellSizeX_ + 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 1);



			/*エッジの表示　３時の方向*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::RIGHT]);
			e->initSub(node, buf1);
			if (
				!(x >= cellSizeX_ - 1)
				) {
				buf1 = node + 1;
			}
			setSub(node, buf1, 2);

			/*エッジの表示　4.5時の方向*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::DOWN_R]);
			e->initSub(node, buf1);
			if (
				!(z >= cellSizeZ_ - 1) && !(x >= cellSizeX_ - 1)
				) {
				buf1 = node + cellSizeX_ + 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 3);


			/*エッジの表示　6時の方向*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::DOWN]);
			e->initSub(node, buf1);
			if (
				!(z >= cellSizeZ_ - 1)
				) {
				buf1 = node + cellSizeX_;
			}
			setSub(node, buf1, 4);

			/*エッジの表示　7.5時の方向*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::DOWN_L]);
			e->initSub(node, buf1);
			if (
				!(z >= cellSizeZ_ - 1) && !(x <= 0)
				) {
				buf1 = node + cellSizeX_ - 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 5);

			/*エッジの表示　9時の方向*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::LEFT]);
			e->initSub(node, buf1);
			if (
				!(x <= 0)
				) {
				buf1 = node - 1;
			}
			setSub(node, buf1, 6);

			/*エッジの表示　10.5時の方向*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::UP_L]);
			e->initSub(node, buf1);
			if (
				!(z <= 0) &&
				!(x <= 0)
				) {
				buf1 = node - cellSizeX_ - 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 7);
		}

	}

	//動的にコストを変えるので最初は0初期化する
	for (auto& c : costMap_)
	{
		c = 0;
	}

	//プレイヤーの場所のコスト１にする
	DirectX::XMFLOAT3 pPos = player_.lock()->transform_->GetWorldPosition();
	std::shared_ptr<NodePath> node = GetNodeVecFromWorldPos(pPos);
	costMap_[node->NodeID] = Player;

	//プレイヤー周りを２にする
	for (auto& e : node->edge)
	{
		costMap_[e->distnationNode] = PlayerMawari;
	}

	//敵の場所を３にする
	for (auto& enemy : EnemyManager::Instance().GetNearEnemies())
	{
		DirectX::XMFLOAT3 ePos = enemy.enemy.lock()->transform_->GetWorldPosition();
		std::shared_ptr<NodePath> enemyNode = GetNodeVecFromWorldPos(ePos);
		costMap_[enemyNode->NodeID] = Enemy;
	}

}

//高さを比較
bool  SeachGraph::setSub(int node, int buf1, int eNo) {

	std::shared_ptr<EdgePath> e = (nodeVec_[node]->edge[eNo]);
	e->distnationNode = buf1;//ノードのエッジを繋ぐ

	if (buf1 < 0) {
		return false;
	}
	if (costMap_[node] == 1) {//繋いではダメなノードなら
		for (int i = 0; i < nodeVec_[buf1]->EDGE_NO; i++) {
			nodeVec_[node]->edge[i]->distnationNode = -1;
		}
		return false;
	};
	float nY = nodeVec_[node]->pos.y;
	float bY = nodeVec_[buf1]->pos.y;
	if (abs(nY - bY) > thresholdY_) {//段差があれば
		costMap_[node] = 1;
		costMap_[buf1] = 1;
		for (int i = 0; i < nodeVec_[buf1]->EDGE_NO; i++) {
			nodeVec_[buf1]->edge[i]->distnationNode = -1;
			nodeVec_[node]->edge[i]->distnationNode = -1;
		}
		return false;
	}
	return true;
}


void SeachGraph::makeMap(int _x, int _z, float _size)
{
	for (int z1 = 0; z1 < _z; z1++) {
		for (int x1 = 0; x1 < _x; x1++) {

			nodeVec_[x1 + z1 * cellSizeX_]->pos.y = 0.3f;
		}
	}
}


bool SeachGraph::DijkstraSseach()
{
	if (goalId_ < 0)return false;

	//エッジ型データのフロンティアツリーを作成
	std::vector<std::shared_ptr<EdgePath>> FNR;
	//サーチ済みのFGをリセット
	for (int i = 0; i < cellNo_; i++) {
		nodeVec_[i]->seachFg = false;
		nodeVec_[i]->costFromStart = 0.0f;  //スタートからのコスト
		m_Answer_[i] = -1;
	}

	std::shared_ptr<EdgePath> edge = std::make_shared<EdgePath>();
	edge->distnationNode = startId_;
	edge->originNode = startId_;


	//前準備としてダミーエッジデータをセット
	nowEdge_ = edge;
	while (true)
	{
		//サーチしたEdgeの記録
		m_Answer_[nowEdge_->distnationNode] = nowEdge_->originNode;
		if (nowEdge_->distnationNode == goalId_) { return true; }



		//nowEdgeの先のノードを取得する。
		std::shared_ptr<NodePath> node = nodeVec_[nowEdge_->distnationNode];//進み先のノード

		for (int edgeNo = 0; edgeNo < NodePath::EDGE_NO; edgeNo++) {
			std::shared_ptr<EdgePath> e = node->edge[edgeNo];
			if (e->distnationNode >= 0) {//そのエッジは有効である。
				std::shared_ptr<NodePath> nextNode = nodeVec_[e->distnationNode];//進み先のノード
				//進み先のノードまでのコストを計算
				float totalCost = node->costFromStart + e->cost;

				//進み先がプレイヤーの周りならコスト500
				if (costMap_[nextNode->NodeID] == PlayerMawari)
				{
					if (nextNode->NodeID != goalId_)
						totalCost += 500;
				}
				//進み先が敵周りならコスト50
				if (costMap_[nextNode->NodeID] == EnemyMawari)
				{
					if (nextNode->NodeID != goalId_)
						totalCost += 50;
				}

				//進み先のコストがまだ計算されていないか、新しいコストの方が低ければ登録
				if (nextNode->costFromStart == 0 || nextNode->costFromStart > totalCost) {
					//プレイヤーの周りか何もない場所なら入れる
					if (costMap_[nextNode->NodeID] != Player||costMap_[nextNode->NodeID] != Enemy)
					{
						nextNode->costFromStart = totalCost;

						FNR.push_back(e);
					}
				}
			}
		}
		nowEdge_ = seachMinCostEdge(FNR, nowEdge_);

		if (nowEdge_ == NULL)return false;
	}
	//何も見つからなければfalse;
	return false;
}

//FNRからコストを考慮して最短エッジ候補を返すサブルーチン
std::shared_ptr<EdgePath>  SeachGraph::seachMinCostEdge(std::vector<std::shared_ptr<EdgePath>>& FNR, const std::shared_ptr<EdgePath> nowEdge_) {
	std::shared_ptr<EdgePath> answer = NULL;//答えの入れ物
	int  answerNo = 0;
	float minCost = FLT_MAX;//最小のコストを保持するワーク

	for (int fnrNo = 0; fnrNo < FNR.size(); fnrNo++) {

		EdgePath* element = FNR[fnrNo].get();
		float nowCost = nodeVec_[nowEdge_->originNode]->costFromStart;
		//接続先の「スタート位置からのコスト」をfrontCostに取り出す(まだ登録されていないなら０となる)
		float frontCost = nodeVec_[element->distnationNode]->costFromStart;


		//frontCostが、今調べているエッジ（nowEdge）の接続先のトータルコスト以上のもの中で
		//一番小さい接続先の「スタート位置からのコスト」を持つエッジを答え（answer）として記憶。

		if (frontCost >= nowCost) {
			if (minCost > frontCost) {//forループで見つけたものの中で最小か？
				minCost = frontCost;
				answer = FNR[fnrNo];
				answerNo = fnrNo;
			}
		}

	}


	if (answer == NULL) {
		return 0;
	}
	FNR.erase(FNR.begin() + answerNo);//答えのエッジはダイクストラのサーチ候補から外す

	return answer;
}

//プレイヤーの周りに配置するための道順を返す
std::vector<DirectX::XMFLOAT3> SeachGraph::SearchEnemySetPos(DirectX::XMFLOAT3 enemyPos, DirectX::XMFLOAT3 goalPos, DIRECTION dirSet)
{
	std::lock_guard<std::mutex> lock(mutex_);

	goalId_ = GetNodeVecFromWorldPos(goalPos)->NodeID;
	if (dirSet != NONE_DIR)	//ポジションから少しずらしたい場合につかう
		goalId_ = nodeVec_[goalId_]->edge[dirSet]->distnationNode;

	startId_ = GetNodeVecFromWorldPos(enemyPos)->NodeID;

	//道順を入れる
	std::vector<DirectX::XMFLOAT3> enemyToPlayer;
	//道順サーチ
	if (DijkstraSseach())
	{
		//成功した場合
		//折り返し地点（曲がるとき）のposを保存
		if (goalId_ > 0)	//ゴールまでの道順
		{

			int startNo = goalId_;
			int endNo = -1;

			//方向を見る
			int dir = -1;

			while (endNo != startId_)
			{
				endNo = m_Answer_[startNo];

				if (dir == -1)
				{
					//方向を見る
					for (int e = 0; e < NodePath::EDGE_NO; ++e)
					{
						if (nodeVec_[startNo]->edge[e]->distnationNode < 0)continue;

						//同じナンバーの場合はとりあえず0を入れる
						if (startNo == endNo)
						{
							//方向を保存
							dir = DIRECTION::UP;
							break;
						}

						if (nodeVec_[nodeVec_[startNo]->edge[e]->distnationNode]->NodeID == endNo)	//次のノードの場合
						{
							//方向を保存
							dir = e;
							break;
						}
					}
				}
				//方向が違う場合
				if (nodeVec_[startNo]->edge[dir]->distnationNode != endNo)
				{
					//前に入れる
					enemyToPlayer.insert(enemyToPlayer.begin(), nodeVec_[startNo]->pos);
					//方向を見る
					for (int e = 0; e < NodePath::EDGE_NO; ++e)
					{
						if (nodeVec_[startNo]->edge[e]->distnationNode < 0)continue;

						if (nodeVec_[nodeVec_[startNo]->edge[e]->distnationNode]->NodeID == endNo)	//次のノードの場合
						{
							//方向を保存
							dir = e;
							break;
						}
					}
				}

				startNo = endNo;
			}

			//最後に目的地を入れる
			enemyToPlayer.emplace_back(nodeVec_[goalId_]->pos);
		}
	}

	return enemyToPlayer;
}


std::shared_ptr<NodePath> SeachGraph::GetNodeVecFromWorldPos(DirectX::XMFLOAT3 pos)
{
	//点をグラフに合わせる
	pos.x -= meshCenterX_;
	pos.z -= meshCenterZ_;

	//マスに変換
	int masX = (int)((pos.x + meshOffsetX_) / (meshSize_));
	int masZ = (int)((pos.z + meshOffsetZ_) / (meshSize_));

	//範囲外補正
	if (masX < 0)masX = 0;
	if (masX > cellSizeX_ - 1)masX = cellSizeX_ - 1;
	if (masZ < 0)masZ = 0;
	if (masZ > cellSizeZ_ - 1)masZ = cellSizeZ_ - 1;


	int id = cellSizeX_ * masZ + masX;


	return nodeVec_[id];
}
