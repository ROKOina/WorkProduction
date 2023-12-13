#include <imgui.h>
#include "../../Graphics/Graphics.h"
#include "Component.h"
#include "GameObject.h"
#include "../RendererCom.h"
#include "../TransformCom.h"
#include "../CameraCom.h"
#include "../ColliderCom.h"
#include "../ParticleSystemCom.h"
#include "GameSource\ScriptComponents\Weapon\WeaponCom.h"
#include "GameSource\ScriptComponents\Weapon\SwordTrailCom.h"
#include "GameSource\ScriptComponents\Enemy\EnemyCom.h"
#include "GameSource\ScriptComponents\Enemy\EnemyManager.h"
#include "GameSource/Math/Collision.h"

//ゲームオブジェクト
#pragma region GameObject

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
	if (!isEnabled_)return;
	for (std::shared_ptr<Component>& component : components_)
	{
		if (!component->GetEnabled())continue;
		component->Update(elapsedTime);
	}
}

// 行列の更新
void GameObject::UpdateTransform()
{
	if (!isEnabled_)return;

	//親子の行列更新
	//武器コンポーネントがないか
	if (!this->GetComponent<WeaponCom>())
	{
		//親がいるか
		if (parentObject_.lock())
		{
			DirectX::XMFLOAT4X4 parentTransform = parentObject_.lock()->transform_->GetWorldTransform();
			transform_->SetParentTransform(parentTransform);
		}
	}

	transform_->UpdateTransform();
}

