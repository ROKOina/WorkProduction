#pragma once

#include "Graphics/Sprite/Sprite.h"
#include "Scene.h"

//�^�C�g���V�[��
class ScneTitle :public Scene
{
public:
    ScneTitle(){}
    ~ScneTitle()override{}

    //������
    void Initialize()override;

    //�I����
    void Finalize()override;

    //�X�V����
    void Update(float elapsedTime)override;

    //�`�揈��
    void Render()override;

private:
    Sprite* sprite = nullptr;
    std::unique_ptr<Sprite> alphaS = std::make_unique<Sprite>("Data/Sprite/LoadingIcon.png");
};