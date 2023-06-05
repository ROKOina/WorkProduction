#include "PlayerCom.h"
#include "Input/Input.h"
#include "../CameraCom.h"
#include "Components\TransformCom.h"
#include <imgui.h>

// 開始処理
void PlayerCom::Start()
{

}

// 更新処理
void PlayerCom::Update(float elapsedTime)
{
    //カメラをプレイヤーにフォーカスする
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");
    std::shared_ptr<CameraCom> camera = cameraObj->GetComponent<CameraCom>();
    camera->SetLookAt(GetGameObject()->transform->GetPosition());

    //プレイヤーのワールドポジションを取得
    DirectX::XMFLOAT3 wp = GetGameObject()->transform->GetWorldPosition();
    wp.z -= 10;
    wp.y += 5;
    cameraObj->transform->SetPosition(wp);

    InputMove(elapsedTime);

    //速力処理更新
    DirectX::XMFLOAT3 p = GetGameObject()->transform->GetPosition();
    DirectX::XMFLOAT4 r = GetGameObject()->transform->GetRotation();
    //UpdateVelocity(elapsedTime, p, r);
    UpdateVelocity(elapsedTime, p, DirectX::XMFLOAT4(0, 0, 0, 0));
    GetGameObject()->transform->SetPosition(p);
    GetGameObject()->transform->SetRotation(r);
}

// GUI描画
void PlayerCom::OnGUI()
{
    DirectX::XMFLOAT3 a = GetGameObject()->transform->GetUp();
    ImGui::InputFloat3("up", &a.x);
    ImGui::DragFloat("turnSpeed", &turnSpeed);
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
DirectX::XMFLOAT3 PlayerCom::GetMoveVec()
{
    //入力情報を取得
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();

    //カメラ方向とスティックの入力値によって進行方向を計算する
    std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
    const DirectX::XMFLOAT3& cameraRight = camera->GetRight();
    const DirectX::XMFLOAT3& cameraFront = camera->GetFront();

    //移動ベクトルはXZ平面に水平なベクトルになるようにする

    //カメラ右方向ベクトルをXZ単位ベクトルに変換
    float cameraRightX = cameraRight.x;
    float cameraRightZ = cameraRight.z;
    float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
    if (cameraRightLength > 0.0f)
    {
        //単位ベクトル化
        cameraRightX /= cameraRightLength;
        cameraRightZ /= cameraRightLength;
    }

    //カメラ前方向ベクトルをXZ単位ベクトルに変換
    float cameraFrontX = cameraFront.x;
    float cameraFrontZ = cameraFront.z;
    float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
    if (cameraFrontLength > 0.0f)
    {
        //単位ベクトル化
        cameraFrontX /= cameraFrontLength;
        cameraFrontZ /= cameraFrontLength;
    }

    //スティックの水平入力値をカメラ右方向に反映し、
    //スティックの垂直入力値をカメラ前方向に反映し、
    //進行ベクトルを計算する
    DirectX::XMFLOAT3 vec;
    //XZ軸方向に移動
    vec.x = cameraFrontX * ay + cameraRightX * ax;
    vec.z = cameraFrontZ * ay + cameraRightZ * ax;
    //Y軸方向には移動しない
    vec.y = 0.0f;

    return vec;
}

//移動入力処理
bool PlayerCom::InputMove(float elapsedTime)
{
    //進行ベクトル取得
    DirectX::XMFLOAT3 moveVec = GetMoveVec();

    //移動処理
    Move(moveVec.x, moveVec.z, moveSpeed);

    //旋回処理
    DirectX::XMFLOAT4 r = GetGameObject()->transform->GetRotation();
    Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed, r, GetGameObject()->transform->GetUp());
    //Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed, DirectX::XMFLOAT4(0, 0, 0, 0), { 0,1,0 });
    GetGameObject()->transform->SetRotation(r);

    //進行ベクトルがゼロベクトルでない場合は入力された
    return moveVec.x * moveVec.x + moveVec.y * moveVec.y + moveVec.z * moveVec.z;
}

