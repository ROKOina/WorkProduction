#include "ActionDerived.h"

#include "Components/TransformCom.h"
#include "Components/AnimatorCom.h"
#include "Components/AnimationCom.h"
#include "Components/MovementCom.h"
#include "Components/RendererCom.h"
#include "Components/ColliderCom.h"


// �ҋ@�s��
ActionBase::State IdleAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
	{
		//�A�j���[�^�[
		std::shared_ptr<AnimatorCom> animator = owner_->GetGameObject()->GetComponent<AnimatorCom>();
		animator->SetTriggerOn("idle");

		runTimer_ = 2;

		step_++;
		break;
	}
	case 1:
		runTimer_ -= elapsedTime;

		// �ҋ@���Ԃ��߂�����
		if (runTimer_ <= 0.0f)
		{
			owner_->SetRandomTargetPosition();
			step_ = 0;
			return ActionBase::State::Complete;
		}

		// �v���C���[���G����������
		if (owner_->SearchPlayer())
		{
			step_ = 0;
			// �p�j������Ԃ�
			return ActionBase::State::Complete;
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
		//�A�j���[�^�[
		std::shared_ptr<AnimatorCom> animator = owner_->GetGameObject()->GetComponent<AnimatorCom>();
		animator->SetTriggerOn("walk");

		step_++;
		break;
	}
	case 1:
	{
		std::shared_ptr<TransformCom> myTransform = owner_->GetGameObject()->transform_;

		// �ړI�n�_�܂ł�XZ���ʂł̋�������
		DirectX::XMFLOAT3 position = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 targetPosition = owner_->GetTargetPosition();
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
		DirectX::XMStoreFloat3(&force, DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(TPos, Pos)), 30.0f * elapsedTime));

		std::shared_ptr<MovementCom> move = owner_->GetGameObject()->GetComponent<MovementCom>();
		move->AddForce(force);
		
		//��]����
		QuaternionStruct myQ = myTransform->GetRotation();
		QuaternionStruct focusQ = QuaternionStruct::LookRotation(force);
		//�⊮����
		DirectX::XMStoreFloat4(&myQ.dxFloat4, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&myQ.dxFloat4), DirectX::XMLoadFloat4(&focusQ.dxFloat4), 0.1f));
		myTransform->SetRotation(myQ.dxFloat4);

		// �v���C���[���G����������
		if (owner_->SearchPlayer())
		{
			step_ = 0;
			// �p�j������Ԃ�
			return ActionBase::State::Complete;
		}
		break;
	}

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
		// �ڕW�n�_���v���C���[�ʒu�ɐݒ�
		owner_->SetTargetPosition(GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition());
		runTimer_ = 2;

		//�A�j���[�^�[
		std::shared_ptr<AnimatorCom> animator = owner_->GetGameObject()->GetComponent<AnimatorCom>();
		animator->SetTriggerOn("run");

		step_++;
		break;
	}
	case 1:
	{
		std::shared_ptr<TransformCom> myTransform = owner_->GetGameObject()->transform_;

		runTimer_ -= elapsedTime;
		// �ڕW�n�_���v���C���[�ʒu�ɐݒ�
		owner_->SetTargetPosition(GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition());

		// �ړI�n�_�܂ł�XZ���ʂł̋�������
		DirectX::XMFLOAT3 position = myTransform->GetWorldPosition();
		DirectX::XMFLOAT3 targetPosition = owner_->GetTargetPosition();

		// �ړI�n�_�ֈړ�
		DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&position);
		DirectX::XMVECTOR TPos = DirectX::XMLoadFloat3(&targetPosition);
		DirectX::XMFLOAT3 force;
		DirectX::XMStoreFloat3(&force, DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(TPos, Pos)), 40.0f * elapsedTime));

		std::shared_ptr<MovementCom> move = owner_->GetGameObject()->GetComponent<MovementCom>();
		move->AddForce(force);

		//��]����
		QuaternionStruct myQ = myTransform->GetRotation();
		QuaternionStruct focusQ = QuaternionStruct::LookRotation(force);
		//�⊮����
		DirectX::XMStoreFloat4(&myQ.dxFloat4, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&myQ.dxFloat4), DirectX::XMLoadFloat4(&focusQ.dxFloat4), 0.1f));
		myTransform->SetRotation(myQ.dxFloat4);

		float vx = targetPosition.x - position.x;
		float vy = targetPosition.y - position.y;
		float vz = targetPosition.z - position.z;
		float dist = sqrtf(vx * vx + vy * vy + vz * vz);
		// �U���͈͂ɂ���Ƃ�
		if (dist < owner_->GetAttackRange())
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
		break;
	}
	}
	return ActionBase::State::Run;
}

