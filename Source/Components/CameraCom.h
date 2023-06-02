#pragma once

#include <DirectXMath.h>

#include "System\Component.h"

//カメラ
class CameraCom : public Component
{
    //コンポーネントオーバーライド
public:
    CameraCom() {}
    ~CameraCom() {}

    // 名前取得
    const char* GetName() const override { return "Camera"; }

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

    //Cameraクラス
public:
    //指定方向を向く
    void SetLookAt(const DirectX::XMFLOAT3& focus,const DirectX::XMFLOAT3& up = { 0,1,0 });

    //パースペクティブ設定
    void SetPerspectiveFov(float fovY, float aspect, float nearZ, float farZ);

    //ビュー行列取得
    const DirectX::XMFLOAT4X4& GetView() const { return view; }

    //プロジェクション行列取得
    const DirectX::XMFLOAT4X4& GetProjection() const { return projection; }

    //注視点取得
    const DirectX::XMFLOAT3& GetFocus()const { return focus; }

    //上方向取得
    const DirectX::XMFLOAT3& GetUp()const { return up; }

    //前方向取得
    const DirectX::XMFLOAT3& GetFront()const { return front; }

    //右方向取得
    const DirectX::XMFLOAT3& GetRight()const { return right; }

private:
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;

    DirectX::XMFLOAT3 focus;

    DirectX::XMFLOAT3 up;
    DirectX::XMFLOAT3 front;
    DirectX::XMFLOAT3 right;
};