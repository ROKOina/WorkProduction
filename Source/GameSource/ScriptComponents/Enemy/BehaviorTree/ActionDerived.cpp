#include "ActionDerived.h"

#include "Components/TransformCom.h"
#include "Components/AnimatorCom.h"
#include "Components/AnimationCom.h"
#include "Components/MovementCom.h"
#include "Components/RendererCom.h"
#include "Components/ColliderCom.h"

#include "../EnemyManager.h"
#include "../EnemyNearCom.h"
#include "../EnemyFarCom.h"
#include "../../CharacterStatusCom.h"

#include "GameSource/Stage/PathSearch.h"

// 待機行動
ActionBase::State IdleAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
	{
		runTimer_ = rand() % 200 * 0.01f;

		step_++;
	}
	break;
	case 1:
		runTimer_ -= elapsedTime;

		// 待機時間が過ぎた時
		if (runTimer_ <= 0.0f)
		{
			owner_.lock()->SetRandomTargetPosition();
			step_ = 0;
			return ActionBase::State::Complete;
		}

		// プレイヤー索敵成功したら
		if (owner_.lock()->SearchPlayer())
		{
			step_ = 0;
			// 徘徊成功を返す
			return ActionBase::State::Complete;
		}

		break;

	//強制終了
	case  Action::END_STEP:
	{
		step_ = 0;
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
		//移動
		std::shared_ptr<MovementCom> move = owner_.lock()->GetGameObject()->GetComponent<MovementCom>();
		move->SetMoveMaxSpeed(owner_.lock()->GetMoveDataEnemy().walkMaxSpeed);

		step_++;
	}
	break;
	case 1:
	{
		std::shared_ptr<TransformCom> myTransform = owner_.lock()->GetGameObject()->transform_;

		// 目的地点までのXZ平面での距離判定
		DirectX::XMFLOAT3 position = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 targetPosition = owner_.lock()->GetTargetPosition();
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
		DirectX::XMStoreFloat3(&force, DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(TPos, Pos)), owner_.lock()->GetMoveDataEnemy().walkSpeed));

		std::shared_ptr<MovementCom> move = owner_.lock()->GetGameObject()->GetComponent<MovementCom>();
		move->AddForce(force);

		//回転する
		QuaternionStruct myQ = myTransform->GetRotation();
		QuaternionStruct focusQ = QuaternionStruct::LookRotation(force);
		//補完する
		DirectX::XMStoreFloat4(&myQ.dxFloat4, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&myQ.dxFloat4), DirectX::XMLoadFloat4(&focusQ.dxFloat4), 0.1f));
		myTransform->SetRotation(myQ.dxFloat4);

		// プレイヤー索敵成功したら
		if (owner_.lock()->SearchPlayer())
		{
			step_ = 0;
			// 徘徊成功を返す
			return ActionBase::State::Complete;
		}
	}
	break;
	//強制終了
	case  Action::END_STEP:
	{
		step_ = 0;
	}
	break;

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
		runTimer_ = 2;

		step_++;
	}
	break;
	case 1:
	{
		std::shared_ptr<TransformCom> myTransform = owner_.lock()->GetGameObject()->transform_;

		runTimer_ -= elapsedTime;

		// 目標地点をプレイヤー位置に設定
		owner_.lock()->SetTargetPosition(GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition());

		//ターゲットポジションに移動
		owner_.lock()->GoTargetMove();

		// 目的地点までのXZ平面での距離判定
		DirectX::XMFLOAT3 position = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 targetPosition = owner_.lock()->GetTargetPosition();

		float vx = targetPosition.x - position.x;
		float vy = targetPosition.y - position.y;
		float vz = targetPosition.z - position.z;
		float dist = sqrtf(vx * vx + vy * vy + vz * vz);

		//近接敵の場合
		std::shared_ptr<EnemyNearCom> enemyNear = owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>();
		if (enemyNear)
		{
		//接近エリアに入ったら
			if (dist < EnemyManager::Instance().GetNearEnemyLevel().radius)
			{
				//接近フラグを持っていない場合
				if (!enemyNear->GetIsNearFlag())
				{
					//フラグ関係なく接近しすぎた場合
					if (dist < 3)
						enemyNear->SetIsNearFlag(true);

					//接近申請
					EnemyManager::Instance().SendMessaging(owner_.lock()->GetID(), EnemyManager::AI_ID::AI_INDEX, MESSAGE_TYPE::MsgAskNearRight);

					//一旦加速停止
					std::shared_ptr<MovementCom> move = owner_.lock()->GetGameObject()->GetComponent<MovementCom>();
					move->ZeroVelocity();

					// 待機する
					step_ = 1;
					return ActionBase::State::Run;
				}
			}
		}

		// 攻撃範囲にいるとき
		if (dist < owner_.lock()->GetAttackRange())
		{					
			//一旦加速停止
			std::shared_ptr<MovementCom> move = owner_.lock()->GetGameObject()->GetComponent<MovementCom>();
			move->ZeroVelocity();

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



	}
	break;
	//強制終了
	case  Action::END_STEP:
	{
		step_ = 0;
	}
	break;
	}
	return ActionBase::State::Run;
}

