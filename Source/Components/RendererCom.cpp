#include "RendererCom.h"

#include "TransformCom.h"

// 開始処理
void RenderderCom::Start()
{

}

// 更新処理
void RenderderCom::Update(float elapsedTime)
{
	// モデルの更新
	if (model != nullptr)
	{
		DirectX::XMFLOAT4X4 transform = GetGameObject()->GetComponent<TransformCom>()->GetTransform();
		model->UpdateTransform(transform);
		model->UpdateAnimation(elapsedTime);
	}
}

// GUI描画
void RenderderCom::OnGUI()
{

}

// モデルの読み込み
void RenderderCom::LoadModel(const char* filename)
{
	model = std::make_unique<Model>(filename);
}
