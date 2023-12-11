#pragma once

#include <memory>
#include <vector>
#include <set>
#include <string>
#include <DirectXMath.h>
#include <future>

// 前方宣言
class Component;
class TransformCom;
class RendererCom;
class ParticleSystemCom;
class SwordTrailCom;
class Collider;

// ゲームオブジェクト
class GameObject : public std::enable_shared_from_this<GameObject>
{
public:
	GameObject() {  }
	virtual ~GameObject() {};

	// 開始処理
	virtual void Start();

	// 更新処理
	virtual void Update(float elapsedTime);

	// 行列の更新
	virtual void UpdateTransform();

	// GUI表示
	virtual void OnGUI();

	//sprite描画用
	virtual void Render2D(float elapsedTime);

	// 名前の設定
	void SetName(const char* name) { this->name_ = name; }

	// 名前の取得
	const char* GetName() const { return name_.c_str(); }

	// コンポーネント追加
	template<class T, class... Args>
	std::shared_ptr<T> AddComponent(Args... args)
	{
		std::shared_ptr<T> component = std::make_shared<T>(args...);
		component->SetGameObject(shared_from_this());

		//transformの場合は保持する
		if (std::strcmp(component->GetName(), "Transform") == 0)
			transform_ = std::dynamic_pointer_cast<TransformCom>(component);

		components_.emplace_back(component);
		return component;
	}

	// コンポーネント取得
	template<class T>
	std::shared_ptr<T> GetComponent()
	{
		for (std::shared_ptr<Component>& component : components_)
		{
			std::shared_ptr<T> p = std::dynamic_pointer_cast<T>(component);
			if (p == nullptr) continue;
			return p;
		}
		return nullptr;
	}


	//親子
	//追加
	std::shared_ptr<GameObject> AddChildObject();

	//親取得
	std::shared_ptr<GameObject> GetParent() { return parentObject_.lock(); }


	//子供達取得 (weak_ptrなので注意)
	std::vector<std::weak_ptr<GameObject>> GetChildren() { return childrenObject_; }
	std::shared_ptr<GameObject> GetChildFind(const char* name) {
		for (auto& child : childrenObject_)
		{
			if (std::strcmp(name, child.lock()->GetName()) == 0)
				return child.lock();
		}
		return nullptr;
	}

	//子から期限切れのポインタ削除
	void EraseExpiredChild();

	const bool GetEnabled() const { return isEnabled_; }
	void SetEnabled(bool enabled) { isEnabled_ = enabled; }

	const float GetObjSpeed() const { return objSpeed_; }
	void SetObjSpeed(float speed) { objSpeed_ = speed; }

	//音解放
	void AudioRelease();

public:
	std::shared_ptr<TransformCom> transform_;

private:
	std::string	name_;

	std::vector<std::shared_ptr<Component>>	components_;

	//親子
	std::weak_ptr<GameObject> parentObject_;
	std::vector<std::weak_ptr<GameObject>> childrenObject_;

	//有効か
	bool isEnabled_ = true;

	//オブジェクトスピード（スローにできる）
	float objSpeed_ = 1;
};

// ゲームオブジェクトマネージャー
class GameObjectManager
{
private:
	GameObjectManager() {}
	~GameObjectManager() {}

public:
	// インスタンス取得
	static GameObjectManager& Instance()
	{
		static GameObjectManager instance;
		return instance;
	}

	// 作成
	std::shared_ptr<GameObject> Create();

	// 削除
	void Remove(std::shared_ptr<GameObject> obj);
	// 全削除
	void AllRemove();

	// 更新
	void Update(float elapsedTime);

	// 行列更新
	void UpdateTransform();

	// 描画
	void Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

	//マスクオブジェクト描画
	void RenderMask();
	//sprite描画
	void Render2D(float elapsedTime);

	//ゲームオブジェクトを探す
	std::shared_ptr<GameObject> Find(const char* name);

	//シェーダーIDを変えたら、ソートするために呼び出す
	void ChangeShaderID() { isChangeShaderID_ = true; }

	//シーンゲーム演出待ちフラグ
	bool GetIsSceneGameStart() { return isSceneGameStart_; }
	void SetIsSceneGameStart(bool flag) { isSceneGameStart_ = flag; }

private:
	void DrawLister();
	void DrawDetail();

	//レンダーオブジェクトをシェーダーID順にソートする
	void SortRenderObject();

	//影描画
	void RenderShadowmap();
	//3D描画
	void Render3D();

	//トレイル描画
	void SwordTrailRender();

	//パーティクル描画
	void ParticleRender();

	//オブジェクト解放
	void EraseObject(std::vector<std::shared_ptr<GameObject>>& objs, std::shared_ptr<GameObject> removeObj);

	//スレッドで敵更新
	void ThreadEnemyUpdate(int id, float elapsedTime);

private:
	std::vector<std::shared_ptr<GameObject>>		startGameObject_;
	std::vector<std::shared_ptr<GameObject>>		updateGameObject_;
	std::set<std::shared_ptr<GameObject>>		selectionGameObject_;
	std::set<std::shared_ptr<GameObject>>		removeGameObject_;

	std::vector<std::weak_ptr<GameObject>>		enemyGameObject_;
	bool EnemyObjFind(std::shared_ptr<GameObject> obj);



	std::vector<std::weak_ptr<Collider>>	colliderObject_;

	//描画順に格納する
	std::vector<std::weak_ptr<RendererCom>>   renderSortObject_;
	bool isChangeShaderID_ = false;
	//マスク用
	std::vector<std::weak_ptr<RendererCom>>   renderMaskObject_;

	//トレイル描画用
	std::vector<std::weak_ptr<SwordTrailCom>>   swordTrailObject_;

	//パーティクル描画用
	std::vector<std::weak_ptr<ParticleSystemCom>>   particleObject_;

	bool					isHiddenLister_ = false;
	bool					isHiddenDetail_ = false;

	//演出待ちフラグ（シーンゲーム）
	bool isSceneGameStart_ = false;

	//スレッド用
	std::vector<std::future<void>> future;

	std::mutex mutex_;
};