//逃走行動
ActionBase::State BackMoveAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
	{
		runTimer_ = 2;

		step_++;
	}
	break;
	case 1:
	{
		runTimer_ -= elapsedTime;
		
		//プレイヤーとは逆方向のベクトルを作る
		std::shared_ptr<TransformCom> myTransform = owner_.lock()->GetGameObject()->transform_;

		DirectX::XMFLOAT3 myPos = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();

		DirectX::XMFLOAT3 targetPos;

		DirectX::XMStoreFloat3(&targetPos, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&myPos), DirectX::XMLoadFloat3(&playerPos)));

		// 目標地点をプレイヤー位置に設定
		owner_.lock()->SetTargetPosition(targetPos);

		//ターゲットポジションに移動
		owner_.lock()->GoTargetMove();

		// 目的地点までのXZ平面での距離判定
		float vx = playerPos.x - myPos.x;
		float vy = playerPos.y - myPos.y;
		float vz = playerPos.z - myPos.z;
		float dist = sqrtf(vx * vx + vy * vy + vz * vz);

		// 逃走範囲よりも大きい場合は終了
		std::shared_ptr<EnemyFarCom> farEnemy = owner_.lock()->GetGameObject()->GetComponent<EnemyFarCom>();
		if (dist > farEnemy->GetBackMoveRange() + 1)
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



	}
	break;
	//強制終了
	case  Action::END_STEP:
	{
		step_ = 0;
	}
	break;
	}
	return ActionBase::State::Run;
}

