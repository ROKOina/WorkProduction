#include "RendererCom.h"

#include "TransformCom.h"

// �J�n����
void RendererCom::Start()
{

}

// �X�V����
void RendererCom::Update(float elapsedTime)
{
	// ���f���̍X�V
	if (model != nullptr)
	{
		DirectX::XMFLOAT4X4 transform = GetGameObject()->GetComponent<TransformCom>()->GetTransform();
		model->UpdateTransform(transform);
	}
}

// GUI�`��
void RendererCom::OnGUI()
{

}

// ���f���̓ǂݍ���
void RendererCom::LoadModel(const char* filename)
{
	model = std::make_unique<Model>(filename);
}
