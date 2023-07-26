#pragma once

#include "Components\System\Component.h"
#include "SystemStruct\QuaternionStruct.h"

class WeaponCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    WeaponCom() {}
    ~WeaponCom() {}

    // ���O�擾
    const char* GetName() const override { return "Weapon"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //WeaponCom�N���X
public:
    //�I�u�W�F�N�g���Z�b�g
    void SetObject(std::shared_ptr<GameObject> obj) { parentObject_ = obj; }
    //���O���Z�b�g
    void SetNodeName(const char* name) { nodeName_ = name; }


    //�A�j���C�x���g�����瓖���蔻���t����
    void SetCollFromEvent(std::string name);

private:
    //����̐e�ɂȂ�I�u�W�F�N�g
    std::shared_ptr<GameObject> parentObject_;
    //����̎q�ɂ��������m�[�h�̖��O
    std::string nodeName_;
};