// 攻撃出来ないときはプレイヤーを囲むように移動
ActionBase::State RoutePathAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
	{
		//移動
		std::shared_ptr<MovementCom> move = owner_.lock()->GetGameObject()->GetComponent<MovementCom>();
		move->SetMoveMaxSpeed(owner_.lock()->GetMoveDataEnemy().runMaxSpeed);

		for (int i = 0; i < 4; ++i)
			quad_[i] = false;	//0:左上 1:左下 2:右上 3:右下

		runTimer_ = 5;
		step_++;
	}
	break;
	case 1:
	{
		//プレイヤーの周りの敵の配置を確認
		DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();
		playerPos.y = 0;
		DirectX::XMFLOAT3 playerUpPos = playerPos;
		playerUpPos.z += 1;
		DirectX::XMVECTOR PlayerUp = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerUpPos), DirectX::XMLoadFloat3(&playerPos)));
		
		//四方を見て空きを探す
		for (auto& nearEnemy : EnemyManager::Instance().GetNearEnemies())
		{
			if (nearEnemy.enemy.expired())continue;

			//自分の場合飛ばす
			if (nearEnemy.enemy.lock()->GetComponent<EnemyCom>()->GetID() == owner_.lock()->GetID())
				continue;

			//近い敵意外飛ばす
			if (!nearEnemy.enemy.lock()->GetComponent<EnemyNearCom>()->GetIsNearFlag())continue;

			DirectX::XMFLOAT3 enemyPos = nearEnemy.enemy.lock()->transform_->GetWorldPosition();
			enemyPos.y = 0;
			DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&enemyPos), DirectX::XMLoadFloat3(&playerPos));
			PE = DirectX::XMVector3Normalize(PE);
			float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(PlayerUp, PE));
			float cross = DirectX::XMVectorGetY(DirectX::XMVector3Cross(PlayerUp, PE));
			if (dot > 0)
			{
				if (cross > 0)	//右上
					quad_[2] = true;
				else	//左上
					quad_[0] = true;
			}
			else
			{
				if (cross > 0)	//右下
					quad_[3] = true;
				else	//左下
					quad_[1] = true;
			}
		}

		//今いる位置が被っていなかったら何もしない
		{
			DirectX::XMFLOAT3 enemyPos = owner_.lock()->GetGameObject()->transform_->GetWorldPosition();
			enemyPos.y = 0;
			DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&enemyPos), DirectX::XMLoadFloat3(&playerPos));
			PE = DirectX::XMVector3Normalize(PE);
			float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(PlayerUp, PE));
			float cross = DirectX::XMVectorGetY(DirectX::XMVector3Cross(PlayerUp, PE));
			bool endAction = false;
			if (dot > 0)
			{
				if (cross > 0)	//右上
				{
					if (!quad_[2])endAction = true;
				}
				else	//左上
				{
					if (!quad_[0])endAction = true;
				}
			}
			else
			{
				if (cross > 0)	//右下
				{
					if (!quad_[3])endAction = true;
				}
				else	//左下
				{
					if (!quad_[1])endAction = true;
				}
			}

			//敵に囲まれていたら何もしない
			if(!endAction)
			{
				bool isNotRun = true;
				for (int i = 0; i < 4; ++i)	//敵がいるか確認
				{
					if (!quad_[i])	//いない場所があればfalseにする
					{
						isNotRun = false;
						break;
					}
				}
				endAction = isNotRun;
			}

			//何もせず終わる
			if (endAction)
			{
				step_ = 0;
				return ActionBase::State::Complete;
			}
		}
		
		step_++;
	}
	break;
	case 2:
	{	
		//敵マネージャー取得
		EnemyManager& enemyManager = EnemyManager::Instance();

		//探索フラグの数で判定
		if (enemyManager.GetCurrentNearPathCount() >= enemyManager.GetNearEnemyLevel().togetherPathCount
			&& !owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->GetIsPathFlag())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		//経路探索フラグON
		owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->SetIsPathFlag(true);

		//移動経路を探索
		//方向を見る
		DIRECTION dir= DIRECTION::NONE_DIR;
		int count = 0;	//敵の塊かたをみる
		int quadSave = -1;	//塊
		for (int i = 0; i < 4; ++i)
		{
			if (quad_[i])
			{
				//敵がいる方向を保存してカウントする
				quadSave = i;
				count++;
				continue;
			}
			//敵がいない方に配置
			if(i==0)
				dir = DIRECTION::DOWN_L;
			if(i==1)
				dir = DIRECTION::UP_L;
			if(i==2)
				dir = DIRECTION::DOWN_R;
			if(i==3)
				dir = DIRECTION::UP_R;
		}
		if (count == 1)	//１塊だった場合
		{
			//反対側に配置する
			if(quadSave==0)
				dir = DIRECTION::UP_R;
			if(quadSave==1)
				dir = DIRECTION::DOWN_R;
			if(quadSave==2)
				dir = DIRECTION::UP_L;
			if(quadSave==3)
				dir = DIRECTION::DOWN_L;
		}

		DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();
		playerPos.y = 0;

		//経路を保存
		routePos_ = SeachGraph::Instance().SearchEnemySetPos(owner_.lock()->GetGameObject()->transform_->GetWorldPosition(), playerPos, dir);
		//経路制限時間
		pathTimer_ = 1;

		step_++;
	}
	break;
	case 3:
	{
		//経路移動
		runTimer_ -= elapsedTime;
		pathTimer_ -= elapsedTime;

		if (routePos_.size() <= 0)
		{
			owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->SetIsPathFlag(false);
			return ActionBase::State::Complete;
		}

		// 目標地点を設定
		owner_.lock()->SetTargetPosition(routePos_[0]);

		//ターゲットポジションに移動
		owner_.lock()->GoTargetMove();

		// 目的地点までのXZ平面での距離判定
		std::shared_ptr<TransformCom> myTransform = owner_.lock()->GetGameObject()->transform_;
		DirectX::XMFLOAT3 position = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 targetPosition = routePos_[0];

		float vx = targetPosition.x - position.x;
		float vy = targetPosition.y - position.y;
		float vz = targetPosition.z - position.z;
		float dist = sqrtf(vx * vx + vy * vy + vz * vz);

		//経路探査に成功していなかったらもう一度経路探査
		if (pathTimer_ < 0)
		{
			step_ = 2;
		}

		//目標地点に着いたら終わる
		if (dist < 0.5f)
		{
			if (routePos_.size() < 2)
			{
				//経路探索フラグOFF
				owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->SetIsPathFlag(false);
				return ActionBase::State::Complete;
			}
			else
				routePos_.erase(routePos_.begin());
		}

		//時間で終わる
		if (runTimer_ < 0)
		{				
			//経路探索フラグOFF
			owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->SetIsPathFlag(false);
			return ActionBase::State::Complete;
		}

	}
		break;
	//強制終了
	case  Action::END_STEP:
	{
		//経路探索フラグOFF
		owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->SetIsPathFlag(false);
		step_ = 0;
	}
	break;

	}

	return ActionBase::State::Run;
}

