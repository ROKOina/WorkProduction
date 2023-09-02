#include "JudgmentDerived.h"

#include "Components/TransformCom.h"

// BattleNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
bool BattleJudgment::Judgment()
{
	// ƒvƒŒƒCƒ„[‚ªŒ©‚Â‚©‚é‚©
	if (owner_->SearchPlayer())
	{
		return true;
	}
	return false;
}

// AttackNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
bool AttackJudgment::Judgment()
{
	std::shared_ptr<TransformCom> myTransform = owner_->GetGameObject()->transform_;

	// ‘ÎÛ‚Æ‚Ì‹——£‚ðŽZo
	DirectX::XMFLOAT3 pos = myTransform->GetWorldPosition();
	DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();

	float vx = playerPos.x - pos.x;
	float vy = playerPos.y - pos.y;
	float vz = playerPos.z - pos.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);

	//‹——£‚ðŒ©‚é
	if (dist < owner_->GetAttackRange())
	{
		//Šp“x‚ðŒ©‚é
		QuaternionStruct myQ = myTransform->GetRotation();
		QuaternionStruct focusQ = QuaternionStruct::LookRotation(DirectX::XMFLOAT3(vx, 0, vz));

		float dot = DirectX::XMVectorGetX(DirectX::XMQuaternionDot(DirectX::XMLoadFloat4(&myQ.dxFloat4), DirectX::XMVectorScale(DirectX::XMLoadFloat4(&focusQ.dxFloat4), -1)));

		if (dot * dot > 0.8f)
			// AttackNode‚Ö‘JˆÚ‚Å‚«‚é
			return true;
	}
	return false;
}

// WanderNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
bool WanderJudgment::Judgment()
{
	// –Ú“I’n“_‚Ü‚Å‚ÌXZ•½–Ê‚Å‚Ì‹——£”»’è
	std::shared_ptr<GameObject> ownerObj = owner_->GetGameObject();
	DirectX::XMFLOAT3 position = ownerObj->transform_->GetWorldPosition();
	DirectX::XMFLOAT3 targetPosition = owner_->GetTargetPosition();
	float vx = targetPosition.x - position.x;
	float vz = targetPosition.z - position.z;
	float distSq = vx * vx + vz * vz;

	// –Ú“I’n‚©‚ç—£‚ê‚Ä‚¢‚éê‡
	float radius = 0.5f;
	if (distSq > radius * radius)
	{
		return true;
	}

	return false;
}
