#include "JudgmentDerived.h"
#include "../EnemyManager.h"
#include "../EnemyNearCom.h"
#include "../EnemyFarCom.h"

#include "Components/TransformCom.h"

// BattleNodeに遷移できるか判定
bool BattleJudgment::Judgment()
{
	// プレイヤーが見つかるか
	if (owner_.lock()->SearchPlayer())
	{
		return true;
	}
	return false;
}

// AttackNodeに遷移できるか判定
bool NearAttackJudgment::Judgment()
{
	std::shared_ptr<TransformCom> myTransform = owner_.lock()->GetGameObject()->transform_;

	// 対象との距離を算出
	DirectX::XMFLOAT3 pos = myTransform->GetWorldPosition();
	DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();

	float vx = playerPos.x - pos.x;
	float vy = playerPos.y - pos.y;
	float vz = playerPos.z - pos.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);

	//距離を見る
	if (dist < owner_.lock()->GetAttackRange())
	{
		if (owner_.lock()->GetIsAttackFlag())
		{
			//角度を見る
			QuaternionStruct myQ = myTransform->GetRotation();
			QuaternionStruct focusQ = QuaternionStruct::LookRotation(DirectX::XMFLOAT3(vx, 0, vz));

			float dot = DirectX::XMVectorGetX(DirectX::XMQuaternionDot(DirectX::XMLoadFloat4(&myQ.dxFloat4), DirectX::XMVectorScale(DirectX::XMLoadFloat4(&focusQ.dxFloat4), -1)));

			if (dot * dot > 0.99f)
				// AttackNodeへ遷移できる
				return true;
		}
		else
		{
			//攻撃権を申請
			EnemyManager::Instance().SendMessaging(owner_.lock()->GetID(), EnemyManager::AI_ID::AI_INDEX, MESSAGE_TYPE::MsgAskAttackRight);
		}
	}
	return false;
}

// WanderNodeに遷移できるか判定
bool WanderJudgment::Judgment()
{
	if (owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->GetIsPathFlag())return false;

	// 目的地点までのXZ平面での距離判定
	std::shared_ptr<GameObject> ownerObj = owner_.lock()->GetGameObject();
	DirectX::XMFLOAT3 position = ownerObj->transform_->GetWorldPosition();
	DirectX::XMFLOAT3 targetPosition = owner_.lock()->GetTargetPosition();
	float vx = targetPosition.x - position.x;
	float vz = targetPosition.z - position.z;
	float distSq = vx * vx + vz * vz;

	// 目的地から離れている場合
	float radius = 0.5f;
	if (distSq > radius * radius)
	{
		return true;	//到着
	}

	return false;
}

// RouteNodeに遷移できるか判定
bool RoutePathJudgment::Judgment()
{
	if (owner_.lock()->GetIsAttackFlag())return false;
	if (!owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->GetIsNearFlag())return false;

	//敵マネージャー取得
	EnemyManager& enemyManager = EnemyManager::Instance();

	//経路探査の数で判定
	if (enemyManager.GetCurrentNearPathCount() >= enemyManager.GetNearEnemyLevel().togetherPathCount)
		return false;

	//攻撃カウントがいっぱいの時
	if (enemyManager.GetCurrentNearAttackCount() >= enemyManager.GetNearEnemyLevel().togetherAttackCount)
	{
		std::shared_ptr<TransformCom> myTransform = owner_.lock()->GetGameObject()->transform_;

		// 目的地点までのXZ平面での距離判定
		DirectX::XMFLOAT3 position = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 targetPosition = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();

		float vx = targetPosition.x - position.x;
		float vy = targetPosition.y - position.y;
		float vz = targetPosition.z - position.z;
		float dist = sqrtf(vx * vx + vy * vy + vz * vz);

		//接近エリアに入っていたら
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
