#pragma once

#include <DirectXMath.h>

//�L�����N�^�[
class Character
{
public:
    Character(){}
    virtual ~Character(){}  //�p���ړI�̃N���X�̓f�X�g���N�^��virtual�����遦�p����̃f�X�g���N�^���Ă΂�Ȃ��Ȃ��Ă��܂�����
private:
    //�������͍X�V����
    void UpdateVertialVelocity(float elapsedFrame);

    //�����ړ��X�V����
    void UpdateVertialMove(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3& up);

    //�������͍X�V����
    void UpdateHorizontalVelocity(float elapsedFrame);

    //�����ړ��X�V����
    void UpdateHorizontalMove(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3& up);

protected:
    //�ړ�����
    void IsMove(float vx, float vz, float speed);

    //���񏈗�
    void Turn(float elapsedTime, float vx, float vz, float speed, DirectX::XMFLOAT4& angle,DirectX::XMFLOAT3 up);

    //�W�����v����
    void Jump(float speed);

    //���͏����X�V
    void UpdateVelocity(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3& up);


protected:  //�p����ɃA�N�Z�X�\�ɂ���

    //�W�����v
    float gravity_ = -1.0f;
    DirectX::XMFLOAT3 velocity_ = { 0,0,0 }; //����

    float friction_ = 0.5f;  //���C

    //����
    float acceleration_ = 1.0f;
    float maxMoveSpeed_ = 5.0f;
    float moveVecX_ = 0.0f;
    float moveVecZ_ = 0.0f;
    float airControl_ = 0.3f;

    float stepOffset_ = 1.0f;
    float slopeRate_ = 1.0f;

};