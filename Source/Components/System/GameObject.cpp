#include <imgui.h>
#include "../../Graphics/Graphics.h"
#include "Component.h"
#include "GameObject.h"
#include "../RendererCom.h"
#include "../TransformCom.h"
#include "../CameraCom.h"

// �J�n����
void GameObject::Start()
{
	for (std::shared_ptr<Component>& component : components_)
	{
		component->Start();
	}
}

// �X�V
void GameObject::Update(float elapsedTime)
{
	for (std::shared_ptr<Component>& component : components_)
	{
		component->Update(elapsedTime);
	}
}

// �s��̍X�V
void GameObject::UpdateTransform()
{
	//�e�q�̍s��X�V
	if (parentObject_.lock())
	{
		DirectX::XMFLOAT4X4 parentTransform = parentObject_.lock()->transform_->GetTransform();
		transform_->SetParentTransform(parentTransform);
	}

	transform_->UpdateTransform();
}

// GUI�\��
void GameObject::OnGUI()
{
	// ���O
	{
		char buffer[1024];
		::strncpy_s(buffer, sizeof(buffer), GetName(), sizeof(buffer));
		if (ImGui::InputText("Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			SetName(buffer);
		}
	}

	// �R���|�[�l���g
	for (std::shared_ptr<Component>& component : components_)
	{
		ImGui::Spacing();
		ImGui::Separator();

		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if(ImGui::TreeNode(component->GetName(), nodeFlags))
		{
			component->OnGUI();
			ImGui::TreePop();
		}
	}
}

//�e�q
std::shared_ptr<GameObject> GameObject::AddChildObject()
{
	std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
	//�e�o�^
	obj->parentObject_ = shared_from_this();
	//�q���ǉ�
	childrenObject_.emplace_back(obj->shared_from_this());

	return obj;
}



///----- �}�l�[�W���[�֐� -----///

// �쐬
std::shared_ptr<GameObject> GameObjectManager::Create()
{
	std::shared_ptr<GameObject> obj = std::make_shared<GameObject>();
	obj->AddComponent<TransformCom>();
	{
		static int id = 0;
		char name[256];
		::sprintf_s(name, sizeof(name), "Actor%d", id++);
		obj->SetName(name);
	}
	startGameObject_.emplace_back(obj);
	return obj;
}

// �폜
void GameObjectManager::Remove(std::shared_ptr<GameObject> obj)
{
	removeGameObject_.insert(obj);
}

// �X�V
void GameObjectManager::Update(float elapsedTime)
{
	for (std::shared_ptr<GameObject>& obj : startGameObject_)
	{
		obj->Start();
		updateGameObject_.emplace_back(obj);

		//�����_���[�R���|�[�l���g������΃����_�[�I�u�W�F�ɓ����
		std::shared_ptr<RendererCom> rendererComponent = obj->GetComponent<RendererCom>();
		if (rendererComponent) 
		{
			//�V�F�[�_�[ID���Ƀ\�[�g���ē����
			int insertShaderID = rendererComponent->GetShaderID();
			int indexSize = renderSortObject_.size();	//�ŏ��̃T�C�Y���擾
			for (int indexID = 0; indexID < renderSortObject_.size(); ++indexID)
			{
				if (renderSortObject_[indexID]->GetShaderID() > insertShaderID)
				{
					renderSortObject_.insert(renderSortObject_.begin() + indexID, rendererComponent);
					break;
				}
			}
			if (indexSize == renderSortObject_.size())
				renderSortObject_.emplace_back(rendererComponent);
		}
	}
	startGameObject_.clear();

	for (std::shared_ptr<GameObject>& obj : updateGameObject_)
	{
		obj->Update(elapsedTime);
	}

	for (const std::shared_ptr<GameObject>& obj : removeGameObject_)
	{
		std::vector<std::shared_ptr<GameObject>>::iterator itStart = std::find(startGameObject_.begin(), startGameObject_.end(), obj);
		if (itStart != startGameObject_.end())
		{
			startGameObject_.erase(itStart);
		}

		std::vector<std::shared_ptr<GameObject>>::iterator itUpdate = std::find(updateGameObject_.begin(), updateGameObject_.end(), obj);
		if (itUpdate != updateGameObject_.end())
		{
			updateGameObject_.erase(itUpdate);
		}

		std::set<std::shared_ptr<GameObject>>::iterator itSelection = selectionGameObject_.find(obj);
		if (itSelection != selectionGameObject_.end())
		{
			selectionGameObject_.erase(itSelection);
		}
	}
	removeGameObject_.clear();
}

// �s��X�V
void GameObjectManager::UpdateTransform()
{
	for (std::shared_ptr<GameObject>& obj : updateGameObject_)
	{
		obj->UpdateTransform();
	}
}

// �`��
void GameObjectManager::Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	//�e�`��


	//3D�`��
	Render3D();

	// ���X�^�[�`��
	DrawLister();

	// �ڍו`��
	DrawDetail();
}

