#pragma once

#include "System\Component.h"
#include "Graphics\Model\Model.h"
#include "Graphics\Graphics.h"

class RendererCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public: 
    RendererCom() {}
    ~RendererCom() {}

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
    Model* GetModel() const { return model_.get(); }

    //�V�F�[�_�[ID�Q�b�g
    const int GetShaderID() const { return shaderID_; }
    void SetShaderID(SHADER_ID shaderID) { shaderID_ = shaderID; }

private:
    std::unique_ptr<Model>	model_;

    int shaderID_ = SHADER_ID::Phong;  //�V�F�[�_�[�����߂�
};