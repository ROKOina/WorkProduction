#include "ActionDerived.h"

#include "Components/TransformCom.h"
#include "Components/AnimatorCom.h"
#include "Components/AnimationCom.h"
#include "Components/MovementCom.h"


// 待機行動
ActionBase::State IdleAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
	{
		//アニメーター
		std::shared_ptr<AnimatorCom> animator = owner_->GetGameObject()->GetComponent<AnimatorCom>();
		animator->SetTriggerOn("idle");

		runTimer_ = 2;

		step_++;
		break;
	}
	case 1:
		runTimer_ -= elapsedTime;

		// 待機時間が過ぎた時
		if (runTimer_ <= 0.0f)
		{
			owner_->SetRandomTargetPosition();
			step_ = 0;
			return ActionBase::State::Complete;
		}

		// プレイヤー索敵成功したら
		if (owner_->SearchPlayer())
		{
			step_ = 0;
			// 徘徊成功を返す
			return ActionBase::State::Complete;
		}

		break;
	}
	return ActionBase::State::Run;
}

// 徘徊行動
ActionBase::State WanderAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
		// 徘徊モーション設定
	{
		//アニメーター
		std::shared_ptr<AnimatorCom> animator = owner_->GetGameObject()->GetComponent<AnimatorCom>();
		animator->SetTriggerOn("walk");

		step_++;
		break;
	}
	case 1:
	{
		std::shared_ptr<TransformCom> myTransform = owner_->GetGameObject()->transform_;

		// 目的地点までのXZ平面での距離判定
		DirectX::XMFLOAT3 position = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 targetPosition = owner_->GetTargetPosition();
		float vx = targetPosition.x - position.x;
		float vz = targetPosition.z - position.z;
		float distSq = vx * vx + vz * vz;

		// 目的地へ着いた
		float radius = 0.5f;
		if (distSq < radius * radius)
		{
			step_ = 0;
			// 徘徊成功を返す
			return ActionBase::State::Complete;
		}

		// 目的地点へ移動
		DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&position);
		DirectX::XMVECTOR TPos = DirectX::XMLoadFloat3(&targetPosition);
		DirectX::XMFLOAT3 force;
		DirectX::XMStoreFloat3(&force, DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(TPos, Pos)), 0.3f));

		std::shared_ptr<MovementCom> move = owner_->GetGameObject()->GetComponent<MovementCom>();
		move->AddForce(force);
		
		//回転する
		QuaternionStruct myQ = myTransform->GetRotation();
		QuaternionStruct focusQ = QuaternionStruct::LookRotation(force);
		//補完する
		DirectX::XMStoreFloat4(&myQ.dxFloat4, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&myQ.dxFloat4), DirectX::XMLoadFloat4(&focusQ.dxFloat4), 0.1f));
		myTransform->SetRotation(myQ.dxFloat4);

		// プレイヤー索敵成功したら
		if (owner_->SearchPlayer())
		{
			step_ = 0;
			// 徘徊成功を返す
			return ActionBase::State::Complete;
		}
		break;
	}

	}
	// 実行中
	return ActionBase::State::Run;
}

// 追跡行動
ActionBase::State PursuitAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
	{
		// 目標地点をプレイヤー位置に設定
		owner_->SetTargetPosition(GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition());
		runTimer_ = 2;

		//アニメーター
		std::shared_ptr<AnimatorCom> animator = owner_->GetGameObject()->GetComponent<AnimatorCom>();
		animator->SetTriggerOn("run");

		step_++;
		break;
	}
	case 1:
	{
		std::shared_ptr<TransformCom> myTransform = owner_->GetGameObject()->transform_;

		runTimer_ -= elapsedTime;
		// 目標地点をプレイヤー位置に設定
		owner_->SetTargetPosition(GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition());

		// 目的地点までのXZ平面での距離判定
		DirectX::XMFLOAT3 position = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 targetPosition = owner_->GetTargetPosition();

		// 目的地点へ移動
		DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&position);
		DirectX::XMVECTOR TPos = DirectX::XMLoadFloat3(&targetPosition);
		DirectX::XMFLOAT3 force;
		DirectX::XMStoreFloat3(&force, DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(TPos, Pos)), 0.5f));

		std::shared_ptr<MovementCom> move = owner_->GetGameObject()->GetComponent<MovementCom>();
		move->AddForce(force);

		//回転する
		QuaternionStruct myQ = myTransform->GetRotation();
		QuaternionStruct focusQ = QuaternionStruct::LookRotation(force);
		//補完する
		DirectX::XMStoreFloat4(&myQ.dxFloat4, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&myQ.dxFloat4), DirectX::XMLoadFloat4(&focusQ.dxFloat4), 0.1f));
		myTransform->SetRotation(myQ.dxFloat4);

		float vx = targetPosition.x - position.x;
		float vy = targetPosition.y - position.y;
		float vz = targetPosition.z - position.z;
		float dist = sqrtf(vx * vx + vy * vy + vz * vz);
		// 攻撃範囲にいるとき
		if (dist < owner_->GetAttackRange())
		{
			step_ = 0;
			// 追跡成功を返す
			return ActionBase::State::Complete;
		}
		// 行動時間が過ぎた時
		if (runTimer_ <= 0.0f)
		{
			step_ = 0;
			// 追跡失敗を返す
			return ActionBase::State::Failed;
		}
		break;
	}
	}
	return ActionBase::State::Run;
}

// 攻撃行動
ActionBase::State AttackAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
	{
		//アニメーター
		std::shared_ptr<AnimatorCom> animator = owner_->GetGameObject()->GetComponent<AnimatorCom>();
		animator->SetTriggerOn("kick");

		step_++;
		break;
	}
	case 1:
	{
		// アニメーションが終了しているとき
		if (!owner_->GetGameObject()->GetComponent<AnimationCom>()->IsPlayAnimation())
		{
			step_ = 0;
			// 攻撃成功を返す
			return ActionBase::State::Complete;
		}
		break;
	}
	}
	// スキル中を返す
	return ActionBase::State::Run;
}
