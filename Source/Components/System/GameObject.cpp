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
	for (std::shared_ptr<Component>& component : components)
	{
		component->Start();
	}
}

// 更新
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

// 行列の更新
void GameObject::UpdateTransform()
{
	//親子の行列更新
	if (parentObject.lock())
	{
		DirectX::XMFLOAT4X4 parentTransform = parentObject.lock()->transform->GetTransform();
		transform->SetParentTransform(parentTransform);
	}

	transform->UpdateTransform();
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
	for (std::shared_ptr<Component>& component : components)
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
	obj->parentObject = shared_from_this();
	//子供追加
	childrenObject.emplace_back(obj->shared_from_this());

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
	startGameObject.emplace_back(obj);
	return obj;
}

// 削除
void GameObjectManager::Remove(std::shared_ptr<GameObject> obj)
{
	removeGameObject.insert(obj);
}

// 更新
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

// 行列更新
void GameObjectManager::UpdateTransform()
{
	for (std::shared_ptr<GameObject>& obj : updateGameObject)
	{
		obj->UpdateTransform();
	}
}

// 描画
void GameObjectManager::Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	Graphics& graphics = Graphics::Instance();
	Shader* shader = graphics.GetShader();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	// 描画処理
	RenderContext rc;	//描画するために必要な構造体

	//カメラパラメーター設定
	rc.view = view;
	rc.projection = projection;

	// ライトの方向
	rc.lightDirection = DirectX::XMFLOAT4(0.2f, -0.8f, 0.0f, 0.0f);

	// 描画
	shader->Begin(dc, rc);

	for (std::shared_ptr<GameObject>& obj : updateGameObject)
	{
		// Rendererコンポーネントがあれば描画
		if (!obj->GetComponent<RenderderCom>())continue;
		Model* model = obj->GetComponent<RenderderCom>()->GetModel();
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
	for (std::shared_ptr<GameObject>& obj : updateGameObject)
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

	hiddenLister = !ImGui::Begin("GameObject Lister", nullptr, ImGuiWindowFlags_None);
	if (!hiddenLister)
	{
		for (std::shared_ptr<GameObject>& obj : updateGameObject)
		{
			//parentがあればcontinue
			if (obj->GetParent())continue;

			//再起関数
			CycleDrawLister(obj, selectionGameObject);
		}
	}
	ImGui::End();
}

// 詳細描画
void GameObjectManager::DrawDetail()
{
	ImGui::SetNextWindowPos(ImVec2(950, 50), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	hiddenDetail = !ImGui::Begin("GameObject Detail", nullptr, ImGuiWindowFlags_None);
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

