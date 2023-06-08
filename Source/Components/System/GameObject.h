#pragma once

#include <memory>
#include <vector>
#include <set>
#include <string>
#include <DirectXMath.h>

// �O���錾
class Component;
class TransformCom;

// �Q�[���I�u�W�F�N�g
class GameObject : public std::enable_shared_from_this<GameObject>
{
public:
	GameObject() {  }
	virtual ~GameObject() {};

	// �J�n����
	virtual void Start();

	// �X�V����
	virtual void Update(float elapsedTime);

	// �s��̍X�V
	virtual void UpdateTransform();

	// GUI�\��
	virtual void OnGUI();

	// ���O�̐ݒ�
	void SetName(const char* name) { this->name = name; }

	// ���O�̎擾
	const char* GetName() const { return name.c_str(); }

	// �R���|�[�l���g�ǉ�
	template<class T, class... Args>
	std::shared_ptr<T> AddComponent(Args... args)
	{
		std::shared_ptr<T> component = std::make_shared<T>(args...);
		component->SetGameObject(shared_from_this());

		//transform�̏ꍇ�͕ێ�����
		if (std::strcmp(component->GetName(), "Transform") == 0)
			transform = std::dynamic_pointer_cast<TransformCom>(component);

		components.emplace_back(component);
		return component;
	}

	// �R���|�[�l���g�擾
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


	//�e�q
	//�ǉ�
	std::shared_ptr<GameObject> AddChildObject();

	//�e�擾
	std::shared_ptr<GameObject> GetParent() { return parentObject.lock(); }

	//�q���B�擾 (weak_ptr�Ȃ̂Œ���)
	std::vector<std::weak_ptr<GameObject>> GetChildren() { return childrenObject; }

public:
	std::shared_ptr<TransformCom> transform;

private:
	std::string	name;

	std::vector<std::shared_ptr<Component>>	components;

	//�e�q
	std::weak_ptr<GameObject> parentObject;
	std::vector<std::weak_ptr<GameObject>> childrenObject;
};

// �Q�[���I�u�W�F�N�g�}�l�[�W���[
class GameObjectManager
{
private:
	GameObjectManager() {}
	~GameObjectManager() {}

public:
	// �C���X�^���X�擾
	static GameObjectManager& Instance()
	{
		static GameObjectManager instance;
		return instance;
	}

	// �쐬
	std::shared_ptr<GameObject> Create();

	// �폜
	void Remove(std::shared_ptr<GameObject> obj);

	// �X�V
	void Update(float elapsedTime);

	// �s��X�V
	void UpdateTransform();

	// �`��
	void Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

	//�Q�[���I�u�W�F�N�g��T��
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
