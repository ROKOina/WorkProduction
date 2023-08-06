#include "AnimationCom.h"

#include "RendererCom.h"
#include "TransformCom.h"
#include <imgui.h>

// �J�n����
void AnimationCom::Start()
{

}

// �X�V����
void AnimationCom::Update(float elapsedTime)
{
	AnimationUpdata(elapsedTime);
}

// GUI�`��
bool isAnimLoop;
void AnimationCom::OnGUI()
{
	if (!GetGameObject()->GetComponent<RendererCom>())return;

	const ModelResource* resource = GetGameObject()->GetComponent<RendererCom>()->GetModel()->GetResource();
	const std::vector<ModelResource::Animation>& animations = resource->GetAnimations();

	ImGui::Checkbox("animationLoop", &isAnimLoop);
	ImGui::Separator();
	int index = 0;
	for (ModelResource::Animation anim : animations)
	{
		if (ImGui::RadioButton(anim.name.c_str(), false))
			PlayAnimation(index, isAnimLoop);
		index++;
	}

}


//�A�j���[�V�����X�V
void AnimationCom::AnimationUpdata(float elapsedTime)
{	
	//�Đ����łȂ��Ȃ珈�����Ȃ�
	if (!IsPlayAnimation())return;

	//���f�����烊�\�[�X���擾
	Model* model = GetGameObject()->GetComponent<RendererCom>()->GetModel();
	const FbxModelResource* resource = model->GetResource();
	
	//�u�����h���̌v�Z
	float blendRate = 1.0f;
	if (animationBlendTime_ < animationBlendSeconds_)
	{
		animationBlendTime_ += elapsedTime;
		if (animationBlendTime_ >= animationBlendSeconds_)
		{
			animationBlendTime_ = animationBlendSeconds_;
		}
		blendRate = animationBlendTime_ / animationBlendSeconds_;
		blendRate *= blendRate;
	}

	//�w��̃A�j���[�V�����f�[�^���擾
	const std::vector<ModelResource::Animation>& animations = resource->GetAnimations();
	const ModelResource::Animation& animation = animations.at(currentAnimationIndex_);

	//�A�j���[�V�����f�[�^����L�[�t���[���f�[�^���X�g���擾
	const std::vector<ModelResource::Keyframe>& keyframes = animation.keyframes;
	int keyCount = static_cast<int>(keyframes.size());
	for (int keyIndex = 0; keyIndex < keyCount - 1; ++keyIndex)
	{
		//���݂̎��Ԃ��ǂ̃L�[�t���[���̊Ԃɂ��邩���肷��
		const ModelResource::Keyframe& keyframe0 = keyframes.at(keyIndex);
		const ModelResource::Keyframe& keyframe1 = keyframes.at(keyIndex + 1);
		if (currentAnimationSeconds_ >= keyframe0.seconds &&
			currentAnimationSeconds_ < keyframe1.seconds)
		{
			//�Đ����ԂƃL�[�t���[���̎��Ԃ���⊮�����Z�o����
			float rate = (currentAnimationSeconds_ - keyframe0.seconds) /
				(keyframe1.seconds - keyframe0.seconds);

			int nodeCount = static_cast<int>(model->GetNodes().size());
			for (int nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
			{
				//2�̃L�[�t���[���̊Ԃ̕⊮�v�Z
				const ModelResource::NodeKeyData& key0 = keyframe0.nodeKeys.at(nodeIndex);
				const ModelResource::NodeKeyData& key1 = keyframe1.nodeKeys.at(nodeIndex);

				Model::Node& node = model->GetNodes()[nodeIndex];

				//�u�����h��ԏ���
				if (blendRate < 1.0f)
				{
					//���݂̎p���Ǝ��̃L�[�t���[���Ƃ̎p���̕⊮

					//MXVECTOR���g���ĕ⊮����
					DirectX::XMVECTOR Key0Rotate = DirectX::XMLoadFloat4(&node.rotate);
					DirectX::XMVECTOR Key1Rotate = DirectX::XMLoadFloat4(&key0.rotate);
					DirectX::XMVECTOR Key0Translate = DirectX::XMLoadFloat3(&node.translate);
					DirectX::XMVECTOR Key1Translate = DirectX::XMLoadFloat3(&key0.translate);

					DirectX::XMStoreFloat4(&node.rotate, DirectX::XMQuaternionSlerp(Key0Rotate, Key1Rotate, blendRate));
					DirectX::XMStoreFloat3(&node.translate, DirectX::XMVectorLerp(Key0Translate, Key1Translate, blendRate));

				}
				//�ʏ�̌v�Z
				else
				{
					//MXVECTOR���g���ĕ⊮����
					DirectX::XMVECTOR Key0Rotate = DirectX::XMLoadFloat4(&key0.rotate);
					DirectX::XMVECTOR Key1Rotate = DirectX::XMLoadFloat4(&key1.rotate);
					DirectX::XMVECTOR Key0Translate = DirectX::XMLoadFloat3(&key0.translate);
					DirectX::XMVECTOR Key1Translate = DirectX::XMLoadFloat3(&key1.translate);

					DirectX::XMStoreFloat4(&node.rotate, DirectX::XMQuaternionSlerp(Key0Rotate, Key1Rotate, rate));
					DirectX::XMStoreFloat3(&node.translate, DirectX::XMVectorLerp(Key0Translate, Key1Translate, rate));
				}

				//�A�j���[�V�����C�x���g�̍X�V
				for (auto& animEvent : currentAnimationEvents_)
				{
					if (animEvent.nodeIndex != nodeIndex)continue;
					animEvent.enabled = false;
					//�ݒ肳�ꂽ�t���[�����̏ꍇ

					if (animEvent.resourceEventData.startFrame > currentAnimationSeconds_
						|| animEvent.resourceEventData.endFrame < currentAnimationSeconds_)continue;
					//�C�x���g�Đ����ԂȂ�true
					animEvent.enabled = true;

					//���[�g�m�[�h�̏ꍇ��position�𖳎�����continue
					if (nodeIndex == 0)continue;

					//���[���h�|�W�V�����ɕϊ����čX�V����
					DirectX::XMVECTOR WorldPos = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&node.translate), DirectX::XMLoadFloat4x4(&node.parent->worldTransform));
					DirectX::XMStoreFloat3(&animEvent.position, WorldPos);
				}
			}
			break;
		}
	}

	//���[�g�ړ�
	std::vector<ModelResource::RootPosition> rootP = animation.rootPosition;
	int rootKeyCount = static_cast<int>(rootP.size());
	for (int keyIndex = 0; keyIndex < rootKeyCount - 1; ++keyIndex)
	{
		// ���݂̎��Ԃ��ǂ̃L�[�t���[���̊Ԃɂ��邩���肷��
		float frame0 = rootP[keyIndex].frame;
		float frame1 = rootP[keyIndex + 1].frame;
		if (currentAnimationSeconds_ >= frame0 && currentAnimationSeconds_ <= frame1)
		{
			float rate = (currentAnimationSeconds_ - frame0) / (frame1 - frame0);
			// �Q�̃L�[�t���[���Ԃ̕⊮�v�Z
			DirectX::XMVECTOR T0 = DirectX::XMLoadFloat3(&rootP[keyIndex].pos);
			DirectX::XMVECTOR T1 = DirectX::XMLoadFloat3(&rootP[keyIndex + 1].pos);
			DirectX::XMVECTOR T = DirectX::XMVectorLerp(T0, T1, rate);
			DirectX::XMVECTOR TOld = DirectX::XMLoadFloat3(&oldRootPos_);

			//�O��ƍ���̈ړ��l���|�W�V�����ɂ���
			DirectX::XMFLOAT3 dist;
			DirectX::XMMATRIX Mat = DirectX::XMLoadFloat4x4(&GetGameObject()->transform_->GetLocalTransform());
			DirectX::XMStoreFloat3(&dist, DirectX::XMVector3TransformCoord(DirectX::XMVectorSubtract(T, TOld), Mat));

			GetGameObject()->transform_->SetWorldPosition(dist);

			DirectX::XMStoreFloat3(&oldRootPos_, T);	//�����pos��ۑ�

			break;
		}
	}

	//�ŏI�t���[������
	if (animationEndFlag_)
	{
		return;
	}

	//���Ԍo��
	currentAnimationSeconds_ += elapsedTime * animationSpeed_;
	animationBlendTime_ += elapsedTime;

	//�Đ����Ԃ��I�[���Ԃ𒴂�����
	if (currentAnimationSeconds_ >= animation.secondsLength)
	{
		if (animationLoopFlag_)
		{
			//�Đ����Ԃ������߂�
			currentAnimationSeconds_ = 0;
			isLooped_ = true;
			//���[�g
			if (rootKeyCount > 0)
				oldRootPos_ = animation.rootPosition[0].pos;
		}
		else
		{
			animationEndFlag_ = true;
		}
	}
}

