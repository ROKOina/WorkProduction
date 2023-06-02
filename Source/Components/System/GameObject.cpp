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
	for (std::shared_ptr<Component>& component : components)
	{
		component->Start();
	}
}

// �X�V
void GameObject::Update(float elapsedTime)
{
	for (std::shared_ptr<Component>& component : components)
	{
		component->Update(elapsedTime);
	}
	for (std::weak_ptr<GameObject>& child : childrenObject)
	{
		child.lock()->Update(elapsedTime);
	}

}

// �s��̍X�V
void GameObject::UpdateTransform()
{
	//�e�q�̍s��X�V
	if (parentObject.lock())
	{
		DirectX::XMFLOAT4X4 parentTransform = parentObject.lock()->GetTransform();
		transformCopy->SetParentTransform(parentTransform);
	}

	transformCopy->UpdateTransform();
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
	for (std::shared_ptr<Component>& component : components)
	{
		ImGui::Spacing();
		ImGui::Separator();

		if(ImGui::TreeNode(component->GetName()))
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
	obj->parentObject = shared_from_this();
	//�q���ǉ�
	childrenObject.emplace_back(obj->shared_from_this());

	return obj;
}

//���[�J���|�W�V����
const DirectX::XMFLOAT3 GameObject::GetPosition() const 
{ 
	return transformCopy->GetPosition();

}
void GameObject::SetPosition(const DirectX::XMFLOAT3& position) 
{
	transformCopy->SetPosition(position);
}

//���[���h�|�W�V����
const DirectX::XMFLOAT3 GameObject::GetWorldPosition() const
{
	return transformCopy->GetWorldPosition();
}


//�X�P�[��
const DirectX::XMFLOAT3 GameObject::GetScale() const
{
	return transformCopy->GetScale();
}
void GameObject::SetScale(const DirectX::XMFLOAT3& scale)
{
	transformCopy->SetScale(scale);
}

// ��]
const DirectX::XMFLOAT4& GameObject::GetRotation() const
{
	return transformCopy->GetRotation();
}
void GameObject::SetRotation(const DirectX::XMFLOAT4& rotation)
{
	transformCopy->SetRotation(rotation);
}

//���[���h�g�����X�t�H�[��
const DirectX::XMFLOAT4X4& GameObject::GetTransform() const
{
	return transformCopy->GetTransform();
}
void GameObject::SetTransform(const DirectX::XMFLOAT4X4& transform)
{
	transformCopy->SetTransform(transform);
}

//���[�J���g�����X�t�H�[��(���̓��[�J���ł͂Ȃ�)
const DirectX::XMFLOAT4X4& GameObject::GetParentTransform() const
{
	return transformCopy->GetParentTransform();
}
void GameObject::SetParentTransform(const DirectX::XMFLOAT4X4& parentTransform)
{
	transformCopy->SetParentTransform(parentTransform);
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
	startGameObject.emplace_back(obj);
	return obj;
}

// �폜
void GameObjectManager::Remove(std::shared_ptr<GameObject> obj)
{
	removeGameObject.insert(obj);
}

// �X�V
void GameObjectManager::Update(float elapsedTime)
{
	for (std::shared_ptr<GameObject>& obj : startGameObject)
	{
		obj->Start();
		updateGameObject.emplace_back(obj);
	}
	startGameObject.clear();

	for (std::shared_ptr<GameObject>& obj : updateGameObject)
	{
		obj->Update(elapsedTime);
	}

	for (const std::shared_ptr<GameObject>& obj : removeGameObject)
	{
		std::vector<std::shared_ptr<GameObject>>::iterator itStart = std::find(startGameObject.begin(), startGameObject.end(), obj);
		if (itStart != startGameObject.end())
		{
			startGameObject.erase(itStart);
		}

		std::vector<std::shared_ptr<GameObject>>::iterator itUpdate = std::find(updateGameObject.begin(), updateGameObject.end(), obj);
		if (itUpdate != updateGameObject.end())
		{
			updateGameObject.erase(itUpdate);
		}

		std::set<std::shared_ptr<GameObject>>::iterator itSelection = selectionGameObject.find(obj);
		if (itSelection != selectionGameObject.end())
		{
			selectionGameObject.erase(itSelection);
		}
	}
	removeGameObject.clear();
}

// �s��X�V
void GameObjectManager::UpdateTransform()
{
	for (std::shared_ptr<GameObject>& obj : updateGameObject)
	{
		obj->UpdateTransform();
	}
}

// �`��
void GameObjectManager::Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	Graphics& graphics = Graphics::Instance();
	Shader* shader = graphics.GetShader();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	// �`�揈��
	RenderContext rc;	//�`�悷�邽�߂ɕK�v�ȍ\����

	//�J�����p�����[�^�[�ݒ�
	rc.view = view;
	rc.projection = projection;

	// ���C�g�̕���
	rc.lightDirection = DirectX::XMFLOAT4(0.2f, -0.8f, 0.0f, 0.0f);

	// �`��
	shader->Begin(dc, rc);

	for (std::shared_ptr<GameObject>& obj : updateGameObject)
	{
		// Renderer�R���|�[�l���g������Ε`��
		if (!obj->GetComponent<RenderderCom>())continue;
		Model* model = obj->GetComponent<RenderderCom>()->GetModel();
		if (model != nullptr)
		{
			shader->Draw(dc, model);
		}
	}

	shader->End(dc);

	// ���X�^�[�`��
	DrawLister();

	// �ڍו`��
	DrawDetail();

}

