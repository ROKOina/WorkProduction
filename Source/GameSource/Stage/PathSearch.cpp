#include "PathSearch.h"

#include "Graphics/Graphics.h"
#include "Components/TransformCom.h"
#include "GameSource/ScriptComponents/Enemy/EnemyManager.h"

#include <imgui.h>

/// EdgePath ///
void EdgePath::initSub(int node, int& buf1)
{
	buf1 = -1;
	originNode = node;
}



/// SeachGraph ///
static std::vector<DirectX::XMFLOAT3> ppp;
void SeachGraph::UpdatePath()
{
	//動的にコストを変えるので最初は0初期化する
	for (auto& c : costMap)
	{
		c = 0;
	}

	//プレイヤーの場所のコスト１にする
	DirectX::XMFLOAT3 pPos = player_.lock()->transform_->GetWorldPosition();
	std::shared_ptr<NodePath> node = GetNodeVecFromWorldPos(pPos);
	costMap[node->NodeID] = Player;

	//プレイヤー周りを２にする
	for (auto& e : node->edge)
	{
		if (e->distnationNode < 0)continue;
		costMap[e->distnationNode] = PlayerMawari;
	}

	//敵の場所を３にする
	for (auto& enemy : EnemyManager::Instance().GetNearEnemies())
	{
		DirectX::XMFLOAT3 ePos = enemy.enemy.lock()->transform_->GetWorldPosition();
		std::shared_ptr<NodePath> enemyNode = GetNodeVecFromWorldPos(ePos);
		costMap[enemyNode->NodeID] = Enemy;
	}
}

void SeachGraph::RenderPath()
{
	if (debugEnabled_)
	{
		int i = 0;
		for (auto& n : nodeVec)
		{
			if (i == goalId)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize * 0.4f ,0.01f,meshSize * 0.4f }, { 0,0,0,1 });
			else if (costMap[i] == Player)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize * 0.4f ,0.01f,meshSize * 0.4f }, { 1,0,0,1 });
			else if (costMap[i] == PlayerMawari)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize * 0.4f ,0.01f,meshSize * 0.4f }, { 0,1,0,1 });
			else if (costMap[i] == Enemy)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize * 0.4f ,0.01f,meshSize * 0.4f }, { 0,1,1,1 });
			else
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize * 0.4f ,0.01f,meshSize * 0.4f }, { 1,1,1,1 });
			i++;
		}

		//ゴールまでの道順
		if (goalId > 0)
		{
			int startNo = goalId;
			int endNo = -1;

			while (endNo != startId)
			{
				endNo = m_Answer[startNo];

				if (endNo == -1)break;

				Graphics::Instance().GetDebugRenderer()->DrawBox(nodeVec[endNo]->pos, { meshSize * 0.4f ,0.01f,meshSize * 0.4f }, { 1,0,1,1 });

				startNo = endNo;
			}
		}
	}

	{
		ImGui::SetNextWindowPos(ImVec2(950, 50), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		ImGui::Begin("PathSearch", nullptr, ImGuiWindowFlags_None);

		ImGui::Checkbox("debugEnabled", &debugEnabled_);

		if (ImGui::Button("startRout"))
		{
			//ppp = SearchEnemySetPos(EnemyManager::Instance().GetNearEnemies()[0].enemy.lock()->transform_->GetWorldPosition());
		}

		ImGui::End();
	}
}


#define NANAME 1.414f;

