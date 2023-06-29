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
    void SetPosition(const DirectX::XMFLOAT3& position) { this->position_ = position; }
    const DirectX::XMFLOAT3& GetPosition() const { return position_; }

    // ワールドポジション
    void SetWorldPosition(const DirectX::XMFLOAT3& worldPosition) { this->worldPosition_ = worldPosition; }
    const DirectX::XMFLOAT3& GetWorldPosition() const { return worldPosition_; }

    // クォータニオン回転
    void SetRotation(const DirectX::XMFLOAT4& rotation) { this->rotation_ = rotation; }
    const DirectX::XMFLOAT4& GetRotation() const { return rotation_; }

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


    // スケール
    void SetScale(const DirectX::XMFLOAT3& scale) { this->scale_ = scale; }
    const DirectX::XMFLOAT3& GetScale() const { return scale_; }

    //ローカル行列
    void SetParentTransform(const DirectX::XMFLOAT4X4& parentTransform) { this->parentTransform_ = parentTransform; }
    const DirectX::XMFLOAT4X4& GetParentTransform() const { return parentTransform_; }

    //ワールド行列
    void SetTransform(const DirectX::XMFLOAT4X4& transform) { this->transform_ = transform; }
    const DirectX::XMFLOAT4X4& GetTransform() const { return transform_; }
    const DirectX::XMFLOAT3& GetUp() const {
        DirectX::XMFLOAT3 up = {transform_._21, transform_._22, transform_._23};
        DirectX::XMStoreFloat3(&up, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&up)));
        return up; 
    }
    const DirectX::XMFLOAT3& GetFront() const {
        DirectX::XMFLOAT3 front = {transform_._31, transform_._32, transform_._33};
        DirectX::XMStoreFloat3(&front, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&front)));
        return front;
    }
    const DirectX::XMFLOAT3& GetRight() const {
        DirectX::XMFLOAT3 right = {transform_._11, transform_._12, transform_._13};
        DirectX::XMStoreFloat3(&right, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&right)));
        return right;
    }

    //指定方向を向く
    void LookAtTransform(const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up = { 0,1,0 });

    //指定のUpに合わせる
    void SetUpTransform(const DirectX::XMFLOAT3& up);

private:
    DirectX::XMFLOAT3	position_ = DirectX::XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT3	worldPosition_ = DirectX::XMFLOAT3(0, 0, 0);

    QuaternionStruct	rotation_ = DirectX::XMFLOAT4(0, 0, 0, 1);   //基準はクォータニオン
    DirectX::XMFLOAT3   eulerRotation_ = DirectX::XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT3	scale_ = DirectX::XMFLOAT3(1, 1, 1);
    DirectX::XMFLOAT4X4	transform_ = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    DirectX::XMFLOAT4X4 parentTransform_ = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
};

