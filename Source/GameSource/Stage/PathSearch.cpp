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
	//���I�ɃR�X�g��ς���̂ōŏ���0����������
	for (auto& c : costMap)
	{
		c = 0;
	}

	//�v���C���[�̏ꏊ�̃R�X�g�P�ɂ���
	DirectX::XMFLOAT3 pPos = player_.lock()->transform_->GetWorldPosition();
	std::shared_ptr<NodePath> node = GetNodeVecFromWorldPos(pPos);
	costMap[node->NodeID] = Player;

	//�v���C���[������Q�ɂ���
	for (auto& e : node->edge)
	{
		if (e->distnationNode < 0)continue;
		costMap[e->distnationNode] = PlayerMawari;
	}

	//�G�̏ꏊ���R�ɂ���
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

		//�S�[���܂ł̓���
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
	thresholdY = _thresholdY;//������臒l

	cellSizeX = _sizeX;  /*�_�C�N�X�g���̏���*/
	cellSizeZ = _sizeZ;
	meshCenterX = _centerX;
	meshCenterZ = _centerZ;
	meshSize = _size;  /*���b�V���P�ʂ̕�*/

	meshOffsetX = meshSize * cellSizeX / 2;  /*�I�t�Z�b�g*/
	meshOffsetZ = meshSize * cellSizeZ / 2;  /*�I�t�Z�b�g*/
	cellNo = cellSizeX * cellSizeZ;
	nodeVec.clear();//�n�`

	//�`��pOBJECT���Z�b�g
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

	//�i�r���b�V�����쐬(���̂Ƃ���y�Œ�)
	makeMap(cellSizeX, cellSizeZ, 1);

	int buf1 = 0;
	for (int z = 0; z < cellSizeZ; z++) {

		for (int x = 0; x < cellSizeX; x++) {
			int node = cellSizeX * z + x;
			//P_EDGE0,  /*�G�b�W�̕\���@12���̕���*/
			std::shared_ptr<EdgePath> e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::UP]);
			e->initSub(node, buf1);

			if (
				!(z <= 0)
				) {
				buf1 = node - cellSizeX;
			}
			setSub(node, buf1, 0);

			/*�G�b�W�̕\���@1.5���̕���*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::UP_R]);
			e->initSub(node, buf1);
			if (
				!(z <= 0) && !(x >= cellSizeX - 1)
				) {
				buf1 = node - cellSizeX + 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 1);



			/*�G�b�W�̕\���@�R���̕���*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::RIGHT]);
			e->initSub(node, buf1);
			if (
				!(x >= cellSizeX - 1)
				) {
				buf1 = node + 1;
			}
			setSub(node, buf1, 2);

			/*�G�b�W�̕\���@4.5���̕���*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::DOWN_R]);
			e->initSub(node, buf1);
			if (
				!(z >= cellSizeZ - 1) && !(x >= cellSizeX - 1)
				) {
				buf1 = node + cellSizeX + 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 3);


			/*�G�b�W�̕\���@6���̕���*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::DOWN]);
			e->initSub(node, buf1);
			if (
				!(z >= cellSizeZ - 1)
				) {
				buf1 = node + cellSizeX;
			}
			setSub(node, buf1, 4);

			/*�G�b�W�̕\���@7.5���̕���*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::DOWN_L]);
			e->initSub(node, buf1);
			if (
				!(z >= cellSizeZ - 1) && !(x <= 0)
				) {
				buf1 = node + cellSizeX - 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 5);

			/*�G�b�W�̕\���@9���̕���*/
			e = (nodeVec[cellSizeX * z + x]->edge[DIRECTION::LEFT]);
			e->initSub(node, buf1);
			if (
				!(x <= 0)
				) {
				buf1 = node - 1;
			}
			setSub(node, buf1, 6);

			/*�G�b�W�̕\���@10.5���̕���*/
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

	//���I�ɃR�X�g��ς���̂ōŏ���0����������
	for (auto& c : costMap)
	{
		c = 0;
	}

	//�v���C���[�̏ꏊ�̃R�X�g�P�ɂ���
	DirectX::XMFLOAT3 pPos = player_.lock()->transform_->GetWorldPosition();
	std::shared_ptr<NodePath> node = GetNodeVecFromWorldPos(pPos);
	costMap[node->NodeID] = Player;

	//�v���C���[������Q�ɂ���
	for (auto& e : node->edge)
	{
		costMap[e->distnationNode] = PlayerMawari;
	}

	//�G�̏ꏊ���R�ɂ���
	for (auto& enemy : EnemyManager::Instance().GetNearEnemies())
	{
		DirectX::XMFLOAT3 ePos = enemy.enemy.lock()->transform_->GetWorldPosition();
		std::shared_ptr<NodePath> enemyNode = GetNodeVecFromWorldPos(ePos);
		costMap[enemyNode->NodeID] = Enemy;
	}

}

