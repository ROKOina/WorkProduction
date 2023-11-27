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

// �ҋ@�s��
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

		// �ҋ@���Ԃ��߂�����
		if (runTimer_ <= 0.0f)
		{
			owner_.lock()->SetRandomTargetPosition();
			step_ = 0;
			return ActionBase::State::Complete;
		}

		// �v���C���[���G����������
		if (owner_.lock()->SearchPlayer())
		{
			step_ = 0;
			// �p�j������Ԃ�
			return ActionBase::State::Complete;
		}

		break;

	//�����I��
	case  Action::END_STEP:
	{
		step_ = 0;
	}
	break;

	}
	return ActionBase::State::Run;
}

// �p�j�s��
ActionBase::State WanderAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
		// �p�j���[�V�����ݒ�
	{
		//�ړ�
		std::shared_ptr<MovementCom> move = owner_.lock()->GetGameObject()->GetComponent<MovementCom>();
		move->SetMoveMaxSpeed(owner_.lock()->GetMoveDataEnemy().walkMaxSpeed);

		step_++;
	}
	break;
	case 1:
	{
		std::shared_ptr<TransformCom> myTransform = owner_.lock()->GetGameObject()->transform_;

		// �ړI�n�_�܂ł�XZ���ʂł̋�������
		DirectX::XMFLOAT3 position = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 targetPosition = owner_.lock()->GetTargetPosition();
		float vx = targetPosition.x - position.x;
		float vz = targetPosition.z - position.z;
		float distSq = vx * vx + vz * vz;

		// �ړI�n�֒�����
		float radius = 0.5f;
		if (distSq < radius * radius)
		{
			step_ = 0;
			// �p�j������Ԃ�
			return ActionBase::State::Complete;
		}

		// �ړI�n�_�ֈړ�
		DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&position);
		DirectX::XMVECTOR TPos = DirectX::XMLoadFloat3(&targetPosition);
		DirectX::XMFLOAT3 force;
		DirectX::XMStoreFloat3(&force, DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(TPos, Pos)), owner_.lock()->GetMoveDataEnemy().walkSpeed));

		std::shared_ptr<MovementCom> move = owner_.lock()->GetGameObject()->GetComponent<MovementCom>();
		move->AddForce(force);

		//��]����
		QuaternionStruct myQ = myTransform->GetRotation();
		QuaternionStruct focusQ = QuaternionStruct::LookRotation(force);
		//�⊮����
		DirectX::XMStoreFloat4(&myQ.dxFloat4, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&myQ.dxFloat4), DirectX::XMLoadFloat4(&focusQ.dxFloat4), 0.1f));
		myTransform->SetRotation(myQ.dxFloat4);

		// �v���C���[���G����������
		if (owner_.lock()->SearchPlayer())
		{
			step_ = 0;
			// �p�j������Ԃ�
			return ActionBase::State::Complete;
		}
	}
	break;
	//�����I��
	case  Action::END_STEP:
	{
		step_ = 0;
	}
	break;

	}
	// ���s��
	return ActionBase::State::Run;
}

