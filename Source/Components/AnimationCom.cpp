#include "AnimationCom.h"

#include "RendererCom.h"
#include <imgui.h>

// 開始処理
void AnimationCom::Start()
{

}

// 更新処理
void AnimationCom::Update(float elapsedTime)
{
	//再生中でないなら処理しない
	if (!IsPlayAnimation())return;

	//モデルからリソースを取得
	Model* model = GetGameObject()->GetComponent<RendererCom>()->GetModel();
	const FbxModelResource* resource = model->GetResource();

	//ブレンド率の計算
	float blendRate = 1.0f;
	if (animationBlendTime < animationBlendSeconds)
	{
		animationBlendTime += elapsedTime;
		if (animationBlendTime >= animationBlendSeconds)
		{
			animationBlendTime = animationBlendSeconds;
		}
		blendRate = animationBlendTime / animationBlendSeconds;
		blendRate *= blendRate;
	}

	//指定のアニメーションデータを取得
	const std::vector<ModelResource::Animation>& animations = resource->GetAnimations();
	const ModelResource::Animation& animation = animations.at(currentAnimationIndex);

	//アニメーションデータからキーフレームデータリストを取得
	const std::vector<ModelResource::Keyframe>& keyframes = animation.keyframes;
	int keyCount = static_cast<int>(keyframes.size());
	for (int keyIndex = 0; keyIndex < keyCount - 1; ++keyIndex)
	{
		//現在の時間がどのキーフレームの間にいるか判定する
		const ModelResource::Keyframe& keyframe0 = keyframes.at(keyIndex);
		const ModelResource::Keyframe& keyframe1 = keyframes.at(keyIndex + 1);
		if (currentAnimationSeconds >= keyframe0.seconds &&
			currentAnimationSeconds < keyframe1.seconds)
		{
			//再生時間とキーフレームの時間から補完率を算出する
			float rate = (currentAnimationSeconds - keyframe0.seconds) /
				(keyframe1.seconds - keyframe0.seconds);
			
			int nodeCount = static_cast<int>(model->GetNodes().size());
			for (int nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
			{
				//2つのキーフレームの間の補完計算
				const ModelResource::NodeKeyData& key0 = keyframe0.nodeKeys.at(nodeIndex);
				const ModelResource::NodeKeyData& key1 = keyframe1.nodeKeys.at(nodeIndex);

				Model::Node& node = model->GetNodes()[nodeIndex];

				//ブレンド補間処理
				if (blendRate < 1.0f)
				{
					//現在の姿勢と次のキーフレームとの姿勢の補完

					//MXVECTORを使って補完する
					DirectX::XMVECTOR Key0Rotate = DirectX::XMLoadFloat4(&node.rotate);
					DirectX::XMVECTOR Key1Rotate = DirectX::XMLoadFloat4(&key0.rotate);
					DirectX::XMVECTOR Key0Translate = DirectX::XMLoadFloat3(&node.translate);
					DirectX::XMVECTOR Key1Translate = DirectX::XMLoadFloat3(&key0.translate);

					DirectX::XMStoreFloat4(&node.rotate, DirectX::XMQuaternionSlerp(Key0Rotate, Key1Rotate, blendRate));
					DirectX::XMStoreFloat3(&node.translate, DirectX::XMVectorLerp(Key0Translate, Key1Translate, blendRate));

				}
				//通常の計算
				else
				{
					//MXVECTORを使って補完する
					DirectX::XMVECTOR Key0Rotate = DirectX::XMLoadFloat4(&key0.rotate);
					DirectX::XMVECTOR Key1Rotate = DirectX::XMLoadFloat4(&key1.rotate);
					DirectX::XMVECTOR Key0Translate = DirectX::XMLoadFloat3(&key0.translate);
					DirectX::XMVECTOR Key1Translate = DirectX::XMLoadFloat3(&key1.translate);

					DirectX::XMStoreFloat4(&node.rotate, DirectX::XMQuaternionSlerp(Key0Rotate, Key1Rotate, rate));
					DirectX::XMStoreFloat3(&node.translate, DirectX::XMVectorLerp(Key0Translate, Key1Translate, rate));
				}
			}
			break;
		}
	}

	//最終フレーム処理
	if (animationEndFlag)
	{
		animationEndFlag = false;
		currentAnimationIndex = -1;
		return;
	}

	//時間経過
	currentAnimationSeconds += elapsedTime;
	animationBlendTime += elapsedTime;

	//再生時間が終端時間を超えたら
	if (currentAnimationSeconds >= animation.secondsLength)
	{
		if (animationLoopFlag)
		{
			//再生時間を巻き戻す
			currentAnimationSeconds = 0;
		}
		else
		{
			animationEndFlag = true;
		}
	}
}

// GUI描画
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

//アニメーション再生
void AnimationCom::PlayAnimation(int index, bool loop, float blendSeconds)
{
	currentAnimationIndex = index;
	currentAnimationSeconds = 0.0f;
	animationLoopFlag = loop;
	animationEndFlag = false;
	animationBlendTime = 0.0f;
	animationBlendSeconds = blendSeconds;
}

//アニメーション再生中か
bool AnimationCom::IsPlayAnimation()
{
	if (currentAnimationIndex < 0)return false;
	const ModelResource* resource = GetGameObject()->GetComponent<RendererCom>()->GetModel()->GetResource();
	if (currentAnimationIndex >= resource->GetAnimations().size())return false;

	return true;
}

// アニメーション入力
void AnimationCom::ImportFbxAnimation(const char* filename)
{
	//モデルからリソースを取得
	std::shared_ptr<FbxModelResource> resource = GetGameObject()->GetComponent<RendererCom>()->GetModel()->GetResourceShared();

	if (resource != nullptr)
	{
		//fbx以外は弾く
		const char* ext = strrchr(filename, '.');
		if (::_stricmp(ext, ".fbx") == 0)
		{
			resource->AddAnimation(filename);
		}
	}
}
