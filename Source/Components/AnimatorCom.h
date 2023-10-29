#pragma once

#include <DirectXMath.h>
#include <map>

#include "System\Component.h"

enum PARAMETER_JUDGE
{
    GREATER,    //大きい
    LESS,       //小さい
};

//アニメーションの遷移をする
class AnimatorCom : public Component
{
    // コンポーネントオーバーライド
public:
    AnimatorCom() {}
    ~AnimatorCom() {}

    // 名前取得
    const char* GetName() const override { return "Animator"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

    //Animatorクラス
private:
    //遷移のパラメーター
    struct AnimTransitionParameterTrigger
    {
        std::string name;
        bool trigger;
    };
    struct AnimTransitionParameterFloat
    {
        std::string name;
        float value;
    };
    struct ParameterFloatJudge  //大きいか小さいか判定する
    {
        std::shared_ptr<AnimTransitionParameterFloat> floatParameter;
        float setFloat;
        PARAMETER_JUDGE judgeValue = PARAMETER_JUDGE::GREATER;
    };

    //遷移用構造体
    struct AnimTransition
    {
        int transitionIndex;    //遷移先のインデックス
        bool hasExit = false;   //アニメーションが終わったら勝手に遷移するか
        float blendTime = 0.2f;

        //パラメーター
        std::vector<std::shared_ptr<AnimTransitionParameterTrigger>> triggerParameters;
        std::vector<ParameterFloatJudge> floatParameters;
    };
    //遷移を管理する構造体
    struct AnimatorData
    {
        bool isLoop = false;    //現在のインデックスのアニメーションをループするか
        std::vector<AnimTransition> transitions;
    };

public:
    //遷移を追加
    void AddAnimatorTransition(int sorce, int destination, bool hasExit = false, float blendTime = 0.2f);
    //どこからでも遷移するTransitionを追加
    void AddAnimatorTransition(int destination, bool hasExit = false, float blendTime = 0.2f);

    //triggerパラメーターを追加
    void AddTriggerParameter(std::string name);
    //遷移パラメーターに設定
    void SetTriggerTransition(int sorce, int destination, std::string name);
    //どこからでも遷移パラメーター設定
    void SetTriggerTransition(int destination, std::string name);
    //triggerオンにする
    void SetTriggerOn(std::string name);

    //floatパラメーターを追加
    void AddFloatParameter(std::string name);
    //遷移パラメーターに設定
    void SetFloatTransition(int sorce, int destination, std::string name, float judgeValue, PARAMETER_JUDGE paramJudge);
    //どこからでも遷移パラメーター設定
    void SetFloatTransition(int destination, std::string name, float judgeValue, PARAMETER_JUDGE paramJudge);
    //floatをセットにする
    void SetFloatValue(std::string name, float value);


    void SetLoopAnimation(int index, bool loop) { animatorData_[index].isLoop = loop; }

    void SetFirstTransition(int index) { firstTransition_ = index; }
    
    //アニメーション停止
    void SetIsStop(bool stop);
    bool GetIsStop();

    //アニメーションスピード（オフセット）
    void SetAnimationSpeedOffset(float speed) { animSpeedOffset_ = speed; }
    float GetAnimationSpeedOffset() { return  animSpeedOffset_; }

    //パラメーターをリセットする
    void ResetParameterList();

private:
    //アニメーション再生速度
    void SetAnimationSpeed(float speed);
    float GetAnimationSpeed();

    //hasExitをクリア
    void ResetHasExit();

private:
    //キーをそのままアニメーションのindexに使う
    std::map<int, AnimatorData> animatorData_;
    //どこからでも遷移するリスト
    std::vector<AnimTransition> animatorAnyTransition_;

    //パラメーターリスト
    std::vector<std::shared_ptr<AnimTransitionParameterTrigger>> triggerParameterList_;
    std::vector<std::shared_ptr<AnimTransitionParameterFloat>> floatParameterList_;

    //最初のアニメーションを絶対に指定する
    int firstTransition_ = -1;
    bool oneFlag_ = false;

    //アニメーションスピードオフセット
    float animSpeedOffset_ = 1;

    //次のアニメーションに行くか
    bool hasExit_ = false; 
    int saveIndex_ = -1;
    float saveBlendTime_ = -1;

};