// �ǐՍs��
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

		// �ڕW�n�_���v���C���[�ʒu�ɐݒ�
		owner_.lock()->SetTargetPosition(GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition());

		//�^�[�Q�b�g�|�W�V�����Ɉړ�
		owner_.lock()->GoTargetMove();

		// �ړI�n�_�܂ł�XZ���ʂł̋�������
		DirectX::XMFLOAT3 position = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 targetPosition = owner_.lock()->GetTargetPosition();

		float vx = targetPosition.x - position.x;
		float vy = targetPosition.y - position.y;
		float vz = targetPosition.z - position.z;
		float dist = sqrtf(vx * vx + vy * vy + vz * vz);

		//�ߐړG�̏ꍇ
		std::shared_ptr<EnemyNearCom> enemyNear = owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>();
		if (enemyNear)
		{
		//�ڋ߃G���A�ɓ�������
			if (dist < EnemyManager::Instance().GetNearEnemyLevel().radius)
			{
				//�ڋ߃t���O�������Ă��Ȃ��ꍇ
				if (!enemyNear->GetIsNearFlag())
				{
					//�t���O�֌W�Ȃ��ڋ߂��������ꍇ
					if (dist < 3)
						enemyNear->SetIsNearFlag(true);

					//�ڋߐ\��
					EnemyManager::Instance().SendMessaging(owner_.lock()->GetID(), EnemyManager::AI_ID::AI_INDEX, MESSAGE_TYPE::MsgAskNearRight);

					//��U������~
					std::shared_ptr<MovementCom> move = owner_.lock()->GetGameObject()->GetComponent<MovementCom>();
					move->ZeroVelocity();

					// �ҋ@����
					step_ = 1;
					return ActionBase::State::Run;
				}
			}
		}

		// �U���͈͂ɂ���Ƃ�
		if (dist < owner_.lock()->GetAttackRange())
		{					
			//��U������~
			std::shared_ptr<MovementCom> move = owner_.lock()->GetGameObject()->GetComponent<MovementCom>();
			move->ZeroVelocity();

			step_ = 0;
			// �ǐՐ�����Ԃ�
			return ActionBase::State::Complete;
		}

		// �s�����Ԃ��߂�����
		if (runTimer_ <= 0.0f)
		{
			step_ = 0;
			// �ǐՎ��s��Ԃ�
			return ActionBase::State::Failed;
		}



	}
	break;
	//�����I��
	case  Action::END_STEP:
	{
		step_ = 0;
	}
	break;
	}
	return ActionBase::State::Run;
}

//�����s��
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
		
		//�v���C���[�Ƃ͋t�����̃x�N�g�������
		std::shared_ptr<TransformCom> myTransform = owner_.lock()->GetGameObject()->transform_;

		DirectX::XMFLOAT3 myPos = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();

		DirectX::XMFLOAT3 targetPos;

		DirectX::XMStoreFloat3(&targetPos, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&myPos), DirectX::XMLoadFloat3(&playerPos)));

		// �ڕW�n�_���v���C���[�ʒu�ɐݒ�
		owner_.lock()->SetTargetPosition(targetPos);

		//�^�[�Q�b�g�|�W�V�����Ɉړ�
		owner_.lock()->GoTargetMove();

		// �ړI�n�_�܂ł�XZ���ʂł̋�������
		float vx = playerPos.x - myPos.x;
		float vy = playerPos.y - myPos.y;
		float vz = playerPos.z - myPos.z;
		float dist = sqrtf(vx * vx + vy * vy + vz * vz);

		// �����͈͂����傫���ꍇ�͏I��
		std::shared_ptr<EnemyFarCom> farEnemy = owner_.lock()->GetGameObject()->GetComponent<EnemyFarCom>();
		if (dist > farEnemy->GetBackMoveRange() + 1)
		{
			step_ = 0;
			// �ǐՐ�����Ԃ�
			return ActionBase::State::Complete;
		}

		// �s�����Ԃ��߂�����
		if (runTimer_ <= 0.0f)
		{
			step_ = 0;
			// �ǐՎ��s��Ԃ�
			return ActionBase::State::Failed;
		}



	}
	break;
	//�����I��
	case  Action::END_STEP:
	{
		step_ = 0;
	}
	break;
	}
	return ActionBase::State::Run;
}

