#include <imgui.h>
#include "../../Graphics/Graphics.h"
#include "Component.h"
#include "GameObject.h"
#include "../RendererCom.h"
#include "../TransformCom.h"
#include "../CameraCom.h"
#include "../ColliderCom.h"
#include "GameSource\ScriptComponents\Weapon\WeaponCom.h"
#include "GameSource\ScriptComponents\Weapon\SwordTrailCom.h"
#include "GameSource\ScriptComponents\Enemy\EnemyCom.h"
#include "GameSource\ScriptComponents\Enemy\EnemyManager.h"
#include "GameSource/Math/Collision.h"

//�Q�[���I�u�W�F�N�g
#pragma region GameObject

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
	if (!isEnabled_)return;
	for (std::shared_ptr<Component>& component : components_)
	{
		if (!component->GetEnabled())continue;
		component->Update(elapsedTime);
	}
}

// �s��̍X�V
void GameObject::UpdateTransform()
{
	if (!isEnabled_)return;

	//�e�q�̍s��X�V
	//����R���|�[�l���g���Ȃ���
	if (!this->GetComponent<WeaponCom>())
	{
		//�e�����邩
		if (parentObject_.lock())
		{
			DirectX::XMFLOAT4X4 parentTransform = parentObject_.lock()->transform_->GetWorldTransform();
			transform_->SetParentTransform(parentTransform);
		}
	}

	transform_->UpdateTransform();
}

