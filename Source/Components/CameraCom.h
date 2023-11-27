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

    // 開始処理
    void Start() override;

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
    const DirectX::XMFLOAT4X4& GetView() const { return view_; }

    //プロジェクション行列取得
    const DirectX::XMFLOAT4X4& GetProjection() const { return projection_; }

    //注視点取得
    const DirectX::XMFLOAT3& GetFocus()const { return focus_; }

    //上方向取得
    const DirectX::XMFLOAT3& GetUp()const { return up_; }

    //前方向取得
    const DirectX::XMFLOAT3& GetFront()const { return front_; }

    //右方向取得
    const DirectX::XMFLOAT3& GetRight()const { return right_; }

    //カメラシェイク実行
    void CameraShake(float power, float seconds) { 
        shakePower_ = power; 
        shakeSec_ = seconds;
    }

    //ヒットストップ
    void HitStop(float sec);
    bool GetIsHitStop() { return isHitStop_; }


private:
    //座標系
    DirectX::XMFLOAT4X4 view_ = {};
    DirectX::XMFLOAT4X4 projection_ = {};

    DirectX::XMFLOAT3 focus_ = {0,0,0};

    DirectX::XMFLOAT3 up_ = { 0,1,0 };
    DirectX::XMFLOAT3 front_ = { 0,0,1 };
    DirectX::XMFLOAT3 right_ = { 1,0,0 };

    bool isLookAt_ = false;


    //演出系
    //カメラシェイク
    float shakeSec_ = {};    //秒数
    float shakePower_ = {};  //強さ
    DirectX::XMFLOAT3 shakePos_ = {};

    //ヒットストップ
    float hitTimer_ = 0;
    float saveWorldSpeed_ = 0;
    bool isHitStop_ = false;
};