// �U���o���Ȃ��Ƃ��̓v���C���[���͂ނ悤�Ɉړ�
ActionBase::State RoutePathAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
	{
		//�ړ�
		std::shared_ptr<MovementCom> move = owner_.lock()->GetGameObject()->GetComponent<MovementCom>();
		move->SetMoveMaxSpeed(owner_.lock()->GetMoveDataEnemy().runMaxSpeed);

		for (int i = 0; i < 4; ++i)
			quad_[i] = false;	//0:���� 1:���� 2:�E�� 3:�E��

		runTimer_ = 5;
		step_++;
	}
	break;
	case 1:
	{
		//�v���C���[�̎���̓G�̔z�u���m�F
		DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();
		playerPos.y = 0;
		DirectX::XMFLOAT3 playerUpPos = playerPos;
		playerUpPos.z += 1;
		DirectX::XMVECTOR PlayerUp = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerUpPos), DirectX::XMLoadFloat3(&playerPos)));
		
		//�l�������ċ󂫂�T��
		for (auto& nearEnemy : EnemyManager::Instance().GetNearEnemies())
		{
			if (nearEnemy.enemy.expired())continue;

			//�����̏ꍇ��΂�
			if (nearEnemy.enemy.lock()->GetComponent<EnemyCom>()->GetID() == owner_.lock()->GetID())
				continue;

			//�߂��G�ӊO��΂�
			if (!nearEnemy.enemy.lock()->GetComponent<EnemyNearCom>()->GetIsNearFlag())continue;

			DirectX::XMFLOAT3 enemyPos = nearEnemy.enemy.lock()->transform_->GetWorldPosition();
			enemyPos.y = 0;
			DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&enemyPos), DirectX::XMLoadFloat3(&playerPos));
			PE = DirectX::XMVector3Normalize(PE);
			float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(PlayerUp, PE));
			float cross = DirectX::XMVectorGetY(DirectX::XMVector3Cross(PlayerUp, PE));
			if (dot > 0)
			{
				if (cross > 0)	//�E��
					quad_[2] = true;
				else	//����
					quad_[0] = true;
			}
			else
			{
				if (cross > 0)	//�E��
					quad_[3] = true;
				else	//����
					quad_[1] = true;
			}
		}

		//������ʒu������Ă��Ȃ������牽�����Ȃ�
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
				if (cross > 0)	//�E��
				{
					if (!quad_[2])endAction = true;
				}
				else	//����
				{
					if (!quad_[0])endAction = true;
				}
			}
			else
			{
				if (cross > 0)	//�E��
				{
					if (!quad_[3])endAction = true;
				}
				else	//����
				{
					if (!quad_[1])endAction = true;
				}
			}

			//�G�Ɉ͂܂�Ă����牽�����Ȃ�
			if(!endAction)
			{
				bool isNotRun = true;
				for (int i = 0; i < 4; ++i)	//�G�����邩�m�F
				{
					if (!quad_[i])	//���Ȃ��ꏊ�������false�ɂ���
					{
						isNotRun = false;
						break;
					}
				}
				endAction = isNotRun;
			}

			//���������I���
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
		//�G�}�l�[�W���[�擾
		EnemyManager& enemyManager = EnemyManager::Instance();

		//�T���t���O�̐��Ŕ���
		if (enemyManager.GetCurrentNearPathCount() >= enemyManager.GetNearEnemyLevel().togetherPathCount
			&& !owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->GetIsPathFlag())
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		//�o�H�T���t���OON
		owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->SetIsPathFlag(true);

		//�ړ��o�H��T��
		//����������
		DIRECTION dir= DIRECTION::NONE_DIR;
		int count = 0;	//�G�̉򂩂����݂�
		int quadSave = -1;	//��
		for (int i = 0; i < 4; ++i)
		{
			if (quad_[i])
			{
				//�G�����������ۑ����ăJ�E���g����
				quadSave = i;
				count++;
				continue;
			}
			//�G�����Ȃ����ɔz�u
			if(i==0)
				dir = DIRECTION::DOWN_L;
			if(i==1)
				dir = DIRECTION::UP_L;
			if(i==2)
				dir = DIRECTION::DOWN_R;
			if(i==3)
				dir = DIRECTION::UP_R;
		}
		if (count == 1)	//�P�򂾂����ꍇ
		{
			//���Α��ɔz�u����
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

		//�o�H��ۑ�
		routePos_ = SeachGraph::Instance().SearchEnemySetPos(owner_.lock()->GetGameObject()->transform_->GetWorldPosition(), playerPos, dir);
		//�o�H��������
		pathTimer_ = 1;

		step_++;
	}
	break;
	case 3:
	{
		//�o�H�ړ�
		runTimer_ -= elapsedTime;
		pathTimer_ -= elapsedTime;

		if (routePos_.size() <= 0)
		{
			owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->SetIsPathFlag(false);
			return ActionBase::State::Complete;
		}

		// �ڕW�n�_��ݒ�
		owner_.lock()->SetTargetPosition(routePos_[0]);

		//�^�[�Q�b�g�|�W�V�����Ɉړ�
		owner_.lock()->GoTargetMove();

		// �ړI�n�_�܂ł�XZ���ʂł̋�������
		std::shared_ptr<TransformCom> myTransform = owner_.lock()->GetGameObject()->transform_;
		DirectX::XMFLOAT3 position = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 targetPosition = routePos_[0];

		float vx = targetPosition.x - position.x;
		float vy = targetPosition.y - position.y;
		float vz = targetPosition.z - position.z;
		float dist = sqrtf(vx * vx + vy * vy + vz * vz);

		//�o�H�T���ɐ������Ă��Ȃ������������x�o�H�T��
		if (pathTimer_ < 0)
		{
			step_ = 2;
		}

		//�ڕW�n�_�ɒ�������I���
		if (dist < 0.5f)
		{
			if (routePos_.size() < 2)
			{
				//�o�H�T���t���OOFF
				owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->SetIsPathFlag(false);
				return ActionBase::State::Complete;
			}
			else
				routePos_.erase(routePos_.begin());
		}

		//���ԂŏI���
		if (runTimer_ < 0)
		{				
			//�o�H�T���t���OOFF
			owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->SetIsPathFlag(false);
			return ActionBase::State::Complete;
		}

	}
		break;
	//�����I��
	case  Action::END_STEP:
	{
		//�o�H�T���t���OOFF
		owner_.lock()->GetGameObject()->GetComponent<EnemyNearCom>()->SetIsPathFlag(false);
		step_ = 0;
	}
	break;

	}

	return ActionBase::State::Run;
}