//�A�j���[�V�����Đ�
void AnimationCom::PlayAnimation(int index, bool loop, float blendSeconds)
{
	currentAnimationIndex_ = index;
	currentAnimationSeconds_ = 0.0f;
	animationLoopFlag_ = loop;
	animationEndFlag_ = false;
	animationBlendTime_ = 0.0f;
	animationBlendSeconds_ = blendSeconds;
	isLooped_ = false;

	//�A�j���[�V�����C�x���g�ۑ�
	currentAnimationEvents_.clear();
	Model* model = GetGameObject()->GetComponent<RendererCom>()->GetModel();
	const FbxModelResource* resource = model->GetResource();
	const ModelResource::Animation& animation = resource->GetAnimations().at(currentAnimationIndex_);
	//�A�j���[�V�����C�x���g��������R�s�[
	for (auto& animEvent : animation.animationEvents)
	{
		//�e�̃C���f�b�N�X����m�[�h��������
		int nodeIndex = animEvent.eventNode.parentIndex + 1;
		while (1)
		{
			Model::Node& node = model->GetNodes()[nodeIndex];
			//���O�������Ȃ�ۑ�
			if (std::strcmp(node.name, animEvent.eventNode.name.c_str()) == 0)
			{
				AnimEvent currentEvent;
				currentEvent.name = animEvent.name;
				currentEvent.nodeIndex = nodeIndex;
				currentEvent.resourceEventData = animEvent;
				currentAnimationEvents_.emplace_back(currentEvent);
				break;
			}
			nodeIndex++;
			//������Ȃ��ꍇ
			if (nodeIndex >= model->GetNodes().size())break;
		}
	}

	//���[�g
	if (animation.rootPosition.size() > 0)
	{
		oldRootPos_ = animation.rootPosition[0].pos;
	}
}