//�������r
bool  SeachGraph::setSub(int node, int buf1, int eNo) {

	std::shared_ptr<EdgePath> e = (nodeVec[node]->edge[eNo]);
	e->distnationNode = buf1;//�m�[�h�̃G�b�W���q��

	if (buf1 < 0) {
		return false;
	}
	if (costMap[node] == 1) {//�q���ł̓_���ȃm�[�h�Ȃ�
		for (int i = 0; i < nodeVec[buf1]->EDGE_NO; i++) {
			nodeVec[node]->edge[i]->distnationNode = -1;
		}
		return false;
	};
	float nY = nodeVec[node]->pos.y;
	float bY = nodeVec[buf1]->pos.y;
	if (abs(nY - bY) > thresholdY) {//�i���������
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


			////�}�E�X���N���b�N�����ꏊ��GET����B
			////pOut ڲ�̔��ˈʒu
			////vOut ڲ�̔��˃x�N�g��
			////sPos �}�E�X�̃N���b�N�ʒu	

			//HitResult result;
			//if (Collision::IntersectRayVsModel(
			//	rayCastS,
			//	rayCastG,
			//	model,
			//	result
			//)) {

			//	nodeVec[x1 + z1 * cellSizeX]->pos.y = result.position.y;//����������B
			//	costMap.push_back(0);//�i�����f�p���[�N

			//}
			//else {
			//	nodeVec[x1 + z1 * cellSizeX]->pos.y = 0;//����������B
			//	costMap.push_back(0);//�i�����f�p���[�N

			//}

			nodeVec[x1 + z1 * cellSizeX]->pos.y = 0.3f;
		}
	}

}


