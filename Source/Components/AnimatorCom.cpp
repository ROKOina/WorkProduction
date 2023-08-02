#include "AnimatorCom.h"

#include <imgui.h>

#include "AnimationCom.h"

// 開始処理
void AnimatorCom::Start()
{
}

// 更新処理
void AnimatorCom::Update(float elapsedTime)
{
    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
    if (!animation)return;

    //最初だけ入る
    {
        if (firstTransition_ < 0)return;
        if (!oneFlag_)
        {
            oneFlag_ = true;
            animation->PlayAnimation(firstTransition_, animatorData_[firstTransition_].isLoop);
            return;
        }
    }

    //どこからでも遷移のフラグを確認
    for (AnimTransition& anyTransition : animatorAnyTransition_)
    {

#pragma region 条件（パラメーター）で遷移判別
        //トリガー
        for (auto& triggerParam : anyTransition.triggerParameters)
        {
            if (triggerParam->trigger)
            {
                triggerParam->trigger = false;
                //アニメーション遷移
                animation->PlayAnimation(
                    anyTransition.transitionIndex,
                    animatorData_[anyTransition.transitionIndex].isLoop,
                    anyTransition.blendTime);
                return;
            }
        }
        //フロート
        for (auto& floatParam : anyTransition.floatParameters)
        {
            if (floatParam.judgeValue == PATAMETER_JUDGE::GREATER)
            {
                if (floatParam.floatParameter.get()->value < floatParam.setFloat)continue;
                //アニメーション遷移
                animation->PlayAnimation(
                    anyTransition.transitionIndex,
                    animatorData_[anyTransition.transitionIndex].isLoop,
                    anyTransition.blendTime);
                return;
            }
            if (floatParam.judgeValue == PATAMETER_JUDGE::LESS)
            {
                if (floatParam.floatParameter.get()->value >= floatParam.setFloat)continue;
                //アニメーション遷移
                animation->PlayAnimation(
                    anyTransition.transitionIndex,
                    animatorData_[anyTransition.transitionIndex].isLoop,
                    anyTransition.blendTime);
                return;
            }
        }
#pragma endregion

    }

    //現在のアニメーション
    int currentIndex = animation->GetCurrentAnimationIndex();
    //遷移先があるか
    std::vector<AnimTransition>& currentTransitions = animatorData_[currentIndex].transitions;
    if (currentTransitions.size() > 0)
    {
        for (AnimTransition& transition : currentTransitions)
        {

#pragma region 条件（パラメーター）で遷移判別
            //トリガー
            for (auto& triggerParam : transition.triggerParameters)
            {
                if (!triggerParam->trigger)continue;
                triggerParam->trigger = false;
                //アニメーション遷移
                animation->PlayAnimation(
                    transition.transitionIndex,
                    animatorData_[transition.transitionIndex].isLoop,
                    transition.blendTime);
                return;
            }
            //フロート
            for (auto& floatParam : transition.floatParameters)
            {
                if (floatParam.judgeValue == PATAMETER_JUDGE::GREATER)
                {
                    if (floatParam.floatParameter.get()->value < floatParam.setFloat)continue;
                    //アニメーション遷移
                    animation->PlayAnimation(
                        transition.transitionIndex,
                        animatorData_[transition.transitionIndex].isLoop,
                        transition.blendTime);
                    return;
                }
                if (floatParam.judgeValue == PATAMETER_JUDGE::LESS)
                    if (floatParam.floatParameter.get()->value >= floatParam.setFloat)continue;
                {
                    //アニメーション遷移
                    animation->PlayAnimation(
                        transition.transitionIndex,
                        animatorData_[transition.transitionIndex].isLoop,
                        transition.blendTime);
                    return;
                }
            }
#pragma endregion

#pragma region hasExitで遷移判別
            if (transition.hasExit)
            {
                //ループ有り場合
                if (animation->IsPlayAnimationLooped())
                {
                    //アニメーション遷移
                    animation->PlayAnimation(
                        transition.transitionIndex,
                        animatorData_[transition.transitionIndex].isLoop,
                        transition.blendTime);
                    return;
                }
                //ループ無し場合
                if (!animation->IsPlayAnimation())
                {
                    //アニメーション遷移
                    animation->PlayAnimation(
                        transition.transitionIndex,
                        animatorData_[transition.transitionIndex].isLoop,
                        transition.blendTime);
                    return;
                }
            }
#pragma endregion

        }
    }
}

// GUI描画
void AnimatorCom::OnGUI()
{
    //アニメーション速度
    float animSpeed = GetAnimationSpeed();
    if (ImGui::DragFloat("animSpeed", &animSpeed, 0.01f))
        SetAnimationSpeed(animSpeed);
}


//遷移を設定
void AnimatorCom::AddAnimatorTransition(int sorce, int destination, bool hasExit, float blendTime)
{
    //同じ元と先は一つしか設定はできない
    AnimTransition transition;
    transition.transitionIndex = destination;
    transition.hasExit = hasExit;
    transition.blendTime = blendTime;
    animatorData_[sorce].transitions.emplace_back(transition);
    animatorData_[destination]; //一応遷移先がない場合遷移しないので宣言しておく
}
//どこからでも遷移するTransitionを追加
void AnimatorCom::AddAnimatorTransition(int destination, bool hasExit, float blendTime)
{
    //同じ元と先は一つしか設定はできない
    AnimTransition transition;
    transition.transitionIndex = destination;
    transition.hasExit = hasExit;
    transition.blendTime = blendTime;
    animatorAnyTransition_.emplace_back(transition);
}

//triggerパラメーターを追加
void AnimatorCom::AddTriggerParameter(std::string name)
{
    std::shared_ptr<AnimTransitionParameterTrigger> triggerPram = std::make_shared<AnimTransitionParameterTrigger>();
    triggerPram->name = name;
    triggerParameterList_.emplace_back(triggerPram);
}

//遷移パラメーターに設定
void AnimatorCom::SetTriggerTransition(int sorce, int destination, std::string name)
{
    for (AnimTransition& transition : animatorData_[sorce].transitions)
    {
        //遷移先と同じなら
        if (transition.transitionIndex != destination)continue;
        for (auto& triggerList : triggerParameterList_)
        {
            if (triggerList->name != name)continue;
            transition.triggerParameters.emplace_back(triggerList);
            return;
        }
    }
}

//どこからでも遷移パラメーター設定
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

//triggerオンにする
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


//floatパラメーターを追加
void AnimatorCom::AddFloatParameter(std::string name)
{
    std::shared_ptr<AnimTransitionParameterFloat> floatPram = std::make_shared<AnimTransitionParameterFloat>();
    floatPram->name = name;
    floatParameterList_.emplace_back(floatPram);
}

//遷移パラメーターに設定
void AnimatorCom::SetFloatTransition(int sorce, int destination, std::string name, float judgeValue, PATAMETER_JUDGE paramJudge)
{
    for (AnimTransition& transition : animatorData_[sorce].transitions)
    {
        //遷移先と同じなら
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

//どこからでも遷移パラメーター設定
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

//floatをセットにする
void AnimatorCom::SetFloatValue(std::string name, float value)
{
    for (auto& floatList : floatParameterList_)
    {
        if (floatList->name != name)continue;
        floatList->value = value;
        return;
    }
}

//アニメーション再生速度
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