// GUI�\��
void GameObject::OnGUI()
{
	//�L��
	ImGui::Checkbox(" ", &isEnabled_);
	ImGui::SameLine();

	// ���O
	{
		char buffer[1024];
		::strncpy_s(buffer, sizeof(buffer), GetName(), sizeof(buffer));
		if (ImGui::InputText("Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			SetName(buffer);
		}
	}

	//�I�u�W�F�N�g�X�s�[�h
	ImGui::DragFloat("objSpeed", &objSpeed_, 0.01f);

	// �R���|�[�l���g
	for (std::shared_ptr<Component>& component : components_)
	{
		ImGui::Spacing();
		ImGui::Separator();

		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if(ImGui::TreeNode(component->GetName(), nodeFlags))
		{
			bool enabled = component->GetEnabled();
			if (ImGui::Checkbox(" ", &enabled))
			{
				component->SetEnabled(enabled);
			}
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

#pragma endregion	endGameObject



//�Q�[���I�u�W�F�N�g�}�l�[�W���[
#pragma region GameObjectManager

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

// �S�폜
void GameObjectManager::AllRemove()
{
	for (auto& updateObj : updateGameObject_)
		removeGameObject_.insert(updateObj);
	for (auto& startObj : startGameObject_)
		removeGameObject_.insert(startObj);
}

bool GameObjectManager::EnemyObjFind(std::shared_ptr<GameObject> obj)
{
	if (obj->GetComponent<EnemyCom>())return true;

	if (obj->GetParent())
		return EnemyObjFind(obj->GetParent());

	return false;
}

void GameObjectManager::ThreadEnemyUpdate(int id, float elapsedTime)
{
	enemyGameObject_[id].lock()->Update(elapsedTime);
}


// �X�V
void GameObjectManager::Update(float elapsedTime)
{
	for (std::shared_ptr<GameObject>& obj : startGameObject_)
	{
		//�����_���[�R���|�[�l���g������΃����_�[�I�u�W�F�ɓ����
		std::shared_ptr<RendererCom> rendererComponent = obj->GetComponent<RendererCom>();
		if (rendererComponent) 
		{
			//���f�����X���b�h�ǂݍ���
			rendererComponent->GetModel()->JoinThred();

			//�V�F�[�_�[ID���Ƀ\�[�g���ē����
			int insertShaderID = rendererComponent->GetShaderID();
			int indexSize = static_cast<int>(renderSortObject_.size());	//�ŏ��̃T�C�Y���擾
			for (int indexID = 0; indexID < renderSortObject_.size(); ++indexID)
			{
				if (renderSortObject_[indexID].lock()->GetShaderID() > insertShaderID)
				{
					renderSortObject_.insert(renderSortObject_.begin() + indexID, rendererComponent);
					break;
				}
			}
			//�\�[�g�Ɉ���������Ȃ��Ȃ���ɓ����
			if (indexSize == renderSortObject_.size())
				renderSortObject_.emplace_back(rendererComponent);
		}

		//�g���C���I�u�W�F�N�g������Γ����
		std::shared_ptr<SwordTrailCom> trailComponent = obj->GetComponent<SwordTrailCom>();
		if (trailComponent)
		{
			swordTrailObject_.emplace_back(trailComponent);
		}

		obj->Start();
		updateGameObject_.emplace_back(obj);


		//�����蔻��R���|�[�l���g�ǉ�
		std::shared_ptr<Collider> colliderComponent = obj->GetComponent<Collider>();
		if (colliderComponent)
		{
			colliderObject_.emplace_back(colliderComponent);
		}

		obj->UpdateTransform();
	}
	startGameObject_.clear();

	//�����蔻��
	{
		//����O�̃N���A
		for (auto& col : colliderObject_)
		{
			col.lock()->ColliderStartClear();
		}

		//����
		for (int col1 = 0; col1 < colliderObject_.size(); ++col1)
		{
			if (!colliderObject_[col1].lock()->GetGameObject()->GetEnabled())continue;
			for (int col2 = col1 + 1; col2 < colliderObject_.size(); ++col2)
			{
				if (!colliderObject_[col2].lock()->GetGameObject()->GetEnabled())continue;
				colliderObject_[col1].lock()->ColliderVSOther(colliderObject_[col2].lock());
			}
		}
	}

	enemyGameObject_.clear();
	//�X�V
	for (std::shared_ptr<GameObject>& obj : updateGameObject_)
	{
		if (EnemyObjFind(obj))	//�X���b�h�p
			enemyGameObject_.emplace_back(obj);
		else
			obj->Update(elapsedTime);
	}

	int enemyCount = static_cast<int>(enemyGameObject_.size());
	for (int enemyC = 0; enemyC < enemyCount; ++enemyC)
	{
		future.emplace_back(Graphics::Instance().GetThreadPool()->submit([&](auto id, auto elapsedTime) { return ThreadEnemyUpdate(id, elapsedTime); }, enemyC, elapsedTime));
	}

	for (auto& f : future)
	{
		f.get();
	}
	future.clear();



	//�폜
	for (const std::shared_ptr<GameObject>& obj : removeGameObject_)
	{
		EraseObject(startGameObject_, obj);
		EraseObject(updateGameObject_, obj);

		std::set<std::shared_ptr<GameObject>>::iterator itSelection = selectionGameObject_.find(obj);
		if (itSelection != selectionGameObject_.end())
		{
			selectionGameObject_.erase(itSelection);
		}
	}
	removeGameObject_.clear();

	//�e�I�u�W�F�N�g���(�폜)
	{
		//collider���
		for (int col = 0; col < colliderObject_.size(); ++col)
		{
			if (colliderObject_[col].expired())
			{
				colliderObject_.erase(colliderObject_.begin() + col);
				--col;
			}
		}
		//renderObject���
		for (int ren = 0; ren < renderSortObject_.size(); ++ren)
		{
			if (renderSortObject_[ren].expired())
			{
				renderSortObject_.erase(renderSortObject_.begin() + ren);
				--ren;
			}
		}
		//swordTrailObject���
		for (int tra = 0; tra < swordTrailObject_.size(); ++tra)
		{
			if (swordTrailObject_[tra].expired())
			{
				swordTrailObject_.erase(swordTrailObject_.begin() + tra);
				--tra;
			}
		}

		//EnemyManager�̔z�񂩂�Enemy���
		EnemyManager::Instance().EraseExpiredEnemy();
	}

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

	//�g���C���`��
	SwordTrailRender();

	//debug
	if(1)
	{
		//�����蔻��p�f�o�b�O�`��
		for (auto& col : colliderObject_)
		{
			if (!col.lock()->GetEnabled())continue;
			if (!col.lock()->GetGameObject()->GetEnabled())continue;
			col.lock()->DebugRender();
		}

		// ���X�^�[�`��
		DrawLister();

		// �ڍו`��
		DrawDetail();
	}
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
			if (renderSortObject_[index].lock()->GetShaderID() > renderSortObject_[index + indexOffset].lock()->GetShaderID()) {
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
	if (renderSortObject_.size() <= 0)return;

	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	//�V�F�[�_�[ID���ύX���ꂽ���ɌĂ�
	if (isChangeShaderID_)
		SortRenderObject();

	// �`��
	int oldShaderID = renderSortObject_[0].lock()->GetShaderID();	//�Ⴄ�V�F�[�_�[���g�p���邽�߁A�Â�ID��ۑ�
	Shader* shader = graphics.GetShader(static_cast<SHADER_ID>(oldShaderID));
	shader->Begin(dc, graphics.shaderParameter3D_);

	for (std::weak_ptr<RendererCom>& renderObj : renderSortObject_)
	{
		if (!renderObj.lock()->GetGameObject()->GetEnabled())continue;
		if (!renderObj.lock()->GetEnabled())continue;

		//�V�F�[�_�[ID���ω�������A�V�F�[�_�[��ύX
		int newShaderID = renderObj.lock()->GetShaderID();
		if (oldShaderID != newShaderID)
		{
			shader->End(dc);

			shader = graphics.GetShader(static_cast<SHADER_ID>(newShaderID));
			oldShaderID = newShaderID;

			shader->Begin(dc, graphics.shaderParameter3D_);
		}

		Model* model = renderObj.lock()->GetModel();
		if (model != nullptr)
		{
			shader->Draw(dc, model);
		}
	}

	shader->End(dc);

}

//�g���C���`��
void GameObjectManager::SwordTrailRender()
{
	if (swordTrailObject_.size() <= 0)return;

	for (std::weak_ptr<SwordTrailCom>& trailObj : swordTrailObject_)
	{
		if (!trailObj.lock()->GetGameObject()->GetEnabled())continue;
		if (!trailObj.lock()->GetEnabled())continue;

		trailObj.lock()->Render();

	}

}

//�I�u�W�F�N�g���
void GameObjectManager::EraseObject(std::vector<std::shared_ptr<GameObject>>& objs, std::shared_ptr<GameObject> removeObj)
{
	//�q������
	for (auto& childObj : removeObj->GetChildren())
	{
		EraseObject(objs, childObj.lock());
	}

	std::vector<std::shared_ptr<GameObject>>::iterator it = std::find(objs.begin(), objs.end(), removeObj);
	if (it != objs.end())
	{
		objs.erase(it);
	}
}

#pragma endregion endGameObjectManager