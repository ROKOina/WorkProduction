#pragma once

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
    virtual void Render() = 0;

    //�����������Ă��邩
    bool IsReady()const { return isReady_; }

    //���������ݒ�
    void SetReady() { isReady_ = true; }

private:
    bool isReady_ = false;
};