//�Q�[���I�u�W�F�N�g��T��
std::shared_ptr<GameObject> GameObjectManager::Find(const char* name)
{
	for (std::shared_ptr<GameObject>& obj : updateGameObject)
	{
		if (std::strcmp(obj->GetName(), name) == 0)return obj;
	}

	return nullptr;
}

//�q������I�u�W�F�N�g�p�A�ċN�֐�
void ChildCycleDrawLister(std::vector<std::weak_ptr<GameObject>> children, std::set<std::shared_ptr<GameObject>>& selectObject)
{
	for (std::weak_ptr<GameObject>& childWeak : children)
	{
		std::shared_ptr<GameObject> child = childWeak.lock();
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;
		//�I��ł���Ȃ�A�t���O��I�����[�h��
		if (selectObject.find(child) != selectObject.end())
		{
			nodeFlags |= ImGuiTreeNodeFlags_Selected;
		}

		//�q�����Ȃ��Ȃ�A���t���Ȃ�
		if (child->GetChildren().size() == 0)
		{
			nodeFlags |= ImGuiTreeNodeFlags_Leaf;
		}

		//�f�o�b�N�`����e
		if (ImGui::TreeNode(child->GetName(), nodeFlags))
		{

			if (ImGui::IsItemClicked())
			{
				// �P��I�������Ή����Ă���
				selectObject.clear();
				selectObject.insert(child);
			}

			ChildCycleDrawLister(child->GetChildren(), selectObject);

			ImGui::TreePop();
		}
	}
}

// ���X�^�[�`��
void GameObjectManager::DrawLister()
{
	ImGui::SetNextWindowPos(ImVec2(30, 50), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	hiddenLister = !ImGui::Begin("Actor Lister", nullptr, ImGuiWindowFlags_None);
	if (!hiddenLister)
	{
		for (std::shared_ptr<GameObject>& obj : updateGameObject)
		{
			//parent�������continue
			if (obj->GetParent())continue;

			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;

			//�I��ł���Ȃ�A�t���O��I�����[�h��
			if (selectionGameObject.find(obj) != selectionGameObject.end())
			{
				nodeFlags |= ImGuiTreeNodeFlags_Selected;
			}

			//�q�����Ȃ��Ȃ�A���t���Ȃ�
			if (obj->GetChildren().size() == 0)
			{
				nodeFlags |= ImGuiTreeNodeFlags_Leaf;
			}

			//�f�o�b�N�`����e
			//ImGui::TreeNodeEx(obj.get(), nodeFlags, obj->GetName());
			if(ImGui::TreeNode(obj->GetName(), nodeFlags))
			{

				if (ImGui::IsItemClicked())
				{
					// �P��I�������Ή����Ă���
					selectionGameObject.clear();
					selectionGameObject.insert(obj);
				}

				ChildCycleDrawLister(obj->GetChildren(), selectionGameObject);


				ImGui::TreePop();
			}
		}
	}
	ImGui::End();
}

// �ڍו`��
void GameObjectManager::DrawDetail()
{
	ImGui::SetNextWindowPos(ImVec2(950, 50), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	hiddenDetail = !ImGui::Begin("Actor Detail", nullptr, ImGuiWindowFlags_None);
	if (!hiddenDetail)
	{
		std::shared_ptr<GameObject> lastSelected = selectionGameObject.empty() ? nullptr : *selectionGameObject.rbegin();
		if (lastSelected != nullptr)
		{
			lastSelected->OnGUI();
		}
	}
	ImGui::End();
}

