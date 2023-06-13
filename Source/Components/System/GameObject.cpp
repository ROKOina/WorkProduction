#include <imgui.h>
#include "../../Graphics/Graphics.h"
#include "Component.h"
#include "GameObject.h"
#include "../RendererCom.h"
#include "../TransformCom.h"
#include "../CameraCom.h"

// 開始処理
void GameObject::Start()
{
	for (std::shared_ptr<Component>& component : components_)
	{
		component->Start();
	}
}

// 更新
void GameObject::Update(float elapsedTime)
{
	for (std::shared_ptr<Component>& component : components_)
	{
		component->Update(elapsedTime);
	}
	for (std::weak_ptr<GameObject>& child : childrenObject_)
	{
		child.lock()->Update(elapsedTime);
	}

}

// 行列の更新
void GameObject::UpdateTransform()
{
	//親子の行列更新
	if (parentObject_.lock())
	{
		DirectX::XMFLOAT4X4 parentTransform = parentObject_.lock()->transform_->GetTransform();
		transform_->SetParentTransform(parentTransform);
	}

	transform_->UpdateTransform();
}

// GUI表示
void GameObject::OnGUI()
{
	// 名前
	{
		char buffer[1024];
		::strncpy_s(buffer, sizeof(buffer), GetName(), sizeof(buffer));
		if (ImGui::InputText("Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			SetName(buffer);
		}
	}

	// コンポーネント
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

//親子
std::shared_ptr<GameObject> GameObject::AddChildObject()
{
	std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
	//親登録
	obj->parentObject_ = shared_from_this();
	//子供追加
	childrenObject_.emplace_back(obj->shared_from_this());

	return obj;
}



///----- マネージャー関数 -----///

// 作成
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

// 削除
void GameObjectManager::Remove(std::shared_ptr<GameObject> obj)
{
	removeGameObject_.insert(obj);
}

// 更新
void GameObjectManager::Update(float elapsedTime)
{
	for (std::shared_ptr<GameObject>& obj : startGameObject_)
	{
		obj->Start();
		updateGameObject_.emplace_back(obj);

		//レンダラーコンポーネントがあればレンダーオブジェに入れる
		std::shared_ptr<RendererCom> rendererComponent = obj->GetComponent<RendererCom>();
		if (rendererComponent)
			renderSortObject_.emplace_back(rendererComponent);
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

// 行列更新
void GameObjectManager::UpdateTransform()
{
	for (std::shared_ptr<GameObject>& obj : updateGameObject_)
	{
		obj->UpdateTransform();
	}
}

// 描画
void GameObjectManager::Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	// 描画処理
	//RenderContext rc_;	//描画するために必要な構造体

	////カメラパラメーター設定
	//rc_.view = view;
	//rc_.projection = projection;

	//// ライトの方向
	//rc_.lightDirection = DirectX::XMFLOAT4(0.2f, -0.8f, 0.0f, 0.0f);

	graphics.rc_.view = view;
	graphics.rc_.projection = projection;
	DirectX::XMFLOAT3 viewPos = Find("Camera")->transform_->GetPosition();
	graphics.rc_.viewPosition = DirectX::XMFLOAT4(viewPos.x, viewPos.y, viewPos.z, 1);

	// 描画
	int oldShaderID = renderSortObject_[0]->GetShaderID();	//違うシェーダーを使用するため、古いIDを保存
	Shader* shader = graphics.GetShader(oldShaderID);
	shader->Begin(dc, graphics.rc_);

	for (std::shared_ptr<RendererCom>& renderObj : renderSortObject_)
	{
		//シェーダーがIDが変化したら、シェーダーを変更
		int newShaderID = renderObj->GetShaderID();
		if (oldShaderID != newShaderID)
		{
			shader->End(dc);

			shader = graphics.GetShader(newShaderID);
			oldShaderID = newShaderID;

			shader->Begin(dc, graphics.rc_);
		}

		Model* model = renderObj->GetModel();
		if (model != nullptr)
		{
			shader->Draw(dc, model);
		}
	}

	shader->End(dc);

	// リスター描画
	DrawLister();

	// 詳細描画
	DrawDetail();

}

//ゲームオブジェクトを探す
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

//デバッグGUI、再起関数
void CycleDrawLister(std::shared_ptr<GameObject> obj, std::set<std::shared_ptr<GameObject>>& selectObject)
{
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	//選んでいるなら、フラグを選択モードに
	if (selectObject.find(obj) != selectObject.end())
	{
		nodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	//子がいないなら、▼付けない
	if (obj->GetChildren().size() == 0)
	{
		nodeFlags |= ImGuiTreeNodeFlags_Leaf;
	}

	//デバック描画内容
	bool openNode = ImGui::TreeNode(obj->GetName(), nodeFlags);


	if (ImGui::IsItemClicked())
	{
		// 単一選択だけ対応しておく
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

// リスター描画
void GameObjectManager::DrawLister()
{
	ImGui::SetNextWindowPos(ImVec2(30, 50), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	//仮太陽方向
	DirectX::XMFLOAT4 l = Graphics::Instance().rc_.lightDirection;
	if (ImGui::SliderFloat4("lightVec", &l.x, -1, 1))
	{
		l.w = 0;
		DirectX::XMStoreFloat4(&Graphics::Instance().rc_.lightDirection, DirectX::XMVector4Normalize(DirectX::XMLoadFloat4(&l)));
	}

	isHiddenLister_ = !ImGui::Begin("GameObject Lister", nullptr, ImGuiWindowFlags_None);
	if (!isHiddenLister_)
	{
		for (std::shared_ptr<GameObject>& obj : updateGameObject_)
		{
			//parentがあればcontinue
			if (obj->GetParent())continue;

			//親子GUI用の再起関数
			CycleDrawLister(obj, selectionGameObject_);
		}
	}
	ImGui::End();
}

// 詳細描画
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

