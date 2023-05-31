#include "Character.h"
#include "GameSource\Stage\Stage.h"
#include <imgui.h>
#include "Logger.h"
#include "GameSource\Math\Mathf.h"
#include "GameSource\Stage\StageManager.h"

//行列更新処理
void Character::UpdateTransform()
{
    //スケール行列を作成
    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

    //回転行列を作成
    DirectX::XMMATRIX X = DirectX::XMMatrixRotationX(angle.x);
    DirectX::XMMATRIX Y = DirectX::XMMatrixRotationY(angle.y);
    DirectX::XMMATRIX Z = DirectX::XMMatrixRotationZ(angle.z);
    DirectX::XMMATRIX R = X * Y * Z;
    
    //位置行列を作成
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

    //3つの行列を組み合わせて、ワールド行列を作成
    DirectX::XMMATRIX W = S * R * T;
    //計算したワールド行列を取り出す
    DirectX::XMStoreFloat4x4(&transform, W);
}

//移動処理
void Character::Move(float vx, float vz, float speed)
{
    //移動方向ベクトルを設定
    moveVecX = vx;
    moveVecZ = vz;

    //最大速度設定
    maxMoveSpeed = speed;
}

//ジャンプ処理
void Character::Jump(float speed)
{
    //上方向の力を設定
    velocity.y = speed;
}

//衝撃を与える
void Character::AddImpulse(const DirectX::XMFLOAT3& impulse)
{
    //速力に力を加える
    velocity.x += impulse.x;
    velocity.y += impulse.y;
    velocity.z += impulse.z;
}

//速力処理更新
void Character::UpdateVelocity(float elapsedTime)
{
    //経過フレーム
    float elapsedFrame = 60.0f * elapsedTime;

    //垂直速力更新処理
    UpdateVertialVelocity(elapsedFrame);

    //水平速力更新処理
    UpdateHorizontalVelocity(elapsedFrame);

    //垂直移動更新処理
    UpdateVertialMove(elapsedTime);

    //水平移動更新処理
    UpdateHorizontalMove(elapsedTime);
}

//垂直速力更新処理
void Character::UpdateVertialVelocity(float elapsedFrame)
{
    //重力処理
    velocity.y += gravity * elapsedFrame;
}

//水平速力更新処理
void Character::UpdateHorizontalVelocity(float elapsedFrame)
{
    //XZ平面の速力を減速する
    float length = sqrtf(velocity.x*velocity.x+velocity.z*velocity.z);
    if (length > 0.0f)
    {
        //摩擦力
        float Friction = this->friction * elapsedFrame;

       //空中にいるときは摩擦力を減らす
        if (!isGround)Friction -= airControl;

        //摩擦による横方向の減速処理
        if (length > friction)
        {
            //単位ベクトル
            float vx = velocity.x / length;
            float vz = velocity.z / length;

            velocity.x -= vx * friction;
            velocity.z -= vz * friction;
        }
        //横方向の速力が摩擦力以下になったので、速力を無効化
        else
        {
            velocity.x = 0;
            velocity.z = 0;
        }

    }

    //XZ平面の速力を加速する
    if (length <= maxMoveSpeed)
    {
        //移動ベクトルがゼロベクトルでないなら加速する
        float moveVecLength = fabsf(moveVecX) + fabsf(moveVecZ);
        if (moveVecLength > 0.0f)
        {
            //加速力
            float acceleration = this->acceleration * elapsedFrame;
           
            //空中にいるときは加速力を減らす
            if (!isGround)acceleration -= airControl;
            
            //移動ベクトルによる加速処理
            velocity.x += moveVecX*acceleration;
            velocity.z += moveVecZ*acceleration;
            
            //最大速度制限
            DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&velocity);
            Velocity = DirectX::XMVectorSetY(Velocity, 0);   //y軸を0に
            DirectX::XMVECTOR Length = DirectX::XMVector3Length(Velocity);

            float length = DirectX::XMVectorGetX(Length);
            //float length = fabsf(velocity.x) + fabsf(velocity.z);
            if (length > maxMoveSpeed)
            {
               Velocity = DirectX::XMVector3Normalize(Velocity);    //正規化して
               Velocity = DirectX::XMVectorScale(Velocity, maxMoveSpeed);//最大速度に設定
               
               velocity.x = DirectX::XMVectorGetX(Velocity);
               velocity.z = DirectX::XMVectorGetZ(Velocity);
            }

            //下り坂でガタガタしないようにする
            if (isGround && slopeRate > 0.0f)
            {
                velocity.y -= length * slopeRate * elapsedFrame;
            }
        }
    }

    //移動ベクトルをリセット
    moveVecX = 0;
    moveVecZ = 0;
}

//垂直移動更新処理
void Character::UpdateVertialMove(float elapsedTime)
{
    //垂直方向の移動量
    float my = velocity.y * elapsedTime;

    //キャラクターのY軸方向となる法線ベクトル
    DirectX::XMFLOAT3 normal = { 0,1,0 };

    slopeRate = 0.0f;   //坂道がたがた用

    //落下中
    if (my < 0.0f)
    {
        //レイの開始位置は足元より少し上
        DirectX::XMFLOAT3 start = { position.x,position.y + stepOffset,position.z };
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

            //回転
            angle.x += hit.rotation.x;
            angle.y += hit.rotation.y;
            angle.z += hit.rotation.z;

            //傾斜率の計算
            float normalLenghtXZ = sqrtf(
                hit.normal.x * hit.normal.x + hit.normal.z * hit.normal.z);
            slopeRate = 1.0f - 
                (hit.normal.y / (normalLenghtXZ + hit.normal.y));

            //着地した
            if (!isGround)
            {
                OnLanding();
            }
            isGround = true;
            velocity.y = 0.0f;
        }
        else
        {
            //空中に浮いている
            position.y += my;
            isGround = false;
        }
    }

    //上昇中
    else if (my > 0.0f)
    {
        position.y += my;
        isGround = false;
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
    }
}

//水平移動更新処理
void Character::UpdateHorizontalMove(float elapsedTime)
{
    //水平速力量計算
    float velocityLengthXZ = fabsf(velocity.x) + fabsf(velocity.z);
    if (velocityLengthXZ > 0.0f)
    {
        //水平移動量
        float mx = velocity.x * elapsedTime;
        float mz = velocity.z * elapsedTime;

        //レイの開始位置と終点位置
        DirectX::XMFLOAT3 start = { position.x ,position.y + stepOffset ,position.z };
        
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
            start.y += stepOffset * 5;
            end.y += stepOffset * 5;
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
void Character::Turn(float elapsedTime, float vx, float vz, float speed)
{
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
        angle.y -= rot;
    }
    else
    {
        angle.y += rot;
    }

}

//ダメージを与える
bool Character::ApplyDamage(int damage, float invincibleTime)
{
    //ダメージが0の場合は健康状態を変更する必要がない
    if (damage <= 0)return false;

    //死亡している場合は健康状態を変更しない
    if (health <= 0)return false;

    //無敵時間
    if (invincibleTimer > 0)return false;

    //ダメージ処理
    health -= damage;

    //無敵時間処理
    invincibleTimer = invincibleTime;

    //死亡通知
    if (health <= 0)
    {
        OnDead();
    }
    //ダメージ通知
    else
    {
        OnDamage();
    }

    //健康状態が変更した場合はtrueを返す
    return true;
}

//無敵時間更新
void Character::UpdateInvincibleTimer(float elapsedTime)
{
    if (invincibleTimer > 0.0f)
    {
        invincibleTimer -= elapsedTime;
    }
}