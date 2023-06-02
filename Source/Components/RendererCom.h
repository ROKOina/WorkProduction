#pragma once

#include "System\Component.h"

class RenderderCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public: 
    RenderderCom() {}
    ~RenderderCom() {}

    // ���O�擾
    const char* GetName() const override { return "Renderer"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;


    //Renderer�N���X
public:
    // ���f���̓ǂݍ���
    void LoadModel(const char* filename);

    // ���f���̎擾
    Model* GetModel() const { return model.get(); }

private:
    std::unique_ptr<Model>	model;

};