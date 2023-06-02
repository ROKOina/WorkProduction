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
	void SetGameObject(std::shared_ptr<GameObject> obj) { this->gameObject = obj; }

	// �Q�[���I�u�W�F�N�g�擾
	std::shared_ptr<GameObject> GetGameObject() { return gameObject.lock(); }

private:
	std::weak_ptr<GameObject>	gameObject;

};