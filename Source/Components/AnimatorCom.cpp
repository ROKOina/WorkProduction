#include "AnimatorCom.h"

#include <imgui.h>

#include "AnimationCom.h"
#include "Graphics/Graphics.h"

// �J�n����
void AnimatorCom::Start()
{
}

// �X�V����
void AnimatorCom::Update(float elapsedTime)
{
    //���E�̑��x�ɍ��킹��
    SetAnimationSpeed(Graphics::Instance().GetWorldSpeed() * animSpeedOffset_ * GetGameObject()->GetObjSpeed());

    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
    if (!animation)return;

    //�ŏ���������
    {
        if (firstTransition_ < 0)return;
        if (!oneFlag_)
        {
            oneFlag_ = true;
            animation->PlayAnimation(firstTransition_, animatorData_[firstTransition_].isLoop);
            return;
        }
    }

    //�ǂ�����ł��J�ڂ̃t���O���m�F
    for (AnimTransition& anyTransition : animatorAnyTransition_)
    {

#pragma region �����i�p�����[�^�[�j�őJ�ڔ���
        //�g���K�[
        for (auto& triggerParam : anyTransition.triggerParameters)
        {
            if (triggerParam->trigger)
            {
                triggerParam->trigger = false;
                //�A�j���[�V�����J��
                animation->PlayAnimation(
                    anyTransition.transitionIndex,
                    animatorData_[anyTransition.transitionIndex].isLoop,
                    anyTransition.blendTime);
                return;
            }
        }
        //�t���[�g
        for (auto& floatParam : anyTransition.floatParameters)
        {
            if (floatParam.judgeValue == PATAMETER_JUDGE::GREATER)
            {
                if (floatParam.floatParameter.get()->value < floatParam.setFloat)continue;
                //�A�j���[�V�����J��
                animation->PlayAnimation(
                    anyTransition.transitionIndex,
                    animatorData_[anyTransition.transitionIndex].isLoop,
                    anyTransition.blendTime);
                return;
            }
            if (floatParam.judgeValue == PATAMETER_JUDGE::LESS)
            {
                if (floatParam.floatParameter.get()->value >= floatParam.setFloat)continue;
                //�A�j���[�V�����J��
                animation->PlayAnimation(
                    anyTransition.transitionIndex,
                    animatorData_[anyTransition.transitionIndex].isLoop,
                    anyTransition.blendTime);
                return;
            }
        }
#pragma endregion

    }

    //���ɑJ��
    if (hasExit_)
    {
        //�A�j���[�V�����J��
        animation->PlayAnimation(
            saveIndex_,
            animatorData_[saveIndex_].isLoop,
            saveBlendTime_);
        hasExit_ = false;
        saveIndex_ = -1;
        saveBlendTime_ = -1;
    }

    //���݂̃A�j���[�V����
    int currentIndex = animation->GetCurrentAnimationIndex();
    //�J�ڐ悪���邩
    std::vector<AnimTransition>& currentTransitions = animatorData_[currentIndex].transitions;
    if (currentTransitions.size() > 0)
    {
        for (AnimTransition& transition : currentTransitions)
        {

#pragma region �����i�p�����[�^�[�j�őJ�ڔ���
            //�g���K�[
            for (auto& triggerParam : transition.triggerParameters)
            {
                if (!triggerParam->trigger)continue;
                triggerParam->trigger = false;
                //�A�j���[�V�����J��
                animation->PlayAnimation(
                    transition.transitionIndex,
                    animatorData_[transition.transitionIndex].isLoop,
                    transition.blendTime);
                return;
            }
            //�t���[�g
            for (auto& floatParam : transition.floatParameters)
            {
                if (floatParam.judgeValue == PATAMETER_JUDGE::GREATER)
                {
                    if (floatParam.floatParameter.get()->value < floatParam.setFloat)continue;
                    //�A�j���[�V�����J��
                    animation->PlayAnimation(
                        transition.transitionIndex,
                        animatorData_[transition.transitionIndex].isLoop,
                        transition.blendTime);
                    return;
                }
                if (floatParam.judgeValue == PATAMETER_JUDGE::LESS)
                    if (floatParam.floatParameter.get()->value >= floatParam.setFloat)continue;
                {
                    //�A�j���[�V�����J��
                    animation->PlayAnimation(
                        transition.transitionIndex,
                        animatorData_[transition.transitionIndex].isLoop,
                        transition.blendTime);
                    return;
                }
            }
#pragma endregion

#pragma region hasExit�őJ�ڔ���
            if (transition.hasExit)
            {
                //���[�v�L��ꍇ
                if (animation->IsPlayAnimationLooped())
                {
                    hasExit_ = true;
                    saveIndex_ = transition.transitionIndex;
                    saveBlendTime_ = transition.blendTime;
                    ////�A�j���[�V�����J��
                    //animation->PlayAnimation(
                    //    transition.transitionIndex,
                    //    animatorData_[transition.transitionIndex].isLoop,
                    //    transition.blendTime);
                    return;
                }
                //���[�v�����ꍇ
                if (!animation->IsPlayAnimation())
                {
                    hasExit_ = true;
                    saveIndex_ = transition.transitionIndex;
                    saveBlendTime_ = transition.blendTime;
                    ////�A�j���[�V�����J��
                    //animation->PlayAnimation(
                    //    transition.transitionIndex,
                    //    animatorData_[transition.transitionIndex].isLoop,
                    //    transition.blendTime);
                    return;
                }
            }
#pragma endregion

        }
    }
}

