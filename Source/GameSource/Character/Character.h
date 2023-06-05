#pragma once

#include <DirectXMath.h>

//キャラクター
class Character
{
public:
    Character(){}
    virtual ~Character(){}  //継承目的のクラスはデストラクタにvirtualをつける※継承先のデストラクタが呼ばれなくなってしまうため
private:
    //垂直速力更新処理
    void UpdateVertialVelocity(float elapsedFrame);

    //垂直移動更新処理
    void UpdateVertialMove(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle);

    //水平速力更新処理
    void UpdateHorizontalVelocity(float elapsedFrame);

    //水平移動更新処理
    void UpdateHorizontalMove(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle);

protected:
    //移動処理
    void Move(float vx, float vz, float speed);

    //旋回処理
    void Turn(float elapsedTime, float vx, float vz, float speed, DirectX::XMFLOAT4& angle,DirectX::XMFLOAT3 up);

    //ジャンプ処理
    void Jump(float speed);

    //速力処理更新
    void UpdateVelocity(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle);


protected:  //継承先にアクセス可能にする

    //ジャンプ
    float gravity = -1.0f;
    DirectX::XMFLOAT3 velocity = { 0,0,0 }; //速力

    float friction = 0.5f;  //摩擦

    //慣性
    float acceleration = 1.0f;
    float maxMoveSpeed = 5.0f;
    float moveVecX = 0.0f;
    float moveVecZ = 0.0f;
    float airControl = 0.3f;

    float stepOffset = 1.0f;
    float slopeRate = 1.0f;

};