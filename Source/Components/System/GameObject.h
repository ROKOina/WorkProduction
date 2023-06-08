#pragma once

#include <memory>
#include <vector>
#include <set>
#include <string>
#include <DirectXMath.h>

// 前方宣言
class Component;
class TransformCom;

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

	// 名前の設定
	void SetName(const char* name) { this->name = name; }

	// 名前の取得
	const char* GetName() const { return name.c_str(); }

	// コンポーネント追加
	template<class T, class... Args>
	std::shared_ptr<T> AddComponent(Args... args)
	{
		std::shared_ptr<T> component = std::make_shared<T>(args...);
		component->SetGameObject(shared_from_this());

		//transformの場合は保持する
		if (std::strcmp(component->GetName(), "Transform") == 0)
			transform = std::dynamic_pointer_cast<TransformCom>(component);

		components.emplace_back(component);
		return component;
	}

	// コンポーネント取得
	template<class T>
	std::shared_ptr<T> GetComponent()
	{
		for (std::shared_ptr<Component>& component : components)
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
	std::shared_ptr<GameObject> GetParent() { return parentObject.lock(); }

	//子供達取得 (weak_ptrなので注意)
	std::vector<std::weak_ptr<GameObject>> GetChildren() { return childrenObject; }

public:
	std::shared_ptr<TransformCom> transform;

private:
	std::string	name;

	std::vector<std::shared_ptr<Component>>	components;

	//親子
	std::weak_ptr<GameObject> parentObject;
	std::vector<std::weak_ptr<GameObject>> childrenObject;
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

	// 更新
	void Update(float elapsedTime);

	// 行列更新
	void UpdateTransform();

	// 描画
	void Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

	//ゲームオブジェクトを探す
	std::shared_ptr<GameObject> Find(const char* name);

private:
	void DrawLister();
	void DrawDetail();

private:
	std::vector<std::shared_ptr<GameObject>>		startGameObject;
	std::vector<std::shared_ptr<GameObject>>		updateGameObject;
	std::set<std::shared_ptr<GameObject>>		selectionGameObject;
	std::set<std::shared_ptr<GameObject>>		removeGameObject;

	bool					hiddenLister = false;
	bool					hiddenDetail = false;
};
