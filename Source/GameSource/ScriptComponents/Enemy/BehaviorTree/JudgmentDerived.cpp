#include "JudgmentDerived.h"

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
bool AttackJudgment::Judgment()
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
		//�p�x������
		QuaternionStruct myQ = myTransform->GetRotation();
		QuaternionStruct focusQ = QuaternionStruct::LookRotation(DirectX::XMFLOAT3(vx, 0, vz));

		float dot = DirectX::XMVectorGetX(DirectX::XMQuaternionDot(DirectX::XMLoadFloat4(&myQ.dxFloat4), DirectX::XMVectorScale(DirectX::XMLoadFloat4(&focusQ.dxFloat4), -1)));

		if (dot * dot > 0.99f)
			// AttackNode�֑J�ڂł���
			return true;
	}
	return false;
}

// WanderNode�ɑJ�ڂł��邩����
bool WanderJudgment::Judgment()
{
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
