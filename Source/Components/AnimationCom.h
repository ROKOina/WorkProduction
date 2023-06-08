#pragma once

#include <DirectXMath.h>

#include "System\Component.h"

#include "Graphics\Model\ModelResource.h"

//�A�j���[�V����
class AnimationCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    AnimationCom() {}
    ~AnimationCom() {}

    // ���O�擾
    const char* GetName() const override { return "Animation"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //Animation�N���X
public:
    //�A�j���[�V�����Đ�
    void PlayAnimation(int index, bool loop, float blendSeconds = 0.2f);

    //�A�j���[�V�����Đ�����
    bool IsPlayAnimation();

    //���݂̃A�j���[�V�����Đ����Ԏ擾
    float GetCurrentAnimationSecoonds()const { return currentAnimationSeconds; }

    //�A�j���[�V�����ǉ�
    void ImportFbxAnimation(const char* filename);

private:
    int currentAnimationIndex = -1;
    float currentAnimationSeconds = 0.0f;
    bool animationLoopFlag = false;
    bool animationEndFlag = false;
    float animationBlendTime = 0.0f;
    float animationBlendSeconds = 0.0f;
};