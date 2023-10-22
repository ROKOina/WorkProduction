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
	//���I�ɃR�X�g��ς���̂ōŏ���0����������
	for (auto& c : costMap)
	{
		c = 0;
	}


	//�G�̏ꏊ��o�^
	for (auto& enemy : EnemyManager::Instance().GetNearEnemies())
	{
		DirectX::XMFLOAT3 ePos = enemy.enemy.lock()->transform_->GetWorldPosition();
		std::shared_ptr<NodePath> enemyNode = GetNodeVecFromWorldPos(ePos);
		costMap[enemyNode->NodeID] = Enemy;

		if (enemy.enemy.lock()->GetComponent<EnemyNearCom>()->GetIsPathFlag())continue;

		//�G����o�^
		for (auto& e : enemyNode->edge)
		{
			if (e->distnationNode < 0)continue;
			costMap[e->distnationNode] = EnemyMawari;
		}
	}

	//�v���C���[�̏ꏊ�̓o�^
	DirectX::XMFLOAT3 pPos = player_.lock()->transform_->GetWorldPosition();
	std::shared_ptr<NodePath> node = GetNodeVecFromWorldPos(pPos);
	costMap[node->NodeID] = Player;

	//�v���C���[�����o�^
	for (auto& e : node->edge)
	{
		if (e->distnationNode < 0)continue;
		costMap[e->distnationNode] = PlayerMawari;
	}
}

