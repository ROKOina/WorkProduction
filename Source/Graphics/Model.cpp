#include "Graphics/Graphics.h"
#include "Graphics/Model.h"
#include "ResourceManager.h"

// コンストラクタ
Model::Model(const char* filename)
{
	// リソース読み込み
	//resource = std::make_shared<ModelResource>();
	//resource->Load(Graphics::Instance().GetDevice(), filename);
	resource = ResourceManager::Instance().LoadModelResource(filename);

	// ノード
	const std::vector<ModelResource::Node>& resNodes = resource->GetNodes();

	nodes.resize(resNodes.size());
	for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
	{
		auto&& src = resNodes.at(nodeIndex);
		auto&& dst = nodes.at(nodeIndex);

		dst.name = src.name.c_str();
		dst.parent = src.parentIndex >= 0 ? &nodes.at(src.parentIndex) : nullptr;
		dst.scale = src.scale;
		dst.rotate = src.rotate;
		dst.translate = src.translate;

		if (dst.parent != nullptr)
		{
			dst.parent->children.emplace_back(&dst);
		}
	}

	// 行列計算
	const DirectX::XMFLOAT4X4 transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	UpdateTransform(transform);
}

// 変換行列計算
void Model::UpdateTransform(const DirectX::XMFLOAT4X4& transform)
{
	DirectX::XMMATRIX Transform = DirectX::XMLoadFloat4x4(&transform);

	for (Node& node : nodes)
	{
		// ローカル行列算出
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotate));
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(node.translate.x, node.translate.y, node.translate.z);
		DirectX::XMMATRIX LocalTransform = S * R * T;

		// ワールド行列算出
		DirectX::XMMATRIX ParentTransform;
		if (node.parent != nullptr)
		{
			ParentTransform = DirectX::XMLoadFloat4x4(&node.parent->worldTransform);
		}
		else
		{
			ParentTransform = Transform;
		}
		DirectX::XMMATRIX WorldTransform = LocalTransform * ParentTransform;

		// 計算結果を格納
		DirectX::XMStoreFloat4x4(&node.localTransform, LocalTransform);
		DirectX::XMStoreFloat4x4(&node.worldTransform, WorldTransform);
	}
}

//ノード検索
Model::Node* Model::FindNode(const char* name)
{
	//全てのノードを総当たりで名前比較する
	for (auto& node : nodes)
	{
		if (strcmp(node.name, name) == 0)
		{
			return &node;
		}
	}

	//見つからなかった
	return nullptr;
}

//アニメーション更新処理
void Model::UpdateAnimetion(float elapsedTime)
{
	//再生中でないなら処理しない
	if (!IsPlayAnimation())return;

	//ブレンド率の計算
	float blendRate = 1.0f;
	//blendRate = animationBlendTime / animationBlendSeconds;
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
			int nodeCount = static_cast<int>(nodes.size());
			for (int nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
			{
				//2つのキーフレームの間の補完計算
				const ModelResource::NodeKeyData& key0 = keyframe0.nodeKeys.at(nodeIndex);
				const ModelResource::NodeKeyData& key1 = keyframe1.nodeKeys.at(nodeIndex);
				
				Node& node = nodes[nodeIndex];

				////補完したい値の差を出す
				//DirectX::XMFLOAT4 rotateSub = { 
				//	key1.rotate.x - key0.rotate.x,
				//	key1.rotate.y - key0.rotate.y,
				//	key1.rotate.z - key0.rotate.z,
				//	key1.rotate.w - key0.rotate.w };
				//DirectX::XMFLOAT3 translateSub = { 
				//	key1.translate.x - key0.translate.x,
				//	key1.translate.y - key0.translate.y,
				//	key1.translate.z - key0.translate.z};

				////補完した値を出す
				//DirectX::XMFLOAT4 rotateComplement = {
				//	rotateSub.x * rate,
				//	rotateSub.y * rate,
				//	rotateSub.z * rate,
				//	rotateSub.w * rate};
				//DirectX::XMFLOAT3 translateComplement = {
				//	translateSub.x * rate,
				//	translateSub.y * rate,
				//	translateSub.z * rate};

				////補間値をノードに反映
				//node.rotate = {
				//	key0.rotate.x + rotateComplement.x,
				//	key0.rotate.y + rotateComplement.y,
				//	key0.rotate.z + rotateComplement.z,
				//	key0.rotate.w + rotateComplement.w };
				//node.translate = {
				//	key0.translate.x + translateComplement.x,
				//	key0.translate.y + translateComplement.y,
				//	key0.translate.z + translateComplement.z };

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
			//currentAnimationSeconds -= animation.secondsLength;
		}
		else
		{
			animationEndFlag = true;
		}
	}
}

//アニメーション再生
void Model::PlayAnimation(int index, bool loop, float blendSeconds)
{
	currentAnimationIndex = index;
	currentAnimationSeconds = 0.0f;
	animationLoopFlag = loop;
	animationEndFlag = false;
	animationBlendTime = 0.0f;
	animationBlendSeconds = blendSeconds;
}

//アニメーション再生中か
bool Model::IsPlayAnimation()const
{
	if (currentAnimationIndex < 0)return false;
	if (currentAnimationIndex >= resource->GetAnimations().size())return false;

	return true;
}