// 近接通常攻撃行動
ActionBase::State AttackAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
	{
		//アニメーター
		std::shared_ptr<AnimatorCom> animator = owner_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
		animator->SetTriggerOn("attack");
		//攻撃を受けても移動しないように
		owner_.lock()->GetGameObject()->GetComponent<CharacterStatusCom>()->SetAttackNonMove(true);

		step_++;
		break;
	}
	//攻撃前行動
	case 1:
	{
		//アニメーション
		std::shared_ptr<AnimationCom> animation = owner_.lock()->GetGameObject()->GetComponent<AnimationCom>();
		//アニメーションイベントの中から攻撃前（DoAttack）を名前検索する
		ModelResource::AnimationEvent animEvent = animation->GetAnimationEvent("DoAttack").resourceEventData;
		if (animEvent.name.size() <= 0)
		{
			return ActionBase::State::Failed;
		}

		//DoAttackを超えた時
		if (animation->GetCurrentAnimationSecoonds() > animEvent.endFrame)
		{
			//アニメーター
			std::shared_ptr<AnimatorCom> animator = owner_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
			animator->SetIsStop(true);
			timer_ = 0.3f;

			//発光準備
			std::shared_ptr<RendererCom> renderer = owner_.lock()->GetGameObject()->GetComponent<RendererCom>();
			std::vector<ModelResource::Material>& materials = renderer->GetModel()->GetResourceShared()->GetMaterialsEdit();
			materials[0].toonStruct._Emissive_Color.w = 0.3f;

			step_++;
		}
		break;
	}
	case 2:
	{
		timer_ -= elapsedTime;

		//発光
		std::shared_ptr<RendererCom> renderer = owner_.lock()->GetGameObject()->GetComponent<RendererCom>();
		std::vector<ModelResource::Material>& materials = renderer->GetModel()->GetResourceShared()->GetMaterialsEdit();
		materials[0].toonStruct._Emissive_Color.w += 2.5f * elapsedTime;

		if (timer_ < 0)
		{
			//アニメーター
			std::shared_ptr<AnimatorCom> animator = owner_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
			animator->SetIsStop(false);

			materials[0].toonStruct._Emissive_Color.w = 0;
			
			owner_.lock()->SetIsJustAvoid(true);

			step_++;
		}
		break;
	}
	case 3:
	{
		// アニメーションが終了しているとき
		if (!owner_.lock()->GetGameObject()->GetComponent<AnimationCom>()->IsPlayAnimation())
		{
			//攻撃フラグを切る
			owner_.lock()->SetIsAttackFlag(false);
			owner_.lock()->GetGameObject()->GetComponent<EnemyCom>()->SetIsAttackIdleFlag(false);
			owner_.lock()->GetGameObject()->GetComponent<CharacterStatusCom>()->SetAttackNonMove(false);

			step_ = 0;
			// 攻撃成功を返す
			return ActionBase::State::Complete;
		}
		break;
	}

		//強制終了
	case  Action::END_STEP:
	{
		//攻撃フラグを切る
		owner_.lock()->SetIsAttackFlag(false);
		owner_.lock()->GetGameObject()->GetComponent<EnemyCom>()->SetIsAttackIdleFlag(false);
		owner_.lock()->GetGameObject()->GetComponent<CharacterStatusCom>()->SetAttackNonMove(false);

		std::shared_ptr<AnimatorCom> animator = owner_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
		animator->SetIsStop(false);

		std::shared_ptr<RendererCom> renderer = owner_.lock()->GetGameObject()->GetComponent<RendererCom>();
		std::vector<ModelResource::Material>& materials = renderer->GetModel()->GetResourceShared()->GetMaterialsEdit();
		materials[0].toonStruct._Emissive_Color.w = 0;

		step_ = 0;
	}
	break;
	}

	// スキル中を返す
	return ActionBase::State::Run;
}

ActionBase::State AttackIdleAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
	{
		owner_.lock()->GetGameObject()->GetComponent<EnemyCom>()->SetIsAttackIdleFlag(true);
		runTimer_ = rand() % 200 * 0.01f;

		step_++;
	}
	break;
	case 1:
		runTimer_ -= elapsedTime;

		// 待機時間が過ぎた時
		if (runTimer_ <= 0.0f)
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		break;

		//強制終了
	case  Action::END_STEP:
	{
		owner_.lock()->SetIsAttackFlag(false);
		owner_.lock()->GetGameObject()->GetComponent<EnemyCom>()->SetIsAttackIdleFlag(false);

		step_ = 0;
	}
	break;

	}
	return ActionBase::State::Run;
}

