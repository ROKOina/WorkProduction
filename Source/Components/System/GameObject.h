#pragma once

#include <memory>
#include <vector>
#include <set>
#include <string>
#include <DirectXMath.h>
#include <future>

// �O���錾
class Component;
class TransformCom;
class RendererCom;
class SwordTrailCom;
class Collider;

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
	void SetName(const char* name) { this->name_ = name; }

	// ���O�̎擾
	const char* GetName() const { return name_.c_str(); }

	// �R���|�[�l���g�ǉ�
	template<class T, class... Args>
	std::shared_ptr<T> AddComponent(Args... args)
	{
		std::shared_ptr<T> component = std::make_shared<T>(args...);
		component->SetGameObject(shared_from_this());

		//transform�̏ꍇ�͕ێ�����
		if (std::strcmp(component->GetName(), "Transform") == 0)
			transform_ = std::dynamic_pointer_cast<TransformCom>(component);

		components_.emplace_back(component);
		return component;
	}

	// �R���|�[�l���g�擾
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


	//�e�q
	//�ǉ�
	std::shared_ptr<GameObject> AddChildObject();

	//�e�擾
	std::shared_ptr<GameObject> GetParent() { return parentObject_.lock(); }


	//�q���B�擾 (weak_ptr�Ȃ̂Œ���)
	std::vector<std::weak_ptr<GameObject>> GetChildren() { return childrenObject_; }
	std::shared_ptr<GameObject> GetChildFind(const char* name) {
		for (auto& child : childrenObject_)
		{
			if (std::strcmp(name, child.lock()->GetName()) == 0)
				return child.lock();
		}
		return nullptr;
	}

	const bool GetEnabled() const { return isEnabled_; }
	void SetEnabled(bool enabled) { isEnabled_ = enabled; }

	const float GetObjSpeed() const { return objSpeed_; }
	void SetObjSpeed(float speed) { objSpeed_ = speed; }

public:
	std::shared_ptr<TransformCom> transform_;

private:
	std::string	name_;

	std::vector<std::shared_ptr<Component>>	components_;

	//�e�q
	std::weak_ptr<GameObject> parentObject_;
	std::vector<std::weak_ptr<GameObject>> childrenObject_;

	//�L����
	bool isEnabled_ = true;

	//�I�u�W�F�N�g�X�s�[�h�i�X���[�ɂł���j
	float objSpeed_ = 1;
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

	//�V�F�[�_�[ID��ς�����A�\�[�g���邽�߂ɌĂяo��
	void ChangeShaderID() { isChangeShaderID_ = true; }

private:
	void DrawLister();
	void DrawDetail();

	//�����_�[�I�u�W�F�N�g���V�F�[�_�[ID���Ƀ\�[�g����
	void SortRenderObject();

	//�e�`��
	void RenderShadowmap();
	//3D�`��
	void Render3D();

	//�g���C���`��
	void SwordTrailRender();

	//�I�u�W�F�N�g���
	void EraseObject(std::vector<std::shared_ptr<GameObject>>& objs, std::shared_ptr<GameObject> removeObj);

	//�X���b�h�œG�X�V
	void ThreadEnemyUpdate(int id, float elapsedTime);

private:
	std::vector<std::shared_ptr<GameObject>>		startGameObject_;
	std::vector<std::shared_ptr<GameObject>>		updateGameObject_;
	std::set<std::shared_ptr<GameObject>>		selectionGameObject_;
	std::set<std::shared_ptr<GameObject>>		removeGameObject_;

	std::vector<std::weak_ptr<GameObject>>		enemyGameObject_;
	bool EnemyObjFind(std::shared_ptr<GameObject> obj);



	std::vector<std::weak_ptr<Collider>>	colliderObject_;

	//�`�揇�Ɋi�[����
	std::vector<std::weak_ptr<RendererCom>>   renderSortObject_;
	bool isChangeShaderID_ = false;

	//�g���C���`��p
	std::vector<std::weak_ptr<SwordTrailCom>>   swordTrailObject_;

	bool					isHiddenLister_ = false;
	bool					isHiddenDetail_ = false;

	//�X���b�h�p
	std::vector<std::future<void>> future;
};
