#pragma once

#include <memory>

//�O���錾
class CameraCom;

//�V�[��
class Scene
{
public:
    Scene(){}
    virtual ~Scene(){}

    //������
    virtual void Initialize() = 0;

    //�I����
    virtual void Finalize() = 0;

    //�X�V����
    virtual void Update(float elapsedTime) = 0;

    //�`�揈��
    virtual void Render(float elapsedTime) = 0;

    //�����������Ă��邩
    bool IsReady()const { return isReady_; }

    //���������ݒ�
    void SetReady() { isReady_ = true; }

protected:
    //���C���J����
    std::shared_ptr<CameraCom> mainCamera_;

private:
    bool isReady_ = false;
};