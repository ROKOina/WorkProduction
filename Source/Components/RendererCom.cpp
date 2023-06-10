#include "RendererCom.h"

#include "TransformCom.h"

// 開始処理
void RendererCom::Start()
{

}

// 更新処理
void RendererCom::Update(float elapsedTime)
{
	// モデルの更新
	if (model_ != nullptr)
	{
		DirectX::XMFLOAT4X4 transform = GetGameObject()->GetComponent<TransformCom>()->GetTransform();
		model_->UpdateTransform(transform);
	}
}

// GUI描画
void RendererCom::OnGUI()
{

}

// モデルの読み込み
void RendererCom::LoadModel(const char* filename)
{
	model_ = std::make_unique<Model>(filename);
}
