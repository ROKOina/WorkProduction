#pragma once

#include "System\Component.h"
#include "SystemStruct\QuaternionStruct.h"

class TransformCom : public Component
{
    //コンポーネントオーバーライド
public:
    TransformCom() {}
    ~TransformCom() {}

    // 名前取得
    const char* GetName() const override { return "Transform"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;


    //TransformComクラス
public:
    //行列更新
    void UpdateTransform();

    // ローカルポジション
    void SetLocalPosition(const DirectX::XMFLOAT3& position) {
        this->localPosition_ = position; 
        UpdateTransform();
    }
    const DirectX::XMFLOAT3& GetLocalPosition() const { return localPosition_; }

    // ワールドポジション
    void SetWorldPosition(const DirectX::XMFLOAT3& worldPosition) {
        //親がいない時はそのままポジションになる
        if (!GetGameObject()->GetParent()) {
            this->localPosition_ = worldPosition;
        }
        else{
            DirectX::XMMATRIX ParentTransformInverse = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&GetGameObject()->GetParent()->transform_->GetWorldTransform()));
            DirectX::XMVECTOR WorldPosition = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&worldPosition), ParentTransformInverse);
            DirectX::XMStoreFloat3(&this->localPosition_, WorldPosition);
        }
        UpdateTransform();
    }
    const DirectX::XMFLOAT3& GetWorldPosition() const { return worldPosition_; }

    // クォータニオン回転
    void SetRotation(const DirectX::XMFLOAT4& rotation) { this->rotation_ = rotation; }
    const DirectX::XMFLOAT4& GetRotation() { return rotation_.dxFloat4; }

    // オイラー回転(クォータニオン->オイラーはきついのでセットだけ)
    void SetEulerRotation(const DirectX::XMFLOAT3& setEuler) { 
        DirectX::XMFLOAT3 euler;
        euler.x = DirectX::XMConvertToRadians(setEuler.x);
        euler.y = DirectX::XMConvertToRadians(setEuler.y);
        euler.z = DirectX::XMConvertToRadians(setEuler.z);
        DirectX::XMVECTOR ROT = DirectX::XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
        DirectX::XMStoreFloat4(&rotation_.dxFloat4, ROT);
        euler.y = DirectX::XMConvertToDegrees(euler.y);
        euler.x = DirectX::XMConvertToDegrees(euler.x);
        euler.z = DirectX::XMConvertToDegrees(euler.z);
        eulerRotation_ = euler;
    }
    const DirectX::XMFLOAT3& GetEulerRotation() { return eulerRotation_; }

    // スケール
    void SetScale(const DirectX::XMFLOAT3& scale) { this->scale_ = scale; }
    const DirectX::XMFLOAT3& GetScale() const { return scale_; }

    //親行列
    void SetParentTransform(const DirectX::XMFLOAT4X4& parentTransform) { this->parentTransform_ = parentTransform; }
    const DirectX::XMFLOAT4X4& GetParentTransform() const { return parentTransform_; }

    //ローカル行列
    const DirectX::XMFLOAT4X4& GetLocalTransform() const { return localTransform_; }

    //ワールド行列
    void SetWorldTransform(const DirectX::XMFLOAT4X4& transform) { this->worldTransform_ = transform; }
    const DirectX::XMFLOAT4X4& GetWorldTransform() { return worldTransform_; }
    const DirectX::XMFLOAT3 GetWorldUp() const {
        DirectX::XMFLOAT3 up = {worldTransform_._21, worldTransform_._22, worldTransform_._23};
        DirectX::XMStoreFloat3(&up, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&up)));
        return up; 
    }
    const DirectX::XMFLOAT3 GetWorldFront() const {
        DirectX::XMFLOAT3 front = {worldTransform_._31, worldTransform_._32, worldTransform_._33};
        DirectX::XMStoreFloat3(&front, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&front)));
        return front;
    }
    const DirectX::XMFLOAT3 GetWorldRight() const {
        DirectX::XMFLOAT3 right = {worldTransform_._11, worldTransform_._12, worldTransform_._13};
        DirectX::XMStoreFloat3(&right, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&right)));
        return right;
    }

    //指定方向を向く
    void LookAtTransform(const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up = { 0,1,0 });

    //指定のUpに合わせる
    void SetUpTransform(const DirectX::XMFLOAT3& up);

private:
    DirectX::XMFLOAT3	localPosition_ = DirectX::XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT3	worldPosition_ = DirectX::XMFLOAT3(0, 0, 0);
    
    QuaternionStruct	rotation_ = DirectX::XMFLOAT4(0, 0, 0, 1);   //基準はクォータニオン
    DirectX::XMFLOAT3   eulerRotation_ = DirectX::XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT3	scale_ = DirectX::XMFLOAT3(1, 1, 1);
    DirectX::XMFLOAT4X4	worldTransform_ = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    DirectX::XMFLOAT4X4	localTransform_ = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    DirectX::XMFLOAT4X4 parentTransform_ = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
};

