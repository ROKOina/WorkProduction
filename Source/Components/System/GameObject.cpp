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
		{
			//シェーダーID順にソートして入れる
			int insertShaderID = rendererComponent->GetShaderID();
			int indexSize = renderSortObject_.size();	//最初のサイズを取得
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
	//影描画


	//3D描画
	Render3D();

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

//レンダーオブジェクトをシェーダーID順にソートする
void GameObjectManager::SortRenderObject()
{
	//コムソート
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

//影描画
void GameObjectManager::RenderShadowmap()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	ID3D11RenderTargetView* rtv = nullptr;
	ShaderParameter3D* sp = &graphics.shaderParameter3D_;
	ShadowMapData* shadowData = &sp->shadowMapData;
	ID3D11DepthStencilView* dsv = shadowData->shadowDsvMap.Get();

	// 画面クリア
	dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	// レンダーターゲット設定
	dc->OMSetRenderTargets(0, &rtv, dsv);
	// ビューポートの設定
	D3D11_VIEWPORT	vp = {};
	vp.Width = static_cast<float>(shadowData->width);
	vp.Height = static_cast<float>(shadowData->height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	dc->RSSetViewports(1, &vp);

	//カメラ用に用意
	ShaderParameter3D spCamera;
	//カメラ設定
	{
		// 平行光源からカメラ位置を作成し、そこから原点の位置を見るように視線行列を生成
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

		// シャドウマップに描画したい範囲の射影行列を生成
		DirectX::XMMATRIX P =
			DirectX::XMMatrixOrthographicLH(
				shadowData->shadowRect, shadowData->shadowRect, 0.1f, 1000.0f);
		DirectX::XMStoreFloat4x4(&spCamera.view, V);
		DirectX::XMStoreFloat4x4(&spCamera.projection, P);
		DirectX::XMStoreFloat4x4(&shadowData->lightViewProjection, V * P);
	}

	//描画
	{

	}
}

//3D描画
void GameObjectManager::Render3D()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	//シェーダーIDが変更された時に呼ぶ
	if (isChangeShaderID)
		SortRenderObject();

	// 描画
	int oldShaderID = renderSortObject_[0]->GetShaderID();	//違うシェーダーを使用するため、古いIDを保存
	Shader* shader = graphics.GetShader(static_cast<SHADER_ID>(oldShaderID));
	shader->Begin(dc, graphics.shaderParameter3D_);

	for (std::shared_ptr<RendererCom>& renderObj : renderSortObject_)
	{
		//シェーダーIDが変化したら、シェーダーを変更
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