// GUI表示
void GameObject::OnGUI()
{
	//有効
	ImGui::Checkbox(" ", &isEnabled_);
	ImGui::SameLine();

	// 名前
	{
		char buffer[1024];
		::strncpy_s(buffer, sizeof(buffer), GetName(), sizeof(buffer));
		if (ImGui::InputText("Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			SetName(buffer);
		}
	}

	//オブジェクトスピード
	ImGui::DragFloat("objSpeed", &objSpeed_, 0.01f);

	// コンポーネント
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

void GameObject::Render2D(float elapsedTime)
{
	// コンポーネント
	for (std::shared_ptr<Component>& component : components_)
	{
		component->Render2D(elapsedTime);
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

void GameObject::EraseExpiredChild()
{
	for (int childCount = 0; childCount < childrenObject_.size(); ++childCount)
	{
		if (childrenObject_[childCount].expired())
		{
			childrenObject_.erase(childrenObject_.begin() + childCount);
			--childCount;
		}
	}
}

void GameObject::AudioRelease()
{
	for (auto& com : components_)
	{
		com->AudioRelease();
	}
}

#pragma endregion	endGameObject



//ゲームオブジェクトマネージャー
#pragma region GameObjectManager

// 作成
std::shared_ptr<GameObject> GameObjectManager::Create()
{
	std::lock_guard<std::mutex> lock(mutex_);

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

// 全削除
void GameObjectManager::AllRemove()
{
	for (auto& updateObj : updateGameObject_)
		Remove(updateObj);
	for (auto& startObj : startGameObject_)
		Remove(startObj);
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


// 更新
void GameObjectManager::Update(float elapsedTime)
{
	for (std::shared_ptr<GameObject>& obj : startGameObject_)
	{
		if (!obj)continue;

		//レンダラーコンポーネントがあればレンダーオブジェに入れる
		std::shared_ptr<RendererCom> rendererComponent = obj->GetComponent<RendererCom>();
		if (rendererComponent)
		{
			//モデルをスレッド読み込み
			rendererComponent->GetModel()->JoinThred();

			//シェーダーID順にソートして入れる
			int insertShaderID = rendererComponent->GetShaderID();
			int indexSize = static_cast<int>(renderSortObject_.size());	//最初のサイズを取得
			for (int indexID = 0; indexID < renderSortObject_.size(); ++indexID)
			{
				if (renderSortObject_[indexID].lock()->GetShaderID() > insertShaderID)
				{
					renderSortObject_.insert(renderSortObject_.begin() + indexID, rendererComponent);
					break;
				}
			}
			//ソートに引っかからないなら後ろに入れる
			if (indexSize == renderSortObject_.size())
				renderSortObject_.emplace_back(rendererComponent);
		}

		//トレイルオブジェクトがあれば入れる
		std::shared_ptr<SwordTrailCom> trailComponent = obj->GetComponent<SwordTrailCom>();
		if (trailComponent)
		{
			swordTrailObject_.emplace_back(trailComponent);
		}

		//パーティクルオブジェクトがあれば入れる
		std::shared_ptr<ParticleSystemCom> particleComponent = obj->GetComponent<ParticleSystemCom>();
		if (particleComponent)
		{
			particleObject_.emplace_back(particleComponent);
		}

		obj->Start();
		updateGameObject_.emplace_back(obj);


		//当たり判定コンポーネント追加
		std::shared_ptr<Collider> colliderComponent = obj->GetComponent<Collider>();
		if (colliderComponent)
		{
			colliderObject_.emplace_back(colliderComponent);
		}

		obj->UpdateTransform();
	}
	startGameObject_.clear();

	//当たり判定
	{
		//判定前のクリア
		for (auto& col : colliderObject_)
		{
			col.lock()->ColliderStartClear();
		}

		//判定
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
	//更新
	for (std::shared_ptr<GameObject>& obj : updateGameObject_)
	{
		if (EnemyObjFind(obj))	//スレッド用
			enemyGameObject_.emplace_back(obj);
		else
			obj->Update(elapsedTime);
	}

	int enemyCount = static_cast<int>(enemyGameObject_.size());
	if (EnemyManager::Instance().GetIsUpdateFlag())	//更新フラグがOnの時
	{
		for (int enemyC = 0; enemyC < enemyCount; ++enemyC)
		{
			future.emplace_back(Graphics::Instance().GetThreadPool()->submit([&](auto id, auto elapsedTime) { return ThreadEnemyUpdate(id, elapsedTime); }, enemyC, elapsedTime));
		}
	}

	for (auto& f : future)
	{
		f.get();
	}
	future.clear();



	//削除
	std::vector<std::weak_ptr<GameObject>> parentObj;
	for (const std::shared_ptr<GameObject>& obj : removeGameObject_)
	{
		EraseObject(startGameObject_, obj);
		EraseObject(updateGameObject_, obj);

		std::set<std::shared_ptr<GameObject>>::iterator itSelection = selectionGameObject_.find(obj);
		if (itSelection != selectionGameObject_.end())
		{
			selectionGameObject_.erase(itSelection);
		}

		if (obj->GetParent())
			parentObj.emplace_back(obj->GetParent());
	}
	removeGameObject_.clear();



	//各オブジェクト解放(削除)
	{
		//child解放
		for (std::weak_ptr<GameObject> parent : parentObj)
		{
			if (!parent.expired())
				parent.lock()->EraseExpiredChild();
		}

		//collider解放
		for (int col = 0; col < colliderObject_.size(); ++col)
		{
			if (colliderObject_[col].expired())
			{
				colliderObject_.erase(colliderObject_.begin() + col);
				--col;
			}
		}
		//renderObject解放
		for (int ren = 0; ren < renderSortObject_.size(); ++ren)
		{
			if (renderSortObject_[ren].expired())
			{
				renderSortObject_.erase(renderSortObject_.begin() + ren);
				--ren;
			}
		}
		//swordTrailObject解放
		for (int tra = 0; tra < swordTrailObject_.size(); ++tra)
		{
			if (swordTrailObject_[tra].expired())
			{
				swordTrailObject_.erase(swordTrailObject_.begin() + tra);
				--tra;
			}
		}

		//particleObject解放
		for (int per = 0; per < particleObject_.size(); ++per)
		{
			if (particleObject_[per].expired())
			{
				particleObject_.erase(particleObject_.begin() + per);
				--per;
			}
		}

		//EnemyManagerの配列からEnemy解放
		EnemyManager::Instance().EraseExpiredEnemy();
	}

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
	RenderShadowmap();

	//3D描画
	Render3D();

	//パーティクル描画
	ParticleRender();

	//トレイル描画
	SwordTrailRender();

	//debug
	if(Graphics::Instance().IsDebugGUI())
	{
		//当たり判定用デバッグ描画
		for (auto& col : colliderObject_)
		{
			if (!col.lock()->GetEnabled())continue;
			if (!col.lock()->GetGameObject()->GetEnabled())continue;
			col.lock()->DebugRender();
		}

		// リスター描画
		DrawLister();

		// 詳細描画
		DrawDetail();
	}
}

void GameObjectManager::Render2D(float elapsedTime)
{
	for (std::shared_ptr<GameObject>& obj : updateGameObject_)
	{
		obj->Render2D(elapsedTime);
	}
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
			if (renderSortObject_[index].lock()->GetShaderID() > renderSortObject_[index + indexOffset].lock()->GetShaderID()) {
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
	ID3D11DepthStencilView* dsv = shadowData->shadowmapDepthStencil->depthStencilView.Get();

	// 画面クリア
	dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	// レンダーターゲット設定
	dc->OMSetRenderTargets(0, &rtv, dsv);
	// ビューポートの設定
	D3D11_VIEWPORT	vp = {};
	vp.Width = static_cast<float>(shadowData->shadowmapDepthStencil->width);
	vp.Height = static_cast<float>(shadowData->shadowmapDepthStencil->height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	dc->RSSetViewports(1, &vp);

	//カメラ用に用意
	ShaderParameter3D spCamera;
	//カメラ設定
	{
		// 平行光源からカメラ位置を作成し、そこから原点の位置を見るように視線行列を生成
		DirectX::XMVECTOR LightPosition =
			//DirectX::XMLoadFloat3(
			//	&DirectX::XMFLOAT3(
			//		sp->lightDirection.x,
			//		sp->lightDirection.y,
			//		sp->lightDirection.z));
			{ 0.0f, -1.0f, 0.0f };

		LightPosition = DirectX::XMVectorScale(LightPosition, -250.0f);
		DirectX::XMMATRIX V = DirectX::XMMatrixLookAtLH(LightPosition,
			//DirectX::XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f),
			DirectX::XMLoadFloat4(&shadowData->shadowCameraPos),
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
		Shader* shader = graphics.GetShader(SHADER_ID::Shadow);
		shader->Begin(dc, graphics.shaderParameter3D_);

		for (std::weak_ptr<RendererCom>& renderObj : renderSortObject_)
		{
			if (!renderObj.lock()->GetGameObject()->GetEnabled())continue;
			if (!renderObj.lock()->GetEnabled())continue;

			//影を落とすか
			if (!renderObj.lock()->GetIsShadowFall())continue;

			Model* model = renderObj.lock()->GetModel();
			if (model != nullptr)
			{
				shader->Draw(dc, model);
			}
		}
		shader->End(dc);
	}
}

//3D描画
void GameObjectManager::Render3D()
{
	if (renderSortObject_.size() <= 0)return;

	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	//ポストエフェクト用切り替え
	PostRenderTarget* ps = Graphics::Instance().GetPostEffectModelRenderTarget().get();
	PostDepthStencil* ds = Graphics::Instance().GetPostEffectModelDepthStencilView().get();

	ID3D11RenderTargetView* rtv = ps->renderTargetView.Get();
	ID3D11DepthStencilView* dsv = ds->depthStencilView.Get();

	// 画面クリア＆レンダーターゲット設定
	FLOAT color[] = { 0.0f, 0.0f, 0.5f, 1.0f };	// RGBA(0.0～1.0)
	dc->OMSetRenderTargets(1, &rtv, dsv);

	// ビューポートの設定
	D3D11_VIEWPORT	vp = {};
	vp.Width = static_cast<float>(ps->width);
	vp.Height = static_cast<float>(ps->height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	dc->RSSetViewports(1, &vp);


	//シェーダーIDが変更された時に呼ぶ
	if (isChangeShaderID_)
	{
		SortRenderObject();
		isChangeShaderID_ = false;
	}

	//マスクオブジェ初期化
	renderMaskObject_.clear();

	// 描画
	int oldShaderID = renderSortObject_[0].lock()->GetShaderID();	//違うシェーダーを使用するため、古いIDを保存
	Shader* shader = graphics.GetShader(static_cast<SHADER_ID>(oldShaderID));
	shader->Begin(dc, graphics.shaderParameter3D_);

	//シルエット描画用
	std::vector<std::weak_ptr<RendererCom>> silhouetteRender;

	for (std::weak_ptr<RendererCom>& renderObj : renderSortObject_)
	{
		if (!renderObj.lock()->GetGameObject()->GetEnabled())continue;
		if (!renderObj.lock()->GetEnabled())continue;

		int newShaderID = renderObj.lock()->GetShaderID();

		//マスクオブジェ
		if (newShaderID == static_cast<int>(SHADER_ID::MaskUnityChan))
		{
			renderMaskObject_.emplace_back(renderObj.lock());
			continue;
		}

		//シルエット描画は最後に行う
		if (renderObj.lock()->GetSilhouetteFlag())
		{
			silhouetteRender.emplace_back(renderObj.lock());
			continue;
		}

		//シェーダーIDが変化したら、シェーダーを変更
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

	//シルエット描画
	for (std::weak_ptr<RendererCom>& silhouette : silhouetteRender)
	{
		//シルエット描画
		shader->End(dc);

		shader = graphics.GetShader(SHADER_ID::Silhoutte);
		oldShaderID = static_cast<int>(SHADER_ID::Silhoutte);

		shader->Begin(dc, graphics.shaderParameter3D_);

		Model* model = silhouette.lock()->GetModel();
		if (model != nullptr)
		{
			shader->Draw(dc, model);
		}

		//モデル描画
		shader->End(dc);

		shader = graphics.GetShader(static_cast<SHADER_ID>(silhouette.lock()->GetShaderID()));

		shader->Begin(dc, graphics.shaderParameter3D_);

		if (model != nullptr)
		{
			shader->Draw(dc, model);
		}

	}

	shader->End(dc);
}

void GameObjectManager::RenderMask()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	Shader* shader = graphics.GetShader(SHADER_ID::UnityChanToon);

	shader->Begin(dc, graphics.shaderParameter3D_);

	for (std::weak_ptr<RendererCom>& renderMaskObj : renderMaskObject_)
	{
		Model* model = renderMaskObj.lock()->GetModel();
		if (model != nullptr)
		{
			shader->Draw(dc, model);
		}
	}

	shader->End(dc);
}

//トレイル描画
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

//パーティクル描画
void GameObjectManager::ParticleRender()
{
	if (particleObject_.size() <= 0)return;

	for (std::weak_ptr<ParticleSystemCom>& particleObj : particleObject_)
	{
		if (!particleObj.lock()->GetGameObject()->GetEnabled())continue;
		if (!particleObj.lock()->GetEnabled())continue;

		particleObj.lock()->Render();

	}
}

//オブジェクト解放
void GameObjectManager::EraseObject(std::vector<std::shared_ptr<GameObject>>& objs, std::shared_ptr<GameObject> removeObj)
{
	//子から解放
	for (auto& childObj : removeObj->GetChildren())
	{
		EraseObject(objs, childObj.lock());
	}

	std::vector<std::shared_ptr<GameObject>>::iterator it = std::find(objs.begin(), objs.end(), removeObj);
	if (it != objs.end())
	{
		removeObj->AudioRelease();
		objs.erase(it);
	}
}

#pragma endregion endGameObjectManager