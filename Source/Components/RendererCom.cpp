#include "RendererCom.h"

#include "TransformCom.h"
#include <imgui.h>
#include <string>

// �J�n����
void RendererCom::Start()
{
	
}

// �X�V����
void RendererCom::Update(float elapsedTime)
{
	// ���f���̍X�V
	if (model_ != nullptr)
	{
		DirectX::XMFLOAT4X4 transform = GetGameObject()->GetComponent<TransformCom>()->GetWorldTransform();
		model_->UpdateTransform(transform);
	}
}

//GUI�p���O
const char* shaderNames[SHADER_ID::MAX] = { //�V�F�[�_�[��ǉ������炱�����ł��ǉ�����
    "Default",
    "Phong",
    "UnityChanToon",
};
// GUI�`��
void RendererCom::OnGUI()
{
    //�V�F�[�_�[�ς���
    if (ImGui::Button("ShaderSelect.."))
        ImGui::OpenPopup("my_select_popup");
    ImGui::SameLine();
    ImGui::TextUnformatted(shaderNames[shaderID_]);
    if (ImGui::BeginPopup("my_select_popup"))
    {
        for (int i = 0; i < IM_ARRAYSIZE(shaderNames); i++)
        {
            if (ImGui::Selectable(shaderNames[i]))
            {
                shaderID_ = i;
                GameObjectManager::Instance().ChangeShaderID();
            }
        }
        ImGui::EndPopup();
    }

    if (ImGui::TreeNode("UnityChanToon"))
    {
        for (ModelResource::Material& mat : model_->GetResourceShared()->GetMaterialsEdit())
        {
            if (ImGui::TreeNode(mat.name.c_str()))
            {
                if (shaderID_ == 2)
                {
                    DirectX::XMFLOAT4 w = mat.toonStruct._Emissive_Color;
                    if (ImGui::DragFloat4("emissivePower", &w.x, 0.01f, 0, 3))
                    {
                        mat.toonStruct._Emissive_Color = w;
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

    DirectX::XMFLOAT4 color = model_->GetMaterialColor();
    if (ImGui::DragFloat4("materialColor", &color.x, 0.01f, 0, 5))
    {
        model_->SetMaterialColor(color);
    }

}

// ���f���̓ǂݍ���
void RendererCom::LoadModel(const char* filename)
{
	model_ = std::make_unique<Model>(filename);
}