// GUI�`��
void AnimatorCom::OnGUI()
{
    //�A�j���[�V�������x
    float animSpeed = GetAnimationSpeed();
    if (ImGui::DragFloat("animSpeed", &animSpeed, 0.01f))
        SetAnimationSpeed(animSpeed);

    //�A�j���[�V�������x�I�t�Z�b�g
    float animSpeedOffset = GetAnimationSpeedOffset();
    if (ImGui::DragFloat("animSpeedOffset", &animSpeedOffset, 0.01f))
        SetAnimationSpeedOffset(animSpeedOffset);

    //�A�j���[�V������~
    bool snimStop = GetIsStop();
    if (ImGui::Checkbox("animStop", &snimStop))
        SetIsStop(snimStop);
}


//�J�ڂ�ݒ�
void AnimatorCom::AddAnimatorTransition(int sorce, int destination, bool hasExit, float blendTime)
{
    //�������Ɛ�͈�����ݒ�͂ł��Ȃ�
    AnimTransition transition;
    transition.transitionIndex = destination;
    transition.hasExit = hasExit;
    transition.blendTime = blendTime;
    animatorData_[sorce].transitions.emplace_back(transition);
    animatorData_[destination]; //�ꉞ�J�ڐ悪�Ȃ��ꍇ�J�ڂ��Ȃ��̂Ő錾���Ă���
}
//�ǂ�����ł��J�ڂ���Transition��ǉ�
void AnimatorCom::AddAnimatorTransition(int destination, bool hasExit, float blendTime)
{
    //�������Ɛ�͈�����ݒ�͂ł��Ȃ�
    AnimTransition transition;
    transition.transitionIndex = destination;
    transition.hasExit = hasExit;
    transition.blendTime = blendTime;
    animatorAnyTransition_.emplace_back(transition);
}

//trigger�p�����[�^�[��ǉ�
void AnimatorCom::AddTriggerParameter(std::string name)
{
    std::shared_ptr<AnimTransitionParameterTrigger> triggerPram = std::make_shared<AnimTransitionParameterTrigger>();
    triggerPram->name = name;
    triggerParameterList_.emplace_back(triggerPram);
}

