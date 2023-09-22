#pragma once

#include <memory>
#include <DirectXMath.h>

//�O���錾
class PlayerCom;
class GameObject;

//�v���C���[�̈ړ����Ǘ�
class MovePlayer
{
public:
    MovePlayer(std::shared_ptr<PlayerCom> player);
    ~MovePlayer(){}

    void Update(float elapsedTime);

    void OnGui();

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
    void DashStateUpdate(float elapsedTime);

    //����؂�ւ���
    void RunTurnMove();
    void RunTurnJudge();
    void EndRunTurn();

public:
    //�����I�Ƀ_�b�V�����I��点��i�U�������j
    //�����͕����ɂ��邩
    void DashEndFlag(bool isWalk = true);

public:
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

    //����؂�ւ���
    bool isRunTurn_ = false;
    int runTurnState_ = -1;
    DirectX::XMFLOAT3 saveTurnVec_;

    //���肩��X�g�b�v���[�V�����t���O
    bool isStopRunStop_ = false;

    float jumpSpeed_ = 20.0f;
    int jumpCount_ = 2;

    bool isDash_ = true;        //�_�b�V���ł��邩
    bool isDashJudge_ = false;  //�_�b�V������
    int dashState_ = -1;        //�_�b�V���̑J��
    float dashMaxSpeed_ = 15;   //�_�b�V���̍ő�X�s�[�h
    float dashStopTime_ = 1;    //�_�b�V���������~�߂邽��
    float dashStopTimer_;
    float dashCoolTime_ = 0.6f;       //�_�b�V���̃N�[���^�C��
    float dashCoolTimer_;

private:
    std::weak_ptr<PlayerCom> player_;
};