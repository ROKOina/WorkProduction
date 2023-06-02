#pragma once

#include "System\Component.h"

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
    void SetPosition(const DirectX::XMFLOAT3& position) { this->position = position; }
    const DirectX::XMFLOAT3& GetPosition() const { return position; }

    // ワールドポジション
    const DirectX::XMFLOAT3& GetWorldPosition() const { return worldPosition; }

    // 回転
    void SetRotation(const DirectX::XMFLOAT4& rotation) { this->rotation = rotation; }
    const DirectX::XMFLOAT4& GetRotation() const { return rotation; }

    // スケール
    void SetScale(const DirectX::XMFLOAT3& scale) { this->scale = scale; }
    const DirectX::XMFLOAT3& GetScale() const { return scale; }

    //ローカル行列
    void SetParentTransform(const DirectX::XMFLOAT4X4& parentTransform) { this->parentTransform = parentTransform; }
    const DirectX::XMFLOAT4X4& GetParentTransform() const { return parentTransform; }

    //ワールド行列
    void SetTransform(const DirectX::XMFLOAT4X4& transform) { this->transform = transform; }
    const DirectX::XMFLOAT4X4& GetTransform() const { return transform; }

private:
    DirectX::XMFLOAT3	position = DirectX::XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT3	worldPosition = DirectX::XMFLOAT3(0, 0, 0);

    DirectX::XMFLOAT4	rotation = DirectX::XMFLOAT4(0, 0, 0, 1);
    DirectX::XMFLOAT3	scale = DirectX::XMFLOAT3(1, 1, 1);
    DirectX::XMFLOAT4X4	transform = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    DirectX::XMFLOAT4X4	parentTransform = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

};