#pragma once

#include <DirectXMath.h>

#include "System\Component.h"

#include "Graphics\Model\ModelResource.h"

//�A�j���[�V����
class AnimationCom : public Component
{
    // �R���|�[�l���g�I�[�o�[���C�h
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

    // �A�j���[�V�����X�V
    void AnimationUpdata(float elapsedTime);

    //Animation�N���X
public:

    //�A�j���[�V�����Đ�
    void PlayAnimation(int index, bool loop, float blendSeconds = 0.2f);

    //�A�j���[�V�����Đ�����
    bool IsPlayAnimation();

    //���݂̃A�j���[�V�����Đ����Ԏ擾
    float GetCurrentAnimationSecoonds()const { return currentAnimationSeconds_; }

    //�A�j���[�V�����ǉ�
    void ImportFbxAnimation(const char* filename);

private:
    int currentAnimationIndex_ = -1;
    float currentAnimationSeconds_ = 0.0f;
    bool animationLoopFlag_ = false;
    bool animationEndFlag_ = false;
    float animationBlendTime_ = 0.0f;
    float animationBlendSeconds_ = 0.0f;
};