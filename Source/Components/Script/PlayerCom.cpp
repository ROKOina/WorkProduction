#include "PlayerCom.h"
#include "Input/Input.h"
#include "../CameraCom.h"

// 開始処理
void PlayerCom::Start()
{

}

// 更新処理
void PlayerCom::Update(float elapsedTime)
{
    std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
    camera->SetLookAt(GetGameObject()->GetPosition());
}

// GUI描画
void PlayerCom::OnGUI()
{

}

//カメラ
void PlayerCom::CameraControll(float elapsedTime)
{
    //GamePad& gamePad = Input::Instance().GetGamePad();
    //float ax = gamePad.GetAxisRX();
    //float ay = gamePad.GetAxisRY();
    ////カメラの回転速度
    //float speed = rollSpeed * elapsedTime;

    ////スティックの入力値を合わせてX軸とY軸を回転
    //if (ay > 0)
    //{
    //    angle.x += speed;
    //}
    //if (ay < 0)
    //{
    //    angle.x -= speed;
    //}
    //if (ax > 0)
    //{
    //    angle.y += speed;
    //}
    //if (ax < 0)
    //{
    //    angle.y -= speed;
    //}

    ////X軸のカメラ回転を制限
    //if (angle.x > maxAngleX)
    //{
    //    angle.x = maxAngleX;
    //}
    //if (angle.x < minAngleX)
    //{
    //    angle.x = minAngleX;
    //}

    ////Y軸のカメラ回転値を-3.14～3.14に収まるようにする
    //if (angle.y < -DirectX::XM_PI)
    //{
    //    angle.y += DirectX::XM_2PI;
    //}
    //if (angle.y > DirectX::XM_PI)
    //{
    //    angle.y -= DirectX::XM_2PI;
    //}

    ////カメラ回転値を回転行列に変換
    //DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);

    ////回転行列から前方向ベクトルを取り出す
    //DirectX::XMVECTOR Front = Transform.r[2];
    //DirectX::XMFLOAT3 front;
    //DirectX::XMStoreFloat3(&front, Front);

    ////注視点から後ろベクトル方向に一定距離離れたカメラ視点を求める
    //DirectX::XMFLOAT3 eye;
    //eye.x = target.x - front.x * range;
    //eye.y = target.y - front.y * range;
    //eye.z = target.z - front.z * range;

    //std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
    //camera->set
}


//スティック入力値から移動ベクトルを取得
DirectX::XMFLOAT3 PlayerCom::GetMoveVec() const
{
    //入力情報を取得
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();

    return {};
}

//移動入力処理
bool PlayerCom::InputMove(float elapsedTime)
{
    return false;
}