// �ߐڒʏ�U���s��
ActionBase::State AttackAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
	{
		//�A�j���[�^�[
		std::shared_ptr<AnimatorCom> animator = owner_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
		animator->SetTriggerOn("attack");
		//�U�����󂯂Ă��ړ����Ȃ��悤��
		owner_.lock()->GetGameObject()->GetComponent<CharacterStatusCom>()->SetAttackNonMove(true);

		step_++;
		break;
	}
	//�U���O�s��
	case 1:
	{
		//�A�j���[�V����
		std::shared_ptr<AnimationCom> animation = owner_.lock()->GetGameObject()->GetComponent<AnimationCom>();
		//�A�j���[�V�����C�x���g�̒�����U���O�iDoAttack�j�𖼑O��������
		ModelResource::AnimationEvent animEvent = animation->GetAnimationEvent("DoAttack").resourceEventData;
		if (animEvent.name.size() <= 0)
		{
			return ActionBase::State::Failed;
		}

		//DoAttack�𒴂�����
		if (animation->GetCurrentAnimationSecoonds() > animEvent.endFrame)
		{
			//�A�j���[�^�[
			std::shared_ptr<AnimatorCom> animator = owner_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
			animator->SetIsStop(true);
			timer_ = 0.3f;

			//��������
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

		//����
		std::shared_ptr<RendererCom> renderer = owner_.lock()->GetGameObject()->GetComponent<RendererCom>();
		std::vector<ModelResource::Material>& materials = renderer->GetModel()->GetResourceShared()->GetMaterialsEdit();
		materials[0].toonStruct._Emissive_Color.w += 2.5f * elapsedTime;

		if (timer_ < 0)
		{
			//�A�j���[�^�[
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
		// �A�j���[�V�������I�����Ă���Ƃ�
		if (!owner_.lock()->GetGameObject()->GetComponent<AnimationCom>()->IsPlayAnimation())
		{
			//�U���t���O��؂�
			owner_.lock()->SetIsAttackFlag(false);
			owner_.lock()->GetGameObject()->GetComponent<EnemyCom>()->SetIsAttackIdleFlag(false);
			owner_.lock()->GetGameObject()->GetComponent<CharacterStatusCom>()->SetAttackNonMove(false);

			step_ = 0;
			// �U��������Ԃ�
			return ActionBase::State::Complete;
		}
		break;
	}

		//�����I��
	case  Action::END_STEP:
	{
		//�U���t���O��؂�
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

	// �X�L������Ԃ�
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

		// �ҋ@���Ԃ��߂�����
		if (runTimer_ <= 0.0f)
		{
			step_ = 0;
			return ActionBase::State::Complete;
		}

		break;

		//�����I��
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

