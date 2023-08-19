#pragma once

#include "Components\System\Component.h"
#include "AttackPlayer.h"

class PlayerCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    PlayerCom() {}
    ~PlayerCom() {}

    // ���O�擾
    const char* GetName() const override { return "Player"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //PlayerCom�N���X

#pragma region �ړ�
private:
    //�X�e�B�b�N���͒l����ړ��x�N�g�����擾
    DirectX::XMFLOAT3 GetMoveVec();

    //�ړ����͏���
    bool IsMove(float elapsedTime);
    //��]
    void Trun(float elapsedTime);
    //�c�����ړ�
    void VerticalMove();
    //�������ړ�
    void HorizonMove();

    //�_�b�V��
    void DashMove(float elapsedTime);
    //�_�b�V�����̍X�V
    void DashStateUpdate(float elapsedTime, std::shared_ptr<GameObject> enemy);
    //�����I�Ƀ_�b�V�����I��点��i�U�������j
    void DashEndFlag();

private:
    //�ړ��n
    //���͒l�ۑ�
    DirectX::XMFLOAT3 inputMoveVec_;

    //����ƕ�����؂�ւ���
    enum MOVE_PARAM
    {
        WALK,
        RUN,
        JUSTDASH,
        DASH,
        MAX,
    };
    struct
    {
        float moveMaxSpeed = 10.0f;
        float moveSpeed = 1.0f;
        float moveAcceleration = 0.2f;
        float turnSpeed = 8.0f;
    }moveParam_[MOVE_PARAM::MAX];
    int moveParamType_ = MOVE_PARAM::WALK;

    //���͂ňړ��ł��邩
    bool isInputMove_ = true;
    //��]�ł��邩
    bool isInputTrun_ = true;

    float jumpSpeed_ = 20.0f;
    int jumpCount_ = 2;

    bool isDash_ = true;        //�_�b�V���ł��邩
    bool isDashJudge_ = false;  //�_�b�V������
    int dashState_ = -1;        //�_�b�V���̑J��
    float dashMaxSpeed_ = 20;   //�_�b�V���̍ő�X�s�[�h
    float dashStopTime_ = 1;    //�_�b�V���������~�߂邽��
    float dashStopTimer_;

#pragma endregion

#pragma region �W���X�g���
private:
    //�W���X�g���������
    void JustInisialize();
    //�W���X�g��𔽌��X�V����
    void JustAvoidanceAttackUpdate(float elapsedTime);
    //�W���X�g��𒆈ړ�
    void JustAvoidanceMove(float elapsedTime);
    //�W���X�g��𔽌��̓��͂�����
    void JustAvoidanceAttackInput();

    //������
    void JustAvoidanceSquare(float elapsedTime);

private:
    //�W���X�g��𔻒�
    bool isJustJudge_ = false;  //�W���X�g��𔻒�
    int justAvoidState_ = -1;   //�W���X�g����̑J��
    float justAvoidTime_ = 1;   //�W���X�g�������
    float justAvoidTimer_ = 0;
    std::shared_ptr<GameObject> justHitEnemy_;   //�W���X�g������̓G�ۑ�

    //�W���X�g��𔽌�
    enum class JUST_AVOID_KEY   //���͂𔻒�
    {
        SQUARE,     //��
        TRIANGLE,   //��

        NULL_KEY,
    };
    JUST_AVOID_KEY justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;

    //�W���X�g�����̍U�����U���Ɉ����p���p
    std::string justAnimFlagName_ = "";

#pragma endregion

#pragma region �U��
private:
    //�U���X�V
    void AttackUpdate();

    //�U�������蔻��
    void AttackJudgeCollision();

    //�����I�ɍU�����I��点��i�W�����v�����j
    void AttackFlagEnd();

private:
    bool isNormalAttack_ = true;     //�U���ł��邩
    int comboAttackCount_ = 0;

#pragma endregion

private:
    //�A�j���[�V�����������ݒ�
    void AnimationInitialize();

private:

    DirectX::XMFLOAT3 up_ = {0,1,0};

    //�v���C���[�̍U���̓����̂݊Ǘ�����
    std::shared_ptr<AttackPlayer> attackPlayer_;
};