//�J�ڃp�����[�^�[�ɐݒ�
void AnimatorCom::SetTriggerTransition(int sorce, int destination, std::string name)
{
    for (AnimTransition& transition : animatorData_[sorce].transitions)
    {
        //�J�ڐ�Ɠ����Ȃ�
        if (transition.transitionIndex != destination)continue;
        for (auto& triggerList : triggerParameterList_)
        {
            if (triggerList->name != name)continue;
            transition.triggerParameters.emplace_back(triggerList);
            return;
        }
    }
}

//�ǂ�����ł��J�ڃp�����[�^�[�ݒ�
void AnimatorCom::SetTriggerTransition(int destination, std::string name)
{
    for (AnimTransition& anyTransition : animatorAnyTransition_)
    {
        if (anyTransition.transitionIndex != destination)continue;
        for (auto& triggerList : triggerParameterList_)
        {
            if (triggerList->name != name)continue;
            anyTransition.triggerParameters.emplace_back(triggerList);
            return;
        }
    }
}

//trigger�I���ɂ���
void AnimatorCom::SetTriggerOn(std::string name)
{
    for (auto& triggerList : triggerParameterList_)
    {
        if (triggerList->name == name)
        {
            triggerList->trigger = true;
            return;
        }
    }
}


//float�p�����[�^�[��ǉ�
void AnimatorCom::AddFloatParameter(std::string name)
{
    std::shared_ptr<AnimTransitionParameterFloat> floatPram = std::make_shared<AnimTransitionParameterFloat>();
    floatPram->name = name;
    floatParameterList_.emplace_back(floatPram);
}

//�J�ڃp�����[�^�[�ɐݒ�
void AnimatorCom::SetFloatTransition(int sorce, int destination, std::string name, float judgeValue, PATAMETER_JUDGE paramJudge)
{
    for (AnimTransition& transition : animatorData_[sorce].transitions)
    {
        //�J�ڐ�Ɠ����Ȃ�
        if (transition.transitionIndex != destination)continue;
        for (auto& floatList : floatParameterList_)
        {
            if (floatList->name != name)continue;
            ParameterFloatJudge pramFloat;
            pramFloat.floatParameter = floatList;
            pramFloat.setFloat = judgeValue;
            pramFloat.judgeValue = paramJudge;
            transition.floatParameters.emplace_back(pramFloat);
            return;
        }
    }
}

//�ǂ�����ł��J�ڃp�����[�^�[�ݒ�
void AnimatorCom::SetFloatTransition(int destination, std::string name, float judgeValue, PATAMETER_JUDGE paramJudge)
{
    for (AnimTransition& anyTransition : animatorAnyTransition_)
    {
        if (anyTransition.transitionIndex != destination)continue;
        for (auto& floatList : floatParameterList_)
        {
            if (floatList->name != name)continue;
            ParameterFloatJudge pramFloat;
            pramFloat.floatParameter = floatList;
            pramFloat.setFloat = judgeValue;
            pramFloat.judgeValue = paramJudge;
            anyTransition.floatParameters.emplace_back(pramFloat);
            return;
        }
    }
}

//float���Z�b�g�ɂ���
void AnimatorCom::SetFloatValue(std::string name, float value)
{
    for (auto& floatList : floatParameterList_)
    {
        if (floatList->name != name)continue;
        floatList->value = value;
        return;
    }
}

//�A�j���[�V�����Đ����x
void AnimatorCom::SetAnimationSpeed(float speed)
{
    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
    if (!animation)return;

    animation->SetAnimationSpeed(speed);
}
float AnimatorCom::GetAnimationSpeed()
{
    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
    if (!animation)return 0;

    return animation->GetAnimationSpeed();
}

//�A�j���[�V������~
void AnimatorCom::SetIsStop(bool stop)
{
    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
    if (!animation)return;

    animation->SetIsStop(stop);
}
bool AnimatorCom::GetIsStop()
{
    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
    if (!animation)return false;

    return animation->GetIsStop();
}

//�p�����[�^�[�����Z�b�g����
void AnimatorCom::ResetParameterList()
{
    for (auto& triggerList : triggerParameterList_)
    {
        triggerList->trigger = false;
    }
}