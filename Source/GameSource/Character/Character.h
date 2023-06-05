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
    void UpdateVertialMove(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle);

    //�������͍X�V����
    void UpdateHorizontalVelocity(float elapsedFrame);

    //�����ړ��X�V����
    void UpdateHorizontalMove(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle);

protected:
    //�ړ�����
    void Move(float vx, float vz, float speed);

    //���񏈗�
    void Turn(float elapsedTime, float vx, float vz, float speed, DirectX::XMFLOAT4& angle,DirectX::XMFLOAT3 up);

    //�W�����v����
    void Jump(float speed);

    //���͏����X�V
    void UpdateVelocity(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle);


protected:  //�p����ɃA�N�Z�X�\�ɂ���

    //�W�����v
    float gravity = -1.0f;
    DirectX::XMFLOAT3 velocity = { 0,0,0 }; //����

    float friction = 0.5f;  //���C

    //����
    float acceleration = 1.0f;
    float maxMoveSpeed = 5.0f;
    float moveVecX = 0.0f;
    float moveVecZ = 0.0f;
    float airControl = 0.3f;

    float stepOffset = 1.0f;
    float slopeRate = 1.0f;

};