bool SeachGraph::DijkstraSseach()
{
	if (goalId < 0)return false;

	//�G�b�W�^�f�[�^�̃t�����e�B�A�c���[���쐬
	std::vector<std::shared_ptr<EdgePath>> FNR;
	//�T�[�`�ς݂�FG�����Z�b�g
	for (int i = 0; i < cellNo; i++) {
		nodeVec[i]->seachFg = false;
		nodeVec[i]->costFromStart = 0.0f;  //�X�^�[�g����̃R�X�g
		m_Answer[i] = -1;
	}

	std::shared_ptr<EdgePath> edge = std::make_shared<EdgePath>();
	edge->distnationNode = startId;
	edge->originNode = startId;


	//if (costMap[goalId] == 1) {
	//	return false;
	//}

	//�O�����Ƃ��ă_�~�[�G�b�W�f�[�^���Z�b�g
	nowEdge = edge;
	while (true)
	{
		//�T�[�`����Edge�̋L�^
		m_Answer[nowEdge->distnationNode] = nowEdge->originNode;
		if (nowEdge->distnationNode == goalId) { return true; }



		//nowEdge�̐�̃m�[�h���擾����B
		std::shared_ptr<NodePath> node = nodeVec[nowEdge->distnationNode];//�i�ݐ�̃m�[�h

		for (int edgeNo = 0; edgeNo < NodePath::EDGE_NO; edgeNo++) {
			std::shared_ptr<EdgePath> e = node->edge[edgeNo];
			if (e->distnationNode >= 0) {//���̃G�b�W�͗L���ł���B
				std::shared_ptr<NodePath> nextNode = nodeVec[e->distnationNode];//�i�ݐ�̃m�[�h
				//�i�ݐ�̃m�[�h�܂ł̃R�X�g���v�Z
				float totalCost = node->costFromStart + e->cost;

				//�i�ݐ悪�v���C���[�̎���Ȃ�R�X�g50
				if (costMap[nextNode->NodeID] == PlayerMawari&& nextNode->NodeID != goalId)
				{
					totalCost += 50;
				}

				//�i�ݐ�̃R�X�g���܂��v�Z����Ă��Ȃ����A�V�����R�X�g�̕����Ⴏ��Γo�^
				if (nextNode->costFromStart == 0 || nextNode->costFromStart > totalCost) {
					//�v���C���[�̎��肩�����Ȃ��ꏊ�Ȃ�����
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
	//����������Ȃ����false;
	return false;
}

//FNR����R�X�g���l�����čŒZ�G�b�W����Ԃ��T�u���[�`��
std::shared_ptr<EdgePath>  SeachGraph::seachMinCostEdge(std::vector<std::shared_ptr<EdgePath>>& FNR, const std::shared_ptr<EdgePath> nowEdge) {
	std::shared_ptr<EdgePath> answer = NULL;//�����̓��ꕨ
	int  answerNo = 0;
	float minCost = FLT_MAX;//�ŏ��̃R�X�g��ێ����郏�[�N

	for (int fnrNo = 0; fnrNo < FNR.size(); fnrNo++) {

		EdgePath* element = FNR[fnrNo].get();
		float nowCost = nodeVec[nowEdge->originNode]->costFromStart;
		//�ڑ���́u�X�^�[�g�ʒu����̃R�X�g�v��frontCost�Ɏ��o��(�܂��o�^����Ă��Ȃ��Ȃ�O�ƂȂ�)
		float frontCost = nodeVec[element->distnationNode]->costFromStart;


		//frontCost���A�����ׂĂ���G�b�W�inowEdge�j�̐ڑ���̃g�[�^���R�X�g�ȏ�̂��̒���
		//��ԏ������ڑ���́u�X�^�[�g�ʒu����̃R�X�g�v�����G�b�W�𓚂��ianswer�j�Ƃ��ċL���B

		if (frontCost >= nowCost) {
			if (minCost > frontCost) {//for���[�v�Ō��������̂̒��ōŏ����H
				minCost = frontCost;
				answer = FNR[fnrNo];
				answerNo = fnrNo;
			}
		}

	}


	if (answer == NULL) {
		return 0;
	}
	FNR.erase(FNR.begin() + answerNo);//�����̃G�b�W�̓_�C�N�X�g���̃T�[�`��₩��O��

	return answer;
}

//�v���C���[�̎���ɔz�u���邽�߂̓�����Ԃ�
std::vector<DirectX::XMFLOAT3> SeachGraph::SearchEnemySetPos(DirectX::XMFLOAT3 enemyPos, DirectX::XMFLOAT3 goalPos, DIRECTION dirSet)
{
	goalId = GetNodeVecFromWorldPos(goalPos)->NodeID;
	if (dirSet != NONE_DIR)	//�|�W�V�������班�����炵�����ꍇ�ɂ���
		goalId = nodeVec[goalId]->edge[dirSet]->distnationNode;

	startId = GetNodeVecFromWorldPos(enemyPos)->NodeID;

	//����������
	std::vector<DirectX::XMFLOAT3> enemyToPlayer;
	//�����T�[�`
	if (DijkstraSseach())
	{
		//���������ꍇ
		//�܂�Ԃ��n�_�i�Ȃ���Ƃ��j��pos��ۑ�
		if (goalId > 0)	//�S�[���܂ł̓���
		{

			int startNo = goalId;
			int endNo = -1;

			//����������
			int dir = -1;

			while (endNo != startId)
			{
				endNo = m_Answer[startNo];

				if (dir == -1)
				{
					//����������
					for (int e = 0; e < NodePath::EDGE_NO; ++e)
					{
						if (nodeVec[startNo]->edge[e]->distnationNode < 0)continue;

						//�����i���o�[�̏ꍇ�͂Ƃ肠����0������
						if (startNo == endNo)
						{
							//������ۑ�
							dir = DIRECTION::UP;
							break;
						}

						if (nodeVec[nodeVec[startNo]->edge[e]->distnationNode]->NodeID == endNo)	//���̃m�[�h�̏ꍇ
						{
							//������ۑ�
							dir = e;
							break;
						}
					}
				}
				//�������Ⴄ�ꍇ
				if (nodeVec[startNo]->edge[dir]->distnationNode != endNo)
				{
					//�O�ɓ����
					enemyToPlayer.insert(enemyToPlayer.begin(), nodeVec[startNo]->pos);
					//����������
					for (int e = 0; e < NodePath::EDGE_NO; ++e)
					{
						if (nodeVec[nodeVec[startNo]->edge[e]->distnationNode]->NodeID == endNo)	//���̃m�[�h�̏ꍇ
						{
							//������ۑ�
							dir = e;
							break;
						}
					}
				}

				startNo = endNo;
			}

			//�Ō�ɖړI�n������
			enemyToPlayer.emplace_back(nodeVec[goalId]->pos);
		}

	}

	return enemyToPlayer;
}


std::shared_ptr<NodePath> SeachGraph::GetNodeVecFromWorldPos(DirectX::XMFLOAT3 pos)
{
	//�_���O���t�ɍ��킹��
	pos.x -= meshCenterX;
	pos.z -= meshCenterZ;

	//�}�X�ɕϊ�
	int masX = (int)((pos.x + meshOffsetX) / (meshSize));
	int masZ = (int)((pos.z + meshOffsetZ) / (meshSize));

	//�͈͊O�␳
	if (masX < 0)masX = 0;
	if (masX > cellSizeX - 1)masX = cellSizeX - 1;
	if (masZ < 0)masZ = 0;
	if (masZ > cellSizeZ - 1)masZ = cellSizeZ - 1;


	int id = cellSizeX * masZ + masX;


	return nodeVec[id];
}
