#pragma once

#include <DirectXMath.h>
#include <map>

#include "System\Component.h"

enum PATAMETER_JUDGE
{
    GREATER,    //�傫��
    LESS,       //������
};

//�A�j���[�V�����̑J�ڂ�����
class AnimatorCom : public Component
{
    // �R���|�[�l���g�I�[�o�[���C�h
public:
    AnimatorCom() {}
    ~AnimatorCom() {}

    // ���O�擾
    const char* GetName() const override { return "Animator"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //Animator�N���X
private:
    //�J�ڂ̃p�����[�^�[
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
    struct ParameterFloatJudge  //�傫���������������肷��
    {
        std::shared_ptr<AnimTransitionParameterFloat> floatParameter;
        float setFloat;
        PATAMETER_JUDGE judgeValue = PATAMETER_JUDGE::GREATER;
    };

    //�J�ڗp�\����
    struct AnimTransition
    {
        int transitionIndex;    //�J�ڐ�̃C���f�b�N�X
        bool hasExit = false;   //�A�j���[�V�������I������珟��ɑJ�ڂ��邩
        float blendTime = 0.2f;

        //�p�����[�^�[
        std::vector<std::shared_ptr<AnimTransitionParameterTrigger>> triggerParameters;
        std::vector<ParameterFloatJudge> floatParameters;
    };
    //�J�ڂ��Ǘ�����\����
    struct AnimatorData
    {
        bool isLoop = false;    //���݂̃C���f�b�N�X�̃A�j���[�V���������[�v���邩
        std::vector<AnimTransition> transitions;
    };

public:
    //�J�ڂ�ǉ�
    void AddAnimatorTransition(int sorce, int destination, bool hasExit = false, float blendTime = 0.2f);
    //�ǂ�����ł��J�ڂ���Transition��ǉ�
    void AddAnimatorTransition(int destination, bool hasExit = false, float blendTime = 0.2f);

    //trigger�p�����[�^�[��ǉ�
    void AddTriggerParameter(std::string name);
    //�J�ڃp�����[�^�[�ɐݒ�
    void SetTriggerTransition(int sorce, int destination, std::string name);
    //�ǂ�����ł��J�ڃp�����[�^�[�ݒ�
    void SetTriggerTransition(int destination, std::string name);
    //trigger�I���ɂ���
    void SetTriggerOn(std::string name);

    //float�p�����[�^�[��ǉ�
    void AddFloatParameter(std::string name);
    //�J�ڃp�����[�^�[�ɐݒ�
    void SetFloatTransition(int sorce, int destination, std::string name, float judgeValue, PATAMETER_JUDGE paramJudge);
    //�ǂ�����ł��J�ڃp�����[�^�[�ݒ�
    void SetFloatTransition(int destination, std::string name, float judgeValue, PATAMETER_JUDGE paramJudge);
    //float���Z�b�g�ɂ���
    void SetFloatValue(std::string name, float value);


    //�A�j���[�V�����Đ����x
    void SetAnimationSpeed(float speed);
    float GetAnimationSpeed();

    void SetLoopAnimation(int index, bool loop) { animatorData_[index].isLoop = loop; }

    void SetFirstTransition(int index) { firstTransition_ = index; }
    
private:
    //�L�[�����̂܂܃A�j���[�V������index�Ɏg��
    std::map<int, AnimatorData> animatorData_;
    //�ǂ�����ł��J�ڂ��郊�X�g
    std::vector<AnimTransition> animatorAnyTransition_;

    //�p�����[�^�[���X�g
    std::vector<std::shared_ptr<AnimTransitionParameterTrigger>> triggerParameterList_;
    std::vector<std::shared_ptr<AnimTransitionParameterFloat>> floatParameterList_;

    //�ŏ��̃A�j���[�V�������΂Ɏw�肷��
    int firstTransition_ = -1;
    bool oneFlag_ = false;
};