// �U���s��
ActionBase::State AttackAction::Run(float elapsedTime)
{
	switch (step_)
	{
	case 0:
	{
		//�A�j���[�^�[
		std::shared_ptr<AnimatorCom> animator = owner_->GetGameObject()->GetComponent<AnimatorCom>();
		animator->SetTriggerOn("attack");
		step_++;
		break;
	}
	//�U���O�s��
	case 1:
	{		
		//�A�j���[�V����
		std::shared_ptr<AnimationCom> animation = owner_->GetGameObject()->GetComponent<AnimationCom>();
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
			std::shared_ptr<AnimatorCom> animator = owner_->GetGameObject()->GetComponent<AnimatorCom>();
			animator->SetIsStop(true);
			timer_ = 0.3f;

			//��������
			std::shared_ptr<RendererCom> renderer = owner_->GetGameObject()->GetComponent<RendererCom>();
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
		std::shared_ptr<RendererCom> renderer = owner_->GetGameObject()->GetComponent<RendererCom>();
		std::vector<ModelResource::Material>& materials = renderer->GetModel()->GetResourceShared()->GetMaterialsEdit();
		materials[0].toonStruct._Emissive_Color.w += 2.5f * elapsedTime;

		if (timer_ < 0)
		{
			//�A�j���[�^�[
			std::shared_ptr<AnimatorCom> animator = owner_->GetGameObject()->GetComponent<AnimatorCom>();
			animator->SetIsStop(false);

			materials[0].toonStruct._Emissive_Color.w = 0;
			
			owner_->SetIsJustAvoid(true);

			step_++;
		}
		break;
	}
	case 3:
	{
		// �A�j���[�V�������I�����Ă���Ƃ�
		if (!owner_->GetGameObject()->GetComponent<AnimationCom>()->IsPlayAnimation())
		{
			step_ = 0;
			// �U��������Ԃ�
			return ActionBase::State::Complete;
		}
		break;
	}
	case 5:
		// �A�j���[�V�������I�����Ă���Ƃ�
		if (!owner_->GetGameObject()->GetComponent<AnimationCom>()->IsPlayAnimation())
		{
			step_ = 0;
			// �U�����s��Ԃ�
			return ActionBase::State::Failed;
		}
		break;

		//�����I��
	case  Action::End_STEP:
	{
			std::shared_ptr<AnimatorCom> animator = owner_->GetGameObject()->GetComponent<AnimatorCom>();
			animator->SetIsStop(false);

			std::shared_ptr<RendererCom> renderer = owner_->GetGameObject()->GetComponent<RendererCom>();
			std::vector<ModelResource::Material>& materials = renderer->GetModel()->GetResourceShared()->GetMaterialsEdit();
			materials[0].toonStruct._Emissive_Color.w = 0;

			owner_->GetGameObject()->GetChildFind("picolaboAttack")->GetComponent<Collider>()->SetEnabled(false);

			step_ = 0;
	}
	}

	////�_���[�W���󂯂���
	//if (owner_->OnDamageEnemy())
	//{
	//	//�A�j���[�^�[
	//	step_ = 5;
	//	std::shared_ptr<AnimatorCom> animator = owner_->GetGameObject()->GetComponent<AnimatorCom>();
	//	animator->SetIsStop(false);
	//	animator->SetTriggerOn("damage");

	//	std::shared_ptr<RendererCom> renderer = owner_->GetGameObject()->GetComponent<RendererCom>();
	//	std::vector<ModelResource::Material>& materials = renderer->GetModel()->GetResourceShared()->GetMaterialsEdit();
	//	materials[0].toonStruct._Emissive_Color.w = 0;

	//	owner_->GetGameObject()->GetChildFind("picolaboAttack")->GetComponent<Collider>()->SetEnabled(false);
	//}

	// �X�L������Ԃ�
	return ActionBase::State::Run;
}
