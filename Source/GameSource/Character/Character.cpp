#include "Character.h"
#include "GameSource\Stage\Stage.h"
#include <imgui.h>
#include "Logger.h"
#include "GameSource\Math\Mathf.h"
#include "GameSource\Stage\StageManager.h"

#include <cmath>

//移動処理
void Character::IsMove(float vx, float vz, float speed)
{
    //移動方向ベクトルを設定
    moveVecX_ = vx;
    moveVecZ_ = vz;

    //最大速度設定
    maxMoveSpeed_ = speed;
}

//ジャンプ処理
void Character::Jump(float speed)
{
    //上方向の力を設定
    velocity_.y = speed;
}

//速力処理更新
void Character::UpdateVelocity(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3& up)
{
    //経過フレーム
    float elapsedFrame = 120.0f * elapsedTime;

    //垂直速力更新処理
    UpdateVertialVelocity(elapsedFrame);

    //水平速力更新処理
    UpdateHorizontalVelocity(elapsedFrame);

    //垂直移動更新処理
    UpdateVertialMove(elapsedTime,position,angle,up);

    //水平移動更新処理
    UpdateHorizontalMove(elapsedTime, position, angle,up);
}

//垂直速力更新処理
void Character::UpdateVertialVelocity(float elapsedFrame)
{
    //重力処理
    velocity_.y += gravity_ * elapsedFrame;
}

//水平速力更新処理
void Character::UpdateHorizontalVelocity(float elapsedFrame)
{
    //XZ平面の速力を減速する
    float length = sqrtf(velocity_.x*velocity_.x+velocity_.z*velocity_.z);
    if (length > 0.0f)
    {
        //摩擦力
        float Friction = this->friction_ * elapsedFrame;

       ////空中にいるときは摩擦力を減らす
       // if (!isGround)Friction -= airControl;

        //摩擦による横方向の減速処理
        if (length > friction_)
        {
            //単位ベクトル
            float vx = velocity_.x / length;
            float vz = velocity_.z / length;

            velocity_.x -= vx * friction_;
            velocity_.z -= vz * friction_;
        }
        //横方向の速力が摩擦力以下になったので、速力を無効化
        else
        {
            velocity_.x = 0;
            velocity_.z = 0;
        }

    }

    //XZ平面の速力を加速する
    if (length <= maxMoveSpeed_)
    {
        //移動ベクトルがゼロベクトルでないなら加速する
        float moveVecLength = fabsf(moveVecX_) + fabsf(moveVecZ_);
        if (moveVecLength > 0.0f)
        {
            //加速力
            float acceleration = this->acceleration_ * elapsedFrame;
           
            ////空中にいるときは加速力を減らす
            //if (!isGround)acceleration -= airControl;
            
            //移動ベクトルによる加速処理
            velocity_.x += moveVecX_*acceleration;
            velocity_.z += moveVecZ_*acceleration;
            
            //最大速度制限
            DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&velocity_);
            Velocity = DirectX::XMVectorSetY(Velocity, 0);   //y軸を0に
            DirectX::XMVECTOR Length = DirectX::XMVector3Length(Velocity);

            float length = DirectX::XMVectorGetX(Length);
            if (length > maxMoveSpeed_)
            {
               Velocity = DirectX::XMVector3Normalize(Velocity);    //正規化して
               Velocity = DirectX::XMVectorScale(Velocity, maxMoveSpeed_);//最大速度に設定
               
               velocity_.x = DirectX::XMVectorGetX(Velocity);
               velocity_.z = DirectX::XMVectorGetZ(Velocity);
            }

            ////下り坂でガタガタしないようにする
            //if (isGround && slopeRate > 0.0f)
            //{
            //    velocity.y -= length * slopeRate * elapsedFrame;
            //}
        }
    }

    //移動ベクトルをリセット
    moveVecX_ = 0;
    moveVecZ_ = 0;
}

//垂直移動更新処理
void Character::UpdateVertialMove(float elapsedTime,DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3& up)
{
    //垂直方向の移動量
    float my = velocity_.y * elapsedTime;

    //キャラクターのY軸方向となる法線ベクトル
    DirectX::XMFLOAT3 normal = { 0,1,0 };

    slopeRate_ = 0.0f;   //坂道がたがた用

    //落下中
    if (my < 0.0f)
    {
        //レイの開始位置は足元より少し上
        DirectX::XMFLOAT3 start = { position.x,position.y + stepOffset_,position.z };
        //レイの終点位置は移動後の位置
        DirectX::XMFLOAT3 end = { position.x,position.y + my,position.z };

        //レイキャストによる地面判定
        HitResult hit;
        if (StageManager::Instance().RayCast(start, end, hit))
        {
            //法線ベクトル取得
            normal = hit.normal;

            //地面に接地している
            position = hit.position;

            //傾斜率の計算
            float normalLenghtXZ = sqrtf(
                hit.normal.x * hit.normal.x + hit.normal.z * hit.normal.z);
            slopeRate_ = 1.0f - 
                (hit.normal.y / (normalLenghtXZ + hit.normal.y));

            ////着地した
            //if (!isGround)
            //{
            //    OnLanding();
            //}
            //isGround = true;
            velocity_.y = 0.0f;
        }
        else
        {
            //空中に浮いている
            position.y += my;
            //isGround = false;
        }
    }

    //上昇中
    else if (my > 0.0f)
    {
        position.y += my;
        //isGround = false;
    }
    //地面の向きに沿うようにXZ軸回転
    {
        //Y軸が法線ベクトル方向に向くオイラー角回転を算出する
        float angleX, angleZ;
        angleX = atan2f( normal.z, normal.y);
        angleZ = atan2f(-normal.x, normal.y);

        //線形補完で滑らかに回転する
        angle.x = Mathf::Lerp(angle.x, angleX, 0.1f);
        angle.z = Mathf::Lerp(angle.z, angleZ, 0.1f);

        up = normal;
    }
}