void SeachGraph::InitSub(int _sizeX, int _sizeZ, float _centerX, float _centerZ, float _size, float _thresholdY, std::shared_ptr<GameObject> player)
{
	player_ = player;
	thresholdY = _thresholdY;//高さの閾値

	cellSizeX = _sizeX;  /*ダイクストラの升目*/
	cellSizeZ = _sizeZ;
	meshCenterX = _centerX;
	meshCenterZ = _centerZ;
	meshSize = _size;  /*メッシュ単位の幅*/

	meshOffsetX = meshSize * cellSizeX / 2;  /*オフセット*/
	meshOffsetZ = meshSize * cellSizeZ / 2;  /*オフセット*/
	cellNo = cellSizeX * cellSizeZ;
	nodeVec.clear();//地形

	//描画用OBJECTをセット
	for (int i = 0; i < cellNo; i++) {
		auto piece = std::make_shared<NodePath>();
		piece->NodeID = i;

		float culms = (float)cellSizeX;
		piece->pos.x = (meshOffsetX * -1.0f + (float)(i % cellSizeX) * meshSize + (meshSize / 2.0f)) + meshCenterX;
		piece->pos.z = (meshOffsetZ * -1.0f + (float)(i / cellSizeX) * meshSize +
			+(meshSize / 2.0f)) + meshCenterZ;
		nodeVec.push_back(std::move(piece));

		m_Answer.push_back(-1);
		costMap.push_back(0);

	}

	//ナビメッシュを作成(今のところy固定)
	makeMap(cellSizeX, cellSizeZ, 1);

	int buf1 = 0;
	for (int z = 0; z < cellSizeZ; z++) {

		for (int x = 0; x < cellSizeX; x++) {
			int node = cellSizeX * z + x;
			//P_EDGE0,  /*エッジの表示　12時の方向*/
			std::shared_ptr<EdgePath> e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::UP]);
			e->initSub(node, buf1);

			if (
				!(z <= 0)
				) {
				buf1 = node - cellSizeX;
			}
			setSub(node, buf1, 0);

			/*エッジの表示　1.5時の方向*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::UP_R]);
			e->initSub(node, buf1);
			if (
				!(z <= 0) && !(x >= cellSizeX - 1)
				) {
				buf1 = node - cellSizeX + 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 1);



			/*エッジの表示　３時の方向*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::RIGHT]);
			e->initSub(node, buf1);
			if (
				!(x >= cellSizeX - 1)
				) {
				buf1 = node + 1;
			}
			setSub(node, buf1, 2);

			/*エッジの表示　4.5時の方向*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::DOWN_R]);
			e->initSub(node, buf1);
			if (
				!(z >= cellSizeZ - 1) && !(x >= cellSizeX - 1)
				) {
				buf1 = node + cellSizeX + 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 3);


			/*エッジの表示　6時の方向*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::DOWN]);
			e->initSub(node, buf1);
			if (
				!(z >= cellSizeZ - 1)
				) {
				buf1 = node + cellSizeX;
			}
			setSub(node, buf1, 4);

			/*エッジの表示　7.5時の方向*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::DOWN_L]);
			e->initSub(node, buf1);
			if (
				!(z >= cellSizeZ - 1) && !(x <= 0)
				) {
				buf1 = node + cellSizeX - 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 5);

			/*エッジの表示　9時の方向*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::LEFT]);
			e->initSub(node, buf1);
			if (
				!(x <= 0)
				) {
				buf1 = node - 1;
			}
			setSub(node, buf1, 6);

			/*エッジの表示　10.5時の方向*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::UP_L]);
			e->initSub(node, buf1);
			if (
				!(z <= 0) &&
				!(x <= 0)
				) {
				buf1 = node - cellSizeX - 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 7);
		}

	}

	//動的にコストを変えるので最初は0初期化する
	for (auto& c : costMap)
	{
		c = 0;
	}

	//プレイヤーの場所のコスト１にする
	DirectX::XMFLOAT3 pPos = player_.lock()->transform_->GetWorldPosition();
	std::shared_ptr<NodePath> node = GetNodeVecFromWorldPos(pPos);
	costMap[node->NodeID] = Player;

	//プレイヤー周りを２にする
	for (auto& e : node->edge)
	{
		costMap[e->distnationNode] = PlayerMawari;
	}

	//敵の場所を３にする
	for (auto& enemy : EnemyManager::Instance().GetNearEnemies())
	{
		DirectX::XMFLOAT3 ePos = enemy.enemy.lock()->transform_->GetWorldPosition();
		std::shared_ptr<NodePath> enemyNode = GetNodeVecFromWorldPos(ePos);
		costMap[enemyNode->NodeID] = Enemy;
	}

}

//高さを比較
bool  SeachGraph::setSub(int node, int buf1, int eNo) {

	std::shared_ptr<EdgePath> e = (nodeVec[node]->edge[eNo]);
	e->distnationNode = buf1;//ノードのエッジを繋ぐ

	if (buf1 < 0) {
		return false;
	}
	if (costMap[node] == 1) {//繋いではダメなノードなら
		for (int i = 0; i < nodeVec[buf1]->EDGE_NO; i++) {
			nodeVec[node]->edge[i]->distnationNode = -1;
		}
		return false;
	};
	float nY = nodeVec[node]->pos.y;
	float bY = nodeVec[buf1]->pos.y;
	if (abs(nY - bY) > thresholdY) {//段差があれば
		costMap[node] = 1;
		costMap[buf1] = 1;
		for (int i = 0; i < nodeVec[buf1]->EDGE_NO; i++) {
			nodeVec[buf1]->edge[i]->distnationNode = -1;
			nodeVec[node]->edge[i]->distnationNode = -1;
		}
		return false;
	}
	return true;
}


void SeachGraph::makeMap(int _x, int _z, float _size)
{
	//costMap[SeachGraph::CULMS_NO * SeachGraph::CULMS_NO];
	for (int z1 = 0; z1 < _z; z1++) {
		for (int x1 = 0; x1 < _x; x1++) {
			//V3 	rayCastS = V3(meshOffsetX * -1.0f + x1 * meshSize + meshCenterX + (meshSize / 2.0f),
			//	1000,
			//	meshOffsetZ * -1.0f + z1 * meshSize + meshCenterZ + (meshSize / 2.0f));
			//V3 	rayCastG = V3(meshOffsetX * -1.0f + x1 * meshSize + meshCenterX + (meshSize / 2.0f),
			//	-1000,
			//	meshOffsetZ * -1.0f + z1 * meshSize + meshCenterZ + (meshSize / 2.0f));


			////マウスをクリックした場所をGETする。
			////pOut ﾚｲの発射位置
			////vOut ﾚｲの発射ベクトル
			////sPos マウスのクリック位置	

			//HitResult result;
			//if (Collision::IntersectRayVsModel(
			//	rayCastS,
			//	rayCastG,
			//	model,
			//	result
			//)) {

			//	nodeVec[x1 + z1 * cellSizeX]->pos.y = result.position.y;//高さを入れる。
			//	costMap.push_back(0);//段差判断用ワーク

			//}
			//else {
			//	nodeVec[x1 + z1 * cellSizeX]->pos.y = 0;//高さを入れる。
			//	costMap.push_back(0);//段差判断用ワーク

			//}

			nodeVec[x1 + z1 * cellSizeX]->pos.y = 0.3f;
		}
	}

}


bool SeachGraph::DijkstraSseach()
{
	if (goalId < 0)return false;

	//エッジ型データのフロンティアツリーを作成
	std::vector<std::shared_ptr<EdgePath>> FNR;
	//サーチ済みのFGをリセット
	for (int i = 0; i < cellNo; i++) {
		nodeVec[i]->seachFg = false;
		nodeVec[i]->costFromStart = 0.0f;  //スタートからのコスト
		m_Answer[i] = -1;
	}

	std::shared_ptr<EdgePath> edge = std::make_shared<EdgePath>();
	edge->distnationNode = startId;
	edge->originNode = startId;


	//if (costMap[goalId] == 1) {
	//	return false;
	//}

	//前準備としてダミーエッジデータをセット
	nowEdge = edge;
	while (true)
	{
		//サーチしたEdgeの記録
		m_Answer[nowEdge->distnationNode] = nowEdge->originNode;
		if (nowEdge->distnationNode == goalId) { return true; }



		//nowEdgeの先のノードを取得する。
		std::shared_ptr<NodePath> node = nodeVec[nowEdge->distnationNode];//進み先のノード

		for (int edgeNo = 0; edgeNo < NodePath::EDGE_NO; edgeNo++) {
			std::shared_ptr<EdgePath> e = node->edge[edgeNo];
			if (e->distnationNode >= 0) {//そのエッジは有効である。
				std::shared_ptr<NodePath> nextNode = nodeVec[e->distnationNode];//進み先のノード
				//進み先のノードまでのコストを計算
				float totalCost = node->costFromStart + e->cost;

				//進み先がプレイヤーの周りならコスト50
				if (costMap[nextNode->NodeID] == PlayerMawari&& nextNode->NodeID != goalId)
				{
					totalCost += 50;
				}

				//進み先のコストがまだ計算されていないか、新しいコストの方が低ければ登録
				if (nextNode->costFromStart == 0 || nextNode->costFromStart > totalCost) {
					//プレイヤーの周りか何もない場所なら入れる
					//if (costMap[nextNode->NodeID] == 0|| costMap[nextNode->NodeID]==PlayerMawari)
					{
						nextNode->costFromStart = totalCost;

						FNR.push_back(e);
					}
				}
			}
		}
		nowEdge = seachMinCostEdge(FNR, nowEdge);

		if (nowEdge == NULL)return false;
	}
	//何も見つからなければfalse;
	return false;
}

//FNRからコストを考慮して最短エッジ候補を返すサブルーチン
std::shared_ptr<EdgePath>  SeachGraph::seachMinCostEdge(std::vector<std::shared_ptr<EdgePath>>& FNR, const std::shared_ptr<EdgePath> nowEdge) {
	std::shared_ptr<EdgePath> answer = NULL;//答えの入れ物
	int  answerNo = 0;
	float minCost = FLT_MAX;//最小のコストを保持するワーク

	for (int fnrNo = 0; fnrNo < FNR.size(); fnrNo++) {

		EdgePath* element = FNR[fnrNo].get();
		float nowCost = nodeVec[nowEdge->originNode]->costFromStart;
		//接続先の「スタート位置からのコスト」をfrontCostに取り出す(まだ登録されていないなら０となる)
		float frontCost = nodeVec[element->distnationNode]->costFromStart;


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
	goalId = GetNodeVecFromWorldPos(goalPos)->NodeID;
	if (dirSet != NONE_DIR)	//ポジションから少しずらしたい場合につかう
		goalId = nodeVec[goalId]->edge[dirSet]->distnationNode;

	startId = GetNodeVecFromWorldPos(enemyPos)->NodeID;

	//道順を入れる
	std::vector<DirectX::XMFLOAT3> enemyToPlayer;
	//道順サーチ
	if (DijkstraSseach())
	{
		//成功した場合
		//折り返し地点（曲がるとき）のposを保存
		if (goalId > 0)	//ゴールまでの道順
		{

			int startNo = goalId;
			int endNo = -1;

			//方向を見る
			int dir = -1;

			while (endNo != startId)
			{
				endNo = m_Answer[startNo];

				if (dir == -1)
				{
					//方向を見る
					for (int e = 0; e < NodePath::EDGE_NO; ++e)
					{
						if (nodeVec[startNo]->edge[e]->distnationNode < 0)continue;

						//同じナンバーの場合はとりあえず0を入れる
						if (startNo == endNo)
						{
							//方向を保存
							dir = DIRECTION::UP;
							break;
						}

						if (nodeVec[nodeVec[startNo]->edge[e]->distnationNode]->NodeID == endNo)	//次のノードの場合
						{
							//方向を保存
							dir = e;
							break;
						}
					}
				}
				//方向が違う場合
				if (nodeVec[startNo]->edge[dir]->distnationNode != endNo)
				{
					//前に入れる
					enemyToPlayer.insert(enemyToPlayer.begin(), nodeVec[startNo]->pos);
					//方向を見る
					for (int e = 0; e < NodePath::EDGE_NO; ++e)
					{
						if (nodeVec[nodeVec[startNo]->edge[e]->distnationNode]->NodeID == endNo)	//次のノードの場合
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
			enemyToPlayer.emplace_back(nodeVec[goalId]->pos);
		}

	}

	return enemyToPlayer;
}


std::shared_ptr<NodePath> SeachGraph::GetNodeVecFromWorldPos(DirectX::XMFLOAT3 pos)
{
	//点をグラフに合わせる
	pos.x -= meshCenterX;
	pos.z -= meshCenterZ;

	//マスに変換
	int masX = (int)((pos.x + meshOffsetX) / (meshSize));
	int masZ = (int)((pos.z + meshOffsetZ) / (meshSize));

	//範囲外補正
	if (masX < 0)masX = 0;
	if (masX > cellSizeX - 1)masX = cellSizeX - 1;
	if (masZ < 0)masZ = 0;
	if (masZ > cellSizeZ - 1)masZ = cellSizeZ - 1;


	int id = cellSizeX * masZ + masX;


	return nodeVec[id];
}
