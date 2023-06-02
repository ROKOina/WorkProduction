#include "RendererCom.h"

#include "TransformCom.h"

// �J�n����
void RenderderCom::Start()
{

}

// �X�V����
void RenderderCom::Update(float elapsedTime)
{
	// ���f���̍X�V
	if (model != nullptr)
	{
		DirectX::XMFLOAT4X4 transform = GetGameObject()->GetComponent<TransformCom>()->GetTransform();
		model->UpdateTransform(transform);
		model->UpdateAnimation(elapsedTime);
	}
}

// GUI�`��
void RenderderCom::OnGUI()
{

}

// ���f���̓ǂݍ���
void RenderderCom::LoadModel(const char* filename)
{
	model = std::make_unique<Model>(filename);
}