//水平移動更新処理
void Character::UpdateHorizontalMove(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3& up)
{
    //水平速力量計算
    float velocityLengthXZ = fabsf(velocity_.x) + fabsf(velocity_.z);
    if (velocityLengthXZ > 0.0f)
    {
        //水平移動量
        float mx = velocity_.x * elapsedTime;
        float mz = velocity_.z * elapsedTime;

        //レイの開始位置と終点位置
        DirectX::XMFLOAT3 start = { position.x ,position.y + stepOffset_ ,position.z };
        
        DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);

        DirectX::XMFLOAT3 end = { start.x + mx ,start.y ,start.z + mz };

        //レイキャストによる壁判定
        HitResult hit;
        if (StageManager::Instance().RayCast(start, end, hit))
        {
            //壁までのベクトル
            DirectX::XMVECTOR StartHit = DirectX::XMLoadFloat3(&hit.position);
            DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
            DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(StartHit, End);

            //壁の法線
            DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&hit.normal);
            DirectX::XMFLOAT3 normal;
            DirectX::XMStoreFloat3(&normal, Normal);

            //Logger::Print("x:%.2f y:%.2f z:%.2f\n", normal.x, normal.y, normal.z);

            //入射ベクトルを法線に射影
            DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(Normal, Vec);

            //補正後の計算
            DirectX::XMVECTOR ND = DirectX::XMVectorScale(Normal, DirectX::XMVectorGetX(Dot));
            DirectX::XMVECTOR EndWall = DirectX::XMVectorScale(Normal, DirectX::XMVectorGetX(Dot)*1.2f);

            //補正後に物体があった場合止まる
            DirectX::XMStoreFloat3(&start, Start);
            DirectX::XMStoreFloat3(&end, EndWall);
            start.y += stepOffset_ * 5;
            end.y += stepOffset_ * 5;
            if (StageManager::Instance().RayCast(start, end, hit))return;  //物体があった場合return

            DirectX::XMVECTOR VecRay = DirectX::XMVectorSubtract(End, Start);
            DirectX::XMVECTOR R = DirectX::XMVectorAdd(VecRay, ND); //壁ずりべくとる取得



            //壁ずり移動
            position.x += DirectX::XMVectorGetX(R);
            position.z += DirectX::XMVectorGetZ(R);

            //コンパイル時定数(https://www.slideshare.net/GenyaMurakami/constexpr-10458089
            constexpr float epsilon = 0.001;

            position.x += normal.x * epsilon;
            position.z += normal.z * epsilon;

        }
        else
        {
            //移動
            position.x += mx;
            position.z += mz;
        }
    }
}

//旋回処理
void Character::Turn(float elapsedTime, float vx, float vz, float speed, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3 up)
{
    if (!std::isfinite(vx))return;

    speed *= elapsedTime;




    //進行ベクトルを単位ベクトル化
    float moveVecLength = sqrtf(vx * vx + vz * vz);
    if (moveVecLength <= 0)return;  //進行ベクトルがゼロベクトルの場合は処理する必要なし
    if (moveVecLength > 0.0f)
    {
        //単位ベクトル化
        vx /= moveVecLength;
        vz /= moveVecLength;
    }

    //自身の回転値から前方向を決める
    float frontX = sinf(angle.y);
    float frontZ = cosf(angle.y);

    //回転角を求めるため、２つの単位ベクトルの内積を計算する
    float dot = (vx * frontX) + (vz * frontZ);

    //内積値は-1.0~1.0で表現されており、２つの単位ベクトルの角度が
    //小さいほどに1.0に近づくという性質を利用して回転速度を調整する
    float rot = 1.0f - dot; //補正量
    if (rot > speed)rot = speed;

    //左右判定を行うために2つの単位ベクトルの外積を計算する
    float cross = (vx * frontZ) - (vz * frontX);

    //2Dの外積値が正の場合か負の場合によって左右判定が行える
    //左右判定を行うことによって左右回転を選択する
    if (cross < 0.0f)   //右
    {
        DirectX::XMVECTOR Angle = DirectX::XMLoadFloat4(&angle);
        DirectX::XMVECTOR AngleAdd = DirectX::XMQuaternionMultiply(DirectX::XMQuaternionRotationAxis({ up.x,up.y,up.z }, DirectX::XMConvertToRadians(-rot)), Angle);
        DirectX::XMStoreFloat4(&angle, AngleAdd);
    }
    else
    {
        DirectX::XMVECTOR Angle = DirectX::XMLoadFloat4(&angle);
        DirectX::XMVECTOR AngleAdd = DirectX::XMQuaternionMultiply(DirectX::XMQuaternionRotationAxis({ up.x,up.y,up.z }, DirectX::XMConvertToRadians(rot)), Angle);
        DirectX::XMStoreFloat4(&angle, AngleAdd);
    }

    //DirectX::XMVECTOR Angle = DirectX::XMLoadFloat4(&angle_);
    //DirectX::XMVECTOR AngleAdd = DirectX::XMQuaternionMultiply(DirectX::XMQuaternionRotationAxis({ up_.x,up_.y,up_.z }, 0.1f), Angle);
    //DirectX::XMStoreFloat4(&angle_, AngleAdd);

}

