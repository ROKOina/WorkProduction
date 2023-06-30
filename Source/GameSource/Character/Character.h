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
    void UpdateVertialMove(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3& up);

    //水平速力更新処理
    void UpdateHorizontalVelocity(float elapsedFrame);

    //水平移動更新処理
    void UpdateHorizontalMove(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3& up);

protected:
    //移動処理
    void IsMove(float vx, float vz, float speed);

    //旋回処理
    void Turn(float elapsedTime, float vx, float vz, float speed, DirectX::XMFLOAT4& angle,DirectX::XMFLOAT3 up);

    //ジャンプ処理
    void Jump(float speed);

    //速力処理更新
    void UpdateVelocity(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3& up);


protected:  //継承先にアクセス可能にする

    //ジャンプ
    float gravity_ = -1.0f;
    DirectX::XMFLOAT3 velocity_ = { 0,0,0 }; //速力

    float friction_ = 0.5f;  //摩擦

    //慣性
    float acceleration_ = 1.0f;
    float maxMoveSpeed_ = 5.0f;
    float moveVecX_ = 0.0f;
    float moveVecZ_ = 0.0f;
    float airControl_ = 0.3f;

    float stepOffset_ = 1.0f;
    float slopeRate_ = 1.0f;

};