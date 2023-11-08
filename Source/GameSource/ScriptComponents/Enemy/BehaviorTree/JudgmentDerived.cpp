#include "JudgmentDerived.h"
#include "../EnemyManager.h"
#include "../EnemyNearCom.h"
#include "../EnemyFarCom.h"

#include "Components/TransformCom.h"

// BattleNode�ɑJ�ڂł��邩����
bool BattleJudgment::Judgment()
{
	// �v���C���[�������邩
	if (owner_.lock()->SearchPlayer())
	{
		return true;
	}
	return false;
}

// AttackNode�ɑJ�ڂł��邩����
bool NearAttackJudgment::Judgment()
{
	std::shared_ptr<TransformCom> myTransform = owner_.lock()->GetGameObject()->transform_;

	// �ΏۂƂ̋������Z�o
	DirectX::XMFLOAT3 pos = myTransform->GetWorldPosition();
	DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();

	float vx = playerPos.x - pos.x;
	float vy = playerPos.y - pos.y;
	float vz = playerPos.z - pos.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);

	//����������
	if (dist < owner_.lock()->GetAttackRange())
	{
		if (owner_.lock()->GetIsAttackFlag())
		{
			//�p�x������
			QuaternionStruct myQ = myTransform->GetRotation();
			QuaternionStruct focusQ = QuaternionStruct::LookRotation(DirectX::XMFLOAT3(vx, 0, vz));

			float dot = DirectX::XMVectorGetX(DirectX::XMQuaternionDot(DirectX::XMLoadFloat4(&myQ.dxFloat4), DirectX::XMVectorScale(DirectX::XMLoadFloat4(&focusQ.dxFloat4), -1)));

			if (dot * dot > 0.99f)
				// AttackNode�֑J�ڂł���
				return true;
		}
		else
		{
			//�U������\��
			EnemyManager::Instance().SendMessaging(owner_.lock()->GetID(), EnemyManager::AI_ID::AI_INDEX, MESSAGE_TYPE::MsgAskAttackRight);
		}
	}
	return false;
}

// WanderNode�ɑJ�ڂł��邩����
bool WanderJudgment::Judgment()
{
	if (owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->GetIsPathFlag())return false;

	// �ړI�n�_�܂ł�XZ���ʂł̋�������
	std::shared_ptr<GameObject> ownerObj = owner_.lock()->GetGameObject();
	DirectX::XMFLOAT3 position = ownerObj->transform_->GetWorldPosition();
	DirectX::XMFLOAT3 targetPosition = owner_.lock()->GetTargetPosition();
	float vx = targetPosition.x - position.x;
	float vz = targetPosition.z - position.z;
	float distSq = vx * vx + vz * vz;

	// �ړI�n���痣��Ă���ꍇ
	float radius = 0.5f;
	if (distSq > radius * radius)
	{
		return true;	//����
	}

	return false;
}

// RouteNode�ɑJ�ڂł��邩����
bool RoutePathJudgment::Judgment()
{
	if (owner_.lock()->GetIsAttackFlag())return false;
	if (!owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->GetIsNearFlag())return false;

	//�G�}�l�[�W���[�擾
	EnemyManager& enemyManager = EnemyManager::Instance();

	//�o�H�T���̐��Ŕ���
	if (enemyManager.GetCurrentNearPathCount() >= enemyManager.GetNearEnemyLevel().togetherPathCount)
		return false;

	//�U���J�E���g�������ς��̎�
	if (enemyManager.GetCurrentNearAttackCount() >= enemyManager.GetNearEnemyLevel().togetherAttackCount)
	{
		std::shared_ptr<TransformCom> myTransform = owner_.lock()->GetGameObject()->transform_;

		// �ړI�n�_�܂ł�XZ���ʂł̋�������
		DirectX::XMFLOAT3 position = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 targetPosition = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();

		float vx = targetPosition.x - position.x;
		float vy = targetPosition.y - position.y;
		float vz = targetPosition.z - position.z;
		float dist = sqrtf(vx * vx + vy * vy + vz * vz);

		//�ڋ߃G���A�ɓ����Ă�����
		if (dist < EnemyManager::Instance().GetNearEnemyLevel().radius)
		{
			return true;
		}
	}

	return false;
}

bool AttackIdleJudgment::Judgment()
{
	if (!owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->GetIsAttackIdleFlag())
	{
		return true;
	}

	return false;
}
