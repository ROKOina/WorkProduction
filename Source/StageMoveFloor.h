#pragma once

#include "Graphics\Model.h"
#include "Stage.h"

//移動床ステージ
class StageMoveFloor :public Stage
{
public:
    StageMoveFloor();
    ~StageMoveFloor();

    //更新処理
    virtual void Update(float elapsedTime)override;

    //描画処理
    virtual void Render(ID3D11DeviceContext* dc, Shader* shader)override;

    //レイキャスト
    virtual bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit)override;

    //スタート位置設定
    void SetStartPoint(const DirectX::XMFLOAT3& start) { this->start=start; }
    
    //ゴール位置設定
    void SetGoalPoint(const DirectX::XMFLOAT3& goal) { this->goal =goal; }
    
    //トルク設定
    void SetTorque(const DirectX::XMFLOAT3& torque) { this->torque=torque; }

private:
    //行列更新処理
    void UpdateTransform();

private :
    Model* model = nullptr;
    DirectX::XMFLOAT3 position = { 0,0,0 };
    DirectX::XMFLOAT3 angle = { 0,0,0 };
    DirectX::XMFLOAT3 scale = { 1,1,1 };
    DirectX::XMFLOAT4X4 oldTransform = { 1,0,0,0
                                      ,0,1,0,0
                                      ,0,0,1,0
                                      ,0,0,0,1 };
    DirectX::XMFLOAT4X4 transform = { 1,0,0,0
                                      ,0,1,0,0
                                      ,0,0,1,0
                                      ,0,0,0,1 };

    DirectX::XMFLOAT3 torque = { 0,0,0 };
    DirectX::XMFLOAT3 start = { 0,0,0 };
    DirectX::XMFLOAT3 goal = { 0,0,0 };

    float moveSpeed = 2.0f;
    float moveRate = 0.0f;
    DirectX::XMFLOAT3 oldAngle = { 0,0,0 };
};