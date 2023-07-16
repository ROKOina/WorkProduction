#pragma once

#include "GameObject.h"

//�R���|�[�l���g
class Component
{
public:
    Component(){}
    virtual ~Component(){}

	// ���O�擾
	virtual const char* GetName() const = 0;

	// �J�n����
	virtual void Start() {}

	// �X�V����
	virtual void Update(float elapsedTime) {}

	// GUI�`��
	virtual void OnGUI() {}

	// �Q�[���I�u�W�F�N�g�ݒ�
	void SetGameObject(std::shared_ptr<GameObject> obj) { this->gameObject_ = obj; }

	// �Q�[���I�u�W�F�N�g�擾
	std::shared_ptr<GameObject> GetGameObject() { return gameObject_.lock(); }

	const bool GetEnabled() const { return isEnabled_; }
	void SetEnabled(bool enabled) { isEnabled_ = enabled; }

private:
	std::weak_ptr<GameObject>	gameObject_;

protected:
	//�L����
	bool isEnabled_ = true;
};