void SeachGraph::RenderPath()
{
	//�f�o�b�O�Ńu���b�N�\��
	if (debugEnabled_)
	{
		int i = 0;
		for (auto& n : nodeVec_)
		{
			if (i == goalId_)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 0,0,0,1 });
			else if (costMap[i] == EnemyMawari)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 0,1,0,1 });
			else if (costMap[i] == PlayerMawari)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 0,1,0,1 });
			else if (costMap[i] == Enemy)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 0,1,1,1 });
			else if (costMap[i] == Player)
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 1,0,0,1 });
			else
				Graphics::Instance().GetDebugRenderer()->DrawBox(n->pos, { meshSize_ * 0.4f ,0.01f,meshSize_ * 0.4f }, { 1,1,1,1 });
			i++;
		}

		//�S�[���܂ł̓���
		if (goalId_ > 0)
		{
			int startNo = goalId_;
			int endNo = -1;

			while (endNo != startId_)
			{
				endNo = m_Answer[startNo];

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
	thresholdY_ = _thresholdY;//������臒l

	cellSizeX_ = _sizeX;  /*�_�C�N�X�g���̏���*/
	cellSizeZ_ = _sizeZ;
	meshCenterX_ = _centerX;
	meshCenterZ_ = _centerZ;
	meshSize_ = _size;  /*���b�V���P�ʂ̕�*/

	meshOffsetX_ = meshSize_ * cellSizeX_ / 2;  /*�I�t�Z�b�g*/
	meshOffsetZ_ = meshSize_ * cellSizeZ_ / 2;  /*�I�t�Z�b�g*/
	cellNo_ = cellSizeX_ * cellSizeZ_;
	nodeVec_.clear();//�n�`

	//�`��pOBJECT���Z�b�g
	for (int i = 0; i < cellNo_; i++) {
		auto piece = std::make_shared<NodePath>();
		piece->NodeID = i;

		float culms = (float)cellSizeX_;
		piece->pos.x = (meshOffsetX_ * -1.0f + (float)(i % cellSizeX_) * meshSize_ + (meshSize_ / 2.0f)) + meshCenterX_;
		piece->pos.z = (meshOffsetZ_ * -1.0f + (float)(i / cellSizeX_) * meshSize_ +
			+(meshSize_ / 2.0f)) + meshCenterZ_;
		nodeVec_.push_back(std::move(piece));

		m_Answer.push_back(-1);
		costMap.push_back(0);

	}

	//�i�r���b�V�����쐬(���̂Ƃ���y�Œ�)
	makeMap(cellSizeX_, cellSizeZ_, 1);

	int buf1 = 0;
	for (int z = 0; z < cellSizeZ_; z++) {

		for (int x = 0; x < cellSizeX_; x++) {
			int node = cellSizeX_ * z + x;
			//P_EDGE0,  /*�G�b�W�̕\���@12���̕���*/
			std::shared_ptr<EdgePath> e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::UP]);
			e->initSub(node, buf1);

			if (
				!(z <= 0)
				) {
				buf1 = node - cellSizeX_;
			}
			setSub(node, buf1, 0);

			/*�G�b�W�̕\���@1.5���̕���*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::UP_R]);
			e->initSub(node, buf1);
			if (
				!(z <= 0) && !(x >= cellSizeX_ - 1)
				) {
				buf1 = node - cellSizeX_ + 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 1);



			/*�G�b�W�̕\���@�R���̕���*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::RIGHT]);
			e->initSub(node, buf1);
			if (
				!(x >= cellSizeX_ - 1)
				) {
				buf1 = node + 1;
			}
			setSub(node, buf1, 2);

			/*�G�b�W�̕\���@4.5���̕���*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::DOWN_R]);
			e->initSub(node, buf1);
			if (
				!(z >= cellSizeZ_ - 1) && !(x >= cellSizeX_ - 1)
				) {
				buf1 = node + cellSizeX_ + 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 3);


			/*�G�b�W�̕\���@6���̕���*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::DOWN]);
			e->initSub(node, buf1);
			if (
				!(z >= cellSizeZ_ - 1)
				) {
				buf1 = node + cellSizeX_;
			}
			setSub(node, buf1, 4);

			/*�G�b�W�̕\���@7.5���̕���*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::DOWN_L]);
			e->initSub(node, buf1);
			if (
				!(z >= cellSizeZ_ - 1) && !(x <= 0)
				) {
				buf1 = node + cellSizeX_ - 1;
			}
			e->cost = NANAME;
			setSub(node, buf1, 5);

			/*�G�b�W�̕\���@9���̕���*/
			e = (nodeVec_[cellSizeX_ * z + x]->edge[DIRECTION::LEFT]);
			e->initSub(node, buf1);
			if (
				!(x <= 0)
				) {
				buf1 = node - 1;
			}
			setSub(node, buf1, 6);

			/*�G�b�W�̕\���@10.5���̕���*/
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

	std::shared_ptr<EdgePath> e = (nodeVec_[node]->edge[eNo]);
	e->distnationNode = buf1;//�m�[�h�̃G�b�W���q��

	if (buf1 < 0) {
		return false;
	}
	if (costMap[node] == 1) {//�q���ł̓_���ȃm�[�h�Ȃ�
		for (int i = 0; i < nodeVec_[buf1]->EDGE_NO; i++) {
			nodeVec_[node]->edge[i]->distnationNode = -1;
		}
		return false;
	};
	float nY = nodeVec_[node]->pos.y;
	float bY = nodeVec_[buf1]->pos.y;
	if (abs(nY - bY) > thresholdY_) {//�i���������
		costMap[node] = 1;
		costMap[buf1] = 1;
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

	//�G�b�W�^�f�[�^�̃t�����e�B�A�c���[���쐬
	std::vector<std::shared_ptr<EdgePath>> FNR;
	//�T�[�`�ς݂�FG�����Z�b�g
	for (int i = 0; i < cellNo_; i++) {
		nodeVec_[i]->seachFg = false;
		nodeVec_[i]->costFromStart = 0.0f;  //�X�^�[�g����̃R�X�g
		m_Answer[i] = -1;
	}

	std::shared_ptr<EdgePath> edge = std::make_shared<EdgePath>();
	edge->distnationNode = startId_;
	edge->originNode = startId_;


	//�O�����Ƃ��ă_�~�[�G�b�W�f�[�^���Z�b�g
	nowEdge = edge;
	while (true)
	{
		//�T�[�`����Edge�̋L�^
		m_Answer[nowEdge->distnationNode] = nowEdge->originNode;
		if (nowEdge->distnationNode == goalId_) { return true; }



		//nowEdge�̐�̃m�[�h���擾����B
		std::shared_ptr<NodePath> node = nodeVec_[nowEdge->distnationNode];//�i�ݐ�̃m�[�h

		for (int edgeNo = 0; edgeNo < NodePath::EDGE_NO; edgeNo++) {
			std::shared_ptr<EdgePath> e = node->edge[edgeNo];
			if (e->distnationNode >= 0) {//���̃G�b�W�͗L���ł���B
				std::shared_ptr<NodePath> nextNode = nodeVec_[e->distnationNode];//�i�ݐ�̃m�[�h
				//�i�ݐ�̃m�[�h�܂ł̃R�X�g���v�Z
				float totalCost = node->costFromStart + e->cost;

				//�i�ݐ悪�v���C���[�̎���Ȃ�R�X�g500
				if (costMap[nextNode->NodeID] == PlayerMawari)
				{
					if (nextNode->NodeID != goalId_)
						totalCost += 500;
				}
				//�i�ݐ悪�G����Ȃ�R�X�g50
				if (costMap[nextNode->NodeID] == EnemyMawari)
				{
					if (nextNode->NodeID != goalId_)
						totalCost += 50;
				}

				//�i�ݐ�̃R�X�g���܂��v�Z����Ă��Ȃ����A�V�����R�X�g�̕����Ⴏ��Γo�^
				if (nextNode->costFromStart == 0 || nextNode->costFromStart > totalCost) {
					//�v���C���[�̎��肩�����Ȃ��ꏊ�Ȃ�����
					if (costMap[nextNode->NodeID] != Player||costMap[nextNode->NodeID] != Enemy)
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
		float nowCost = nodeVec_[nowEdge->originNode]->costFromStart;
		//�ڑ���́u�X�^�[�g�ʒu����̃R�X�g�v��frontCost�Ɏ��o��(�܂��o�^����Ă��Ȃ��Ȃ�O�ƂȂ�)
		float frontCost = nodeVec_[element->distnationNode]->costFromStart;


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
	std::lock_guard<std::mutex> lock(mutex_);

	goalId_ = GetNodeVecFromWorldPos(goalPos)->NodeID;
	if (dirSet != NONE_DIR)	//�|�W�V�������班�����炵�����ꍇ�ɂ���
		goalId_ = nodeVec_[goalId_]->edge[dirSet]->distnationNode;

	startId_ = GetNodeVecFromWorldPos(enemyPos)->NodeID;

	//����������
	std::vector<DirectX::XMFLOAT3> enemyToPlayer;
	//�����T�[�`
	if (DijkstraSseach())
	{
		//���������ꍇ
		//�܂�Ԃ��n�_�i�Ȃ���Ƃ��j��pos��ۑ�
		if (goalId_ > 0)	//�S�[���܂ł̓���
		{

			int startNo = goalId_;
			int endNo = -1;

			//����������
			int dir = -1;

			while (endNo != startId_)
			{
				endNo = m_Answer[startNo];

				if (dir == -1)
				{
					//����������
					for (int e = 0; e < NodePath::EDGE_NO; ++e)
					{
						if (nodeVec_[startNo]->edge[e]->distnationNode < 0)continue;

						//�����i���o�[�̏ꍇ�͂Ƃ肠����0������
						if (startNo == endNo)
						{
							//������ۑ�
							dir = DIRECTION::UP;
							break;
						}

						if (nodeVec_[nodeVec_[startNo]->edge[e]->distnationNode]->NodeID == endNo)	//���̃m�[�h�̏ꍇ
						{
							//������ۑ�
							dir = e;
							break;
						}
					}
				}
				//�������Ⴄ�ꍇ
				if (nodeVec_[startNo]->edge[dir]->distnationNode != endNo)
				{
					//�O�ɓ����
					enemyToPlayer.insert(enemyToPlayer.begin(), nodeVec_[startNo]->pos);
					//����������
					for (int e = 0; e < NodePath::EDGE_NO; ++e)
					{
						if (nodeVec_[nodeVec_[startNo]->edge[e]->distnationNode]->NodeID == endNo)	//���̃m�[�h�̏ꍇ
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
			enemyToPlayer.emplace_back(nodeVec_[goalId_]->pos);
		}
	}

	return enemyToPlayer;
}


std::shared_ptr<NodePath> SeachGraph::GetNodeVecFromWorldPos(DirectX::XMFLOAT3 pos)
{
	//�_���O���t�ɍ��킹��
	pos.x -= meshCenterX_;
	pos.z -= meshCenterZ_;

	//�}�X�ɕϊ�
	int masX = (int)((pos.x + meshOffsetX_) / (meshSize_));
	int masZ = (int)((pos.z + meshOffsetZ_) / (meshSize_));

	//�͈͊O�␳
	if (masX < 0)masX = 0;
	if (masX > cellSizeX_ - 1)masX = cellSizeX_ - 1;
	if (masZ < 0)masZ = 0;
	if (masZ > cellSizeZ_ - 1)masZ = cellSizeZ_ - 1;


	int id = cellSizeX_ * masZ + masX;


	return nodeVec_[id];
}
