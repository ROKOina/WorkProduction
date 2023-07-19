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
private:
    //�A�j���[�V�����C�x���g�p
    struct AnimEvent
    {
        bool enabled = false;
        std::string name;
        int nodeIndex = -1;
        DirectX::XMFLOAT3 position;
        ModelResource::AnimationEvent resourceEventData;
    };

public:

    //�A�j���[�V�����Đ�
    void PlayAnimation(int index, bool loop, float blendSeconds = 0.2f);

    //�A�j���[�V�����Đ�����
    bool IsPlayAnimation();

    //�A�j���[�V�������[�v������
    bool IsPlayAnimationLooped()const { return isLooped_; }

    //���݂̃A�j���[�V�����Đ����Ԏ擾
    float GetCurrentAnimationSecoonds()const { return currentAnimationSeconds_; }

    //���݂̃A�j���[�V����index�擾
    float GetCurrentAnimationIndex()const { return currentAnimationIndex_; }

    //�A�j���[�V�����ǉ�
    void ImportFbxAnimation(const char* filename);

    //�A�j���[�V�����C�x���g�擾(�|�W�V�����ƍĐ�����)
    bool GetCurrentAnimationEvent(const char* eventName, DirectX::XMFLOAT3& position);

    //�A�j���[�V�����C�x���g�擾
    const AnimEvent GetAnimationEvent(const char* eventName) const{
        for (AnimEvent anim : currentAnimationEvents_)
        {
            if (std::strcmp(eventName, anim.name.c_str()) == 0)
            {
                return anim;
            }
        }
    }

private:
    int currentAnimationIndex_ = -1;
    float currentAnimationSeconds_ = 0.0f;
    bool animationLoopFlag_ = false;
    bool animationEndFlag_ = false;
    float animationBlendTime_ = 0.0f;
    float animationBlendSeconds_ = 0.0f;

    bool isLooped_ = false;  //���[�v������

    std::vector<AnimEvent> currentAnimationEvents_;
};