//�A�j���[�V�����Đ�����
bool AnimationCom::IsPlayAnimation()
{
	if (currentAnimationIndex_ < 0)return false;
	const ModelResource* resource = GetGameObject()->GetComponent<RendererCom>()->GetModel()->GetResource();
	if (currentAnimationIndex_ >= resource->GetAnimations().size())return false;

	if(animationEndFlag_)return false;

	return true;
}

// �A�j���[�V��������
void AnimationCom::ImportFbxAnimation(const char* filename)
{
	//���f�����烊�\�[�X���擾
	std::shared_ptr<FbxModelResource> resource = GetGameObject()->GetComponent<RendererCom>()->GetModel()->GetResourceShared();

	if (resource != nullptr)
	{
		//fbx�ȊO�͒e��
		const char* ext = strrchr(filename, '.');
		if (::_stricmp(ext, ".fbx") == 0)
		{
			resource->AddAnimation(filename);
		}
	}
}

//�A�j���[�V�����C�x���g�擾
bool  AnimationCom::GetCurrentAnimationEvent(const char* eventName, DirectX::XMFLOAT3& position)//�C�x���g�̖��O�ŎQ�Ƃ����|�W�V������Ԃ�
{
	for (auto& animEvent : currentAnimationEvents_)
	{
		if (!animEvent.enabled)continue;
		if (std::strcmp(eventName, animEvent.name.c_str()) != 0)continue;
		position = animEvent.position;
		return true;	//�A�j���[�V�����C�x���g����true
	}
	return false;
}

//�A�j���[�V�����C�x���g�擾(�G���h�t���[����Ȃ�)
bool AnimationCom::GetCurrentAnimationEventIsEnd(const char* eventName)
{
	for (auto& animEvent : currentAnimationEvents_)
	{
		if (animEvent.enabled)continue;
		if (std::strcmp(eventName, animEvent.name.c_str()) != 0)continue;
		if (animEvent.resourceEventData.endFrame > currentAnimationSeconds_)continue;
		return true;	//�A�j���[�V�����C�x���g�̃G���h�t���[����Ȃ�true
	}
	return false;
}