//�Q�[���I�u�W�F�N�g��T��
std::shared_ptr<GameObject> GameObjectManager::Find(const char* name)
{
	for (std::shared_ptr<GameObject>& obj : updateGameObject_)
	{
		if (std::strcmp(obj->GetName(), name) == 0)return obj;
	}

	for (std::shared_ptr<GameObject>& obj : startGameObject_)
	{
		if (std::strcmp(obj->GetName(), name) == 0)return obj;
	}

	return nullptr;
}

//�f�o�b�OGUI�A�ċN�֐�
void CycleDrawLister(std::shared_ptr<GameObject> obj, std::set<std::shared_ptr<GameObject>>& selectObject)
{
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	//�I��ł���Ȃ�A�t���O��I�����[�h��
	if (selectObject.find(obj) != selectObject.end())
	{
		nodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	//�q�����Ȃ��Ȃ�A���t���Ȃ�
	if (obj->GetChildren().size() == 0)
	{
		nodeFlags |= ImGuiTreeNodeFlags_Leaf;
	}

	//�f�o�b�N�`����e
	bool openNode = ImGui::TreeNode(obj->GetName(), nodeFlags);


	if (ImGui::IsItemClicked())
	{
		// �P��I�������Ή����Ă���
		selectObject.clear();
		selectObject.insert(obj);
	}

	if (!openNode)return;

	for (std::weak_ptr<GameObject>& child : obj->GetChildren())
	{
		CycleDrawLister(child.lock(), selectObject);
	}

	ImGui::TreePop();

}

// ���X�^�[�`��
void GameObjectManager::DrawLister()
{
	ImGui::SetNextWindowPos(ImVec2(30, 50), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	//�����z����
	if (ImGui::Begin("3DShaderParamerter", nullptr, ImGuiWindowFlags_None))
	{
		if(ImGui::TreeNode("Sun"))
		{
			DirectX::XMFLOAT4 lVec = Graphics::Instance().shaderParameter3D_.lightDirection;
			if (ImGui::SliderFloat4("lightDirection", &lVec.x, -1, 1))
			{
				lVec.w = 0;
				DirectX::XMStoreFloat4(&Graphics::Instance().shaderParameter3D_.lightDirection, DirectX::XMVector4Normalize(DirectX::XMLoadFloat4(&lVec)));
			}
			DirectX::XMFLOAT4 lCor = Graphics::Instance().shaderParameter3D_.lightColor;
			if (ImGui::SliderFloat4("lightColor", &lCor.x, 0, 1))
			{
				lCor.w = 0;
				DirectX::XMStoreFloat4(&Graphics::Instance().shaderParameter3D_.lightColor, DirectX::XMLoadFloat4(&lCor));
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();

	isHiddenLister_ = !ImGui::Begin("GameObject Lister", nullptr, ImGuiWindowFlags_None);
	if (!isHiddenLister_)
	{
		for (std::shared_ptr<GameObject>& obj : updateGameObject_)
		{
			//parent�������continue
			if (obj->GetParent())continue;

			//�e�qGUI�p�̍ċN�֐�
			CycleDrawLister(obj, selectionGameObject_);
		}
	}
	ImGui::End();
}

// �ڍו`��
void GameObjectManager::DrawDetail()
{
	ImGui::SetNextWindowPos(ImVec2(950, 50), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	isHiddenDetail_ = !ImGui::Begin("GameObject Detail", nullptr, ImGuiWindowFlags_None);
	if (!isHiddenDetail_)
	{
		std::shared_ptr<GameObject> lastSelected = selectionGameObject_.empty() ? nullptr : *selectionGameObject_.rbegin();
		if (lastSelected != nullptr)
		{
			lastSelected->OnGUI();
		}
	}
	ImGui::End();
}

//�����_�[�I�u�W�F�N�g���V�F�[�_�[ID���Ƀ\�[�g����
void GameObjectManager::SortRenderObject()
{
	//�R���\�[�g
	size_t indexOffset = (renderSortObject_.size() * 10) / 13;
	bool is_sorted = false;
	while (!is_sorted) {
		if (indexOffset == 1)is_sorted = true;
		for (size_t index = 0; index < renderSortObject_.size() - indexOffset; ++index) {
			if (renderSortObject_[index]->GetShaderID() > renderSortObject_[index + indexOffset]->GetShaderID()) {
				std::iter_swap(renderSortObject_.begin() + index, renderSortObject_.begin() + (index + indexOffset));
				if (is_sorted)is_sorted = false;
			}
		}
		if (indexOffset > 1) indexOffset = (indexOffset * 10) / 13;
		if (indexOffset == 0) indexOffset = 1;
	}
}

//�e�`��
void GameObjectManager::RenderShadowmap()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	ID3D11RenderTargetView* rtv = nullptr;
	ShaderParameter3D* sp = &graphics.shaderParameter3D_;
	ShadowMapData* shadowData = &sp->shadowMapData;
	ID3D11DepthStencilView* dsv = shadowData->shadowDsvMap.Get();

	// ��ʃN���A
	dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	// �����_�[�^�[�Q�b�g�ݒ�
	dc->OMSetRenderTargets(0, &rtv, dsv);
	// �r���[�|�[�g�̐ݒ�
	D3D11_VIEWPORT	vp = {};
	vp.Width = static_cast<float>(shadowData->width);
	vp.Height = static_cast<float>(shadowData->height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	dc->RSSetViewports(1, &vp);

	//�J�����p�ɗp��
	ShaderParameter3D spCamera;
	//�J�����ݒ�
	{
		// ���s��������J�����ʒu���쐬���A�������猴�_�̈ʒu������悤�Ɏ����s��𐶐�
		DirectX::XMVECTOR LightPosition =
			DirectX::XMLoadFloat3(
				&DirectX::XMFLOAT3(
					sp->lightDirection.x,
					sp->lightDirection.y,
					sp->lightDirection.z));
		LightPosition = DirectX::XMVectorScale(LightPosition, -250.0f);
		DirectX::XMMATRIX V = DirectX::XMMatrixLookAtLH(LightPosition,
			DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

		// �V���h�E�}�b�v�ɕ`�悵�����͈͂̎ˉe�s��𐶐�
		DirectX::XMMATRIX P =
			DirectX::XMMatrixOrthographicLH(
				shadowData->shadowRect, shadowData->shadowRect, 0.1f, 1000.0f);
		DirectX::XMStoreFloat4x4(&spCamera.view, V);
		DirectX::XMStoreFloat4x4(&spCamera.projection, P);
		DirectX::XMStoreFloat4x4(&shadowData->lightViewProjection, V * P);
	}

	//�`��
	{

	}
}

//3D�`��
void GameObjectManager::Render3D()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	//�V�F�[�_�[ID���ύX���ꂽ���ɌĂ�
	if (isChangeShaderID)
		SortRenderObject();

	// �`��
	int oldShaderID = renderSortObject_[0]->GetShaderID();	//�Ⴄ�V�F�[�_�[���g�p���邽�߁A�Â�ID��ۑ�
	Shader* shader = graphics.GetShader(static_cast<SHADER_ID>(oldShaderID));
	shader->Begin(dc, graphics.shaderParameter3D_);

	for (std::shared_ptr<RendererCom>& renderObj : renderSortObject_)
	{
		//�V�F�[�_�[ID���ω�������A�V�F�[�_�[��ύX
		int newShaderID = renderObj->GetShaderID();
		if (oldShaderID != newShaderID)
		{
			shader->End(dc);

			shader = graphics.GetShader(static_cast<SHADER_ID>(newShaderID));
			oldShaderID = newShaderID;

			shader->Begin(dc, graphics.shaderParameter3D_);
		}

		Model* model = renderObj->GetModel();
		if (model != nullptr)
		{
			shader->Draw(dc, model);
		}
	}

	shader->End(dc);

}