#include <imgui.h>
#include "Player.h"
#include "Input/Input.h"
#include "Camera.h"
#include "Graphics/Graphics.h"
#include "EnemyManager.h"
#include "Collision.h"
#include "ProjectileStraite.h"
#include "ProjectileHoming.h"

static Player* instance = nullptr;

//インスタンス取得
Player& Player::Instance()
{
    return *instance;
}

//コンストラクタ
Player::Player()
{
    //インスタンスポインタ設定
    instance = this;

    model = new Model("Data/Model/Jammo/Jammo.mdl");
    //model->PlayAnimation(0,true);

    //model = new Model("Data/Model/Mr.Incredible/Mr.Incredible.mdl");

    //モデルが大きいのでスケーリング
    scale.x = scale.y = scale.z = 0.01f;

    //ヒットエフェクト読み込み
    hitEffect = new Effect("Data/Effect/Hit.efk");

    //待機ステートへ遷移
    TransitionIdleState();
}

//デストラクタ
Player::~Player()
{
    delete hitEffect;
    delete model;
}


//スティック入力値
DirectX::XMFLOAT3 Player::GetMoveVec() const
{
    //入力情報を取得
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();

    //カメラ方向とスティックの入力値によって進行方向を計算する
    Camera& camera = Camera::Instance();
    const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
    const DirectX::XMFLOAT3& cameraFront = camera.GetFront();

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
    vec.x = cameraFrontX * ay + cameraRightX * ax;
    vec.z = cameraFrontZ * ay + cameraRightZ * ax;
    //Y軸方向には移動しない
    vec.y = 0.0f;

    return vec;
}

//更新処理
void Player::Update(float elapsedTime)
{
    ////移動入力処理
    //InputMove(elapsedTime);

    ////ジャンプ入力処理
    //InputJump();
    
    ////弾丸入力処理
    //InputProjectile();

    //ステート毎の処理
    switch (state)
    {
    case State::Idle:
        UpdateIdleState(elapsedTime);
        break;

    case State::Move:
        UpdateMoveState(elapsedTime);
        break;

    case State::Jump:
        UpdateJumpState(elapsedTime);
        break;

    case State::Land:
        UpdateLandState(elapsedTime);
        break;

    case State::Attack:
        UpdateAttackState(elapsedTime);
        break;

    case State::Damage:
        UpdateDamageState(elapsedTime);
        break;

    case State::Death:
        UpdateDeathState(elapsedTime);
        break;

    case State::Revive:
        UpdateReviveState(elapsedTime);
        break;

    }

    //速力処理更新
    UpdateVelocity(elapsedTime);

    //無敵時間更新
    UpdateInvincibleTimer(elapsedTime);

    //弾丸更新処理
    projectileManager.Update(elapsedTime);

    //プレイヤーと敵との衝突処理
    CollisionPlayerVsEnemies();

    //弾丸と敵の衝突判定
    CollisionProjectileVsEnemies();

    //オブジェクトの行列を更新
    UpdateTransform();

    //モデルアニメーション更新処理
    model->UpdateAnimetion(elapsedTime);

    //モデル行列更新
    model->UpdateTransform(transform);  //位置行列を渡す
}


bool Player::InputMove(float elapsedTime)
{
    //進行ベクトル取得
    DirectX::XMFLOAT3 moveVec = GetMoveVec();

    //移動処理
    Move(moveVec.x, moveVec.z, moveSpeed);

    //旋回処理
    Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);

    //進行ベクトルがゼロベクトルでない場合は入力された
    return moveVec.x * moveVec.x + moveVec.y * moveVec.y + moveVec.z * moveVec.z;
}

//ジャンプ入力処理
bool Player::InputJump()
{
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_A) 
    {
        if (JumpCount < JumpLimit)  //ジャンプ回数制限
        {
            Jump(jumpSpeed);
            ++JumpCount;

            //ジャンプ入力した
            return true;
        }
    }
    return false;
}

//攻撃入力処理
bool Player::InputAttack()
{
    GamePad& gamePad = Input::Instance().GetGamePad();

    if (gamePad.GetButtonDown() & GamePad::BTN_B)
    {
        return true;
    }
    return false;
}

//着地した時に呼ばれる
void Player::OnLanding()
{
    JumpCount = 0;

    //ダメージ、死亡ステート時は着地した時にステート遷移しないようにする
    if (state != State::Damage && state != State::Death)
    {
        //着地ステートへ遷移
        TransitionLandState();
    }
}

//ダメージを受けた時に呼ばれる
void Player::OnDamage()
{
    //ダメージステートへ遷移
    TransitionDamageState();
}

//死亡した時に呼ばれる
void Player::OnDead()
{
    //死亡した時に呼ばれる
    TransitionDeathState();
}

////ジャンプ処理
//void Player::Jump(float speed)
//{
//    //上方向の力を設定
//    velocity.y = speed;
//}
//
////速力処理更新
//void Player::UpdateVelocity(float elapsedTime)
//{
//    //経過フレーム
//    float elapsedFrame = 60.0f * elapsedTime;
//
//    //重力処理
//    velocity.y += gravity * elapsedFrame;
//
//    //移動処理
//    position.y += velocity.y * elapsedTime;
//
//    //地面判定
//    if (position.y < 0.0f)
//    {
//        position.y = 0.0f;
//        velocity.y = 0.0f;
//    }
//}
//
////旋回処理
//void Player::Turn(float elapsedTime, float vx, float vz, float speed)
//{
//    speed *= elapsedTime;
//
//
//    //進行ベクトルを単位ベクトル化
//    float moveVecLength = sqrtf(vx * vx + vz * vz);
//    if (moveVecLength <= 0)return;  //進行ベクトルがゼロベクトルの場合は処理する必要なし
//    if (moveVecLength > 0.0f)
//    {
//        //単位ベクトル化
//        vx /= moveVecLength;
//        vz /= moveVecLength;
//    }
//
//    //自身の回転値から前方向を決める
//    float frontX = sinf(angle.y);
//    float frontZ = cosf(angle.y);
//
//    //回転角を求めるため、２つの単位ベクトルの内積を計算する
//    float dot = (vx * frontX) + (vz * frontZ);
//
//    //内積値は-1.0~1.0で表現されており、２つの単位ベクトルの角度が
//    //小さいほどに1.0に近づくという性質を利用して回転速度を調整する
//    float rot = 1.0f - dot; //補正量
//    if (rot > speed)rot = speed;
//
//    //左右判定を行うために2つの単位ベクトルの外積を計算する
//    float cross = (vx * frontZ) - (vz * frontX);
//
//    //2Dの外積値が正の場合か負の場合によって左右判定が行える
//    //左右判定を行うことによって左右回転を選択する
//    if (cross < 0.0f)   //右
//    {
//        angle.y -= rot;
//    }
//    else
//    {
//        angle.y += rot;
//    }
//
//}


//弾丸入力処理
void Player::InputProjectile()
{
    GamePad& gamePad = Input::Instance().GetGamePad();

    //直進弾丸発射
    if (gamePad.GetButtonDown() & GamePad::BTN_X|| gamePad.GetButtonDown() & GamePad::BTN_Y)    //C:まっすぐ、V:追尾
    {
        //前方向
        DirectX::XMFLOAT3 dir;  //Z方向のtransformを入れる
        dir.x = sinf(angle.y);
        dir.y = 0;
        dir.z = cosf(angle.y);
        //発射位置（プレイヤーの腰あたり）
        DirectX::XMFLOAT3 pos;
        pos.x = position.x;
        pos.y = position.y + GetHeight() / 2;
        pos.z = position.z;
        if (gamePad.GetButtonDown() & GamePad::BTN_Y) {
            //ターゲット(デフォルトではプレイヤーの前方
            DirectX::XMFLOAT3 target;
            target.x = position.x * 2;
            target.y = position.y + GetHeight() / 2 * 2;
            target.z = position.z * 2;

            //一番近くの敵をターゲットにする
            EnemyManager& enemyManager = EnemyManager::Instance();
            int enemyCount = enemyManager.GetEnemyCount();  //敵の数を持ってくる
            if (enemyCount > 0) //敵がいた場合
            {
                for (int e = 0; e < enemyCount; ++e)    //敵の数だけくりかえす
                {
                    if (e == 0) //１体目だけターゲットに入れて戻る
                    {
                        target = enemyManager.GetEnemy(e)->GetPosition();    //targetに入れる
                        continue;
                    }

                    //敵のベクトル取得
                    DirectX::XMVECTOR enemy0 = DirectX::XMLoadFloat3(&target);
                    //次の敵のベクトル取得
                    DirectX::XMVECTOR enemy = DirectX::XMLoadFloat3(&enemyManager.GetEnemy(e)->GetPosition());
                    //キャラのベクトル取得
                    DirectX::XMVECTOR ziki = DirectX::XMLoadFloat3(&pos);
                    //大きさを測る
                    DirectX::XMVECTOR E0Z = DirectX::XMVectorSubtract(enemy0, ziki);
                    E0Z = DirectX::XMVector3Length(E0Z);
                    float length0;
                    DirectX::XMStoreFloat(&length0, E0Z);
                    DirectX::XMVECTOR EZ = DirectX::XMVectorSubtract(enemy, ziki);
                    EZ = DirectX::XMVector3Length(EZ);
                    float length;
                    DirectX::XMStoreFloat(&length, EZ);

                    //近い方をターゲットにする
                    if (length < length0)target = enemyManager.GetEnemy(e)->GetPosition();

                }
            }
        //発射
        ProjectileHoming* projectile = new ProjectileHoming(&projectileManager);
        projectile->Launch(dir, pos,target);
        }
        else
        {
            //発射
            ProjectileStraite* projectile = new ProjectileStraite(&projectileManager);
            projectile->Launch(dir, pos);
        }
        ////projectileManager.Register(projectile);
    }
}

//プレイヤーとの当たり判定
void Player::CollisionPlayerVsEnemies()
{
    EnemyManager& enemyManager = EnemyManager::Instance();

    //全ての数と総当たりで衝突処理
    int enemyCount = enemyManager.GetEnemyCount();
    for (int i = 0; i < enemyCount; ++i)
    {
        Enemy* enemy = enemyManager.GetEnemy(i);

        //衝突処理
        DirectX::XMFLOAT3 outPosition;
        //if (Collision::IntersectSphereVsSphere(   //円
        //    this->GetPosition(), this->GetRadius(),
        //    enemy->GetPosition(), enemy->GetRadius(),
        //    outPosition))
        //{
        //    //押し出し後の位置設定
        //    enemy->SetPosition(outPosition);
        //}

        if (Collision::IntersectCylinder(
            this->GetPosition(), this->GetRadius(), this->GetHeight(),
            enemy->GetPosition(), enemy->GetRadius(), enemy->GetHeight(),
            outPosition
        ))
        {
            DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&this->GetPosition());
            DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->GetPosition());
            DirectX::XMVECTOR EP = DirectX::XMVectorSubtract(P, E);
            DirectX::XMVECTOR N = DirectX::XMVector3Normalize(EP);

            //押し出し後の位置設定
            if(DirectX::XMVectorGetY(N)>0.8f) Jump(10);
            /*if(IsGround())*/
            else enemy->SetPosition(outPosition);
        }
    }
}

//弾丸と敵の衝突処理
void Player::CollisionProjectileVsEnemies()
{
    EnemyManager& enemyManager = EnemyManager::Instance();

    //全ての弾丸と全ての敵を総当たりで衝突処理
    int projectileCount = projectileManager.GetProjectileCount();
    int enemyCount = enemyManager.GetEnemyCount();
    for (int i = 0; i < projectileCount; ++i)
    {
        Projectile* projectile = projectileManager.GetProjectile(i);
        for (int j = 0; j < enemyCount; ++j)
        {
            Enemy* enemy = enemyManager.GetEnemy(j);
            //衝突処理
            DirectX::XMFLOAT3 outPosition;
            if (Collision::IntersectSphereVsCylider(
                projectile->GetPosition(),
                projectile->GetRadius(),
                enemy->GetPosition(),
                enemy->GetRadius(),
                enemy->GetHeight(),
                outPosition))
            {
                //ダメージを与える
                if (enemy->ApplyDamage(1, 0.5f))
                {
                    //吹き飛ばす
                    {
                        DirectX::XMFLOAT3 impulse;

                        DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&projectile->GetPosition());
                        DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&enemy->GetPosition());
                        DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);  //引き算で方向を求めるa->b
                        Vec = DirectX::XMVector3Normalize(Vec);
                        Vec = DirectX::XMVectorScale(Vec, 10.0f);   //球と敵の距離を吹き飛ばす
                        DirectX::XMStoreFloat3(&impulse, Vec);

                        impulse.y = 3.0f;//少しジャンプ
                        enemy->AddImpulse(impulse);
                    }

                        //ヒットエフェクト再生
                    {
                        DirectX::XMFLOAT3 e = enemy->GetPosition();
                        e.y += enemy->GetHeight() * 0.5f;
                        hitEffect->Play(e);
                    }

                    //弾丸破棄
                    projectile->Destroy();
                }
            }
        }
    }
}

//描画更新
void Player::Render(ID3D11DeviceContext* dc, Shader* shader)
{
    shader->Draw(dc, model);

    //弾丸描画処理
    projectileManager.Render(dc, shader);
}

//デバッグプリミティブ描画
void Player::DrawDebugPrimitive()
{
    if (!VisibleDebugPrimitive)return;
    DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();

    //衝突判定用のデバッグ球を描画
    //debugRenderer->DrawSphere(position, radius, DirectX::XMFLOAT4(0, 0, 0, 1));  
    //衝突判定用のデバッグ円柱を描画
    debugRenderer->DrawCylinder(position, radius, height, DirectX::XMFLOAT4(0, 0, 0, 1));  
    
    //弾丸デバッグプリミティブ描画
    projectileManager.DrawDebugPrimitive();

    //攻撃衝突用の左手ノードのデバッグ球を描画
    if (attackCollisionFlag)
    {
        Model::Node* leftHandBone = model->FindNode("mixamorig:LeftHand");
        debugRenderer->DrawSphere(DirectX::XMFLOAT3(
            leftHandBone->worldTransform._41,
            leftHandBone->worldTransform._42,
            leftHandBone->worldTransform._43),
            leftHandRadius,
            DirectX::XMFLOAT4(1, 0, 0, 1));
    }
}

//デバッグ用GUI描画
void Player::DrawDebugGUI()
{
    //ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    //ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

    //if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
    if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("VisibleDebugPrimitive", &VisibleDebugPrimitive);
            //位置
            ImGui::InputFloat3("Position", &position.x);
            //回転
            DirectX::XMFLOAT3 a;
            a.x = DirectX::XMConvertToDegrees(angle.x);
            a.y = DirectX::XMConvertToDegrees(angle.y);
            a.z = DirectX::XMConvertToDegrees(angle.z);
            ImGui::InputFloat3("Angle", &a.x);
            angle.x = DirectX::XMConvertToRadians(a.x);
            angle.y = DirectX::XMConvertToRadians(a.y);
            angle.z = DirectX::XMConvertToRadians(a.z);
            //スケール
            ImGui::InputFloat3("Scale", &scale.x);
            //speed
            ImGui::InputFloat3("velocity", &velocity.x);
            ImGui::SliderFloat("MoveSpeed", &moveSpeed, 0, 100);
            ImGui::SliderFloat("TurnSpeed", &turnSpeed, 0, 100);
            ImGui::InputFloat("Radius", &radius);
            ImGui::InputFloat("Height", &height);
            ImGui::InputFloat("Friction", &friction);
            ImGui::InputFloat("Acceleration", &acceleration);
            ImGui::InputFloat("AirControl", &airControl);
    }
    //ImGui::End();
}

//ノードとエネミーの衝突判定
void Player::CollisionNodeVsEnemies(const char* nodeName, float nodeRadius)
{
    //ノード取得
    DirectX::XMFLOAT3 nodePosition;
    Model::Node* n = model->FindNode(nodeName);
    nodePosition.x = n->worldTransform._41;
    nodePosition.y = n->worldTransform._42;
    nodePosition.z = n->worldTransform._43;


    //指定のノードと全ての敵を総当たりで衝突判定
    EnemyManager& enemyManager = EnemyManager::Instance();

    //全ての数と総当たりで衝突処理
    int enemyCount = enemyManager.GetEnemyCount();
    for (int i = 0; i < enemyCount; ++i)
    {
        Enemy* enemy = enemyManager.GetEnemy(i);

        //衝突処理
        DirectX::XMFLOAT3 outPosition;
        if (Collision::IntersectSphereVsCylider(   //円
            nodePosition, nodeRadius,
            enemy->GetPosition(), enemy->GetRadius(),
            enemy->GetHeight(),
            outPosition))
        {
            //ダメージを与える
            if (enemy->ApplyDamage(1, 0.5f))
            {
                //吹き飛ばす
                {
                    DirectX::XMFLOAT3 impulse;

                    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&nodePosition);
                    DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&enemy->GetPosition());
                    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);  //引き算で方向を求めるa->b
                    Vec = DirectX::XMVector3Normalize(Vec);
                    Vec = DirectX::XMVectorScale(Vec, 10.0f);   //球と敵の距離を吹き飛ばす
                    DirectX::XMStoreFloat3(&impulse, Vec);

                    impulse.y = 3.0f;//少しジャンプ
                    enemy->AddImpulse(impulse);
                }

                //ヒットエフェクト再生
                {
                    DirectX::XMFLOAT3 e = enemy->GetPosition();
                    e.y += enemy->GetHeight() * 0.5f;
                    hitEffect->Play(e);
                }
            }
        }

        //if (Collision::IntersectCylinder(
        //    this->GetPosition(), this->GetRadius(), this->GetHeight(),
        //    enemy->GetPosition(), enemy->GetRadius(), enemy->GetHeight(),
        //    outPosition
        //))
        //{
        //    DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&this->GetPosition());
        //    DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->GetPosition());
        //    DirectX::XMVECTOR EP = DirectX::XMVectorSubtract(P, E);
        //    DirectX::XMVECTOR N = DirectX::XMVector3Normalize(EP);

        //    //押し出し後の位置設定
        //    if (DirectX::XMVectorGetY(N) > 0.8f) Jump(10);
        //    /*if(IsGround())*/
        //    else enemy->SetPosition(outPosition);
        //}
    }

}

//アニメーション
//待機ステートへ遷移
void Player::TransitionIdleState()
{
    state = State::Idle;
    
    //待機アニメーション再生
    model->PlayAnimation(Anim_Idle, true);
}

//待機ステート更新処理
void Player::UpdateIdleState(float elapsedTime)
{
    //移動入力処理
    if (InputMove(elapsedTime))TransitionMoveState();

    //ジャンプ入力処理
    if (InputJump())TransitionJumpState();

    //弾丸入力処理
    InputProjectile();

    //攻撃入力処理
    if (InputAttack())TransitionAttackState();
}

//移動ステートへ遷移
void Player::TransitionMoveState()
{
    state = State::Move;

    //走りアニメーション再生
    model->PlayAnimation(Anim_Running, true);
}

//移動ステート更新処理
void Player::UpdateMoveState(float elapsedTime)
{
    //移動入力処理
    if (!InputMove(elapsedTime))TransitionIdleState();

    //ジャンプ入力処理
    if (InputJump())TransitionJumpState();

    //弾丸入力処理
    InputProjectile();

    //攻撃入力処理
    if (InputAttack())TransitionAttackState();
}

//ジャンプステートへ遷移
void Player::TransitionJumpState()
{
    state = State::Jump;

    //ジャンプアニメーション再生
    model->PlayAnimation(Anim_Jump, false);
}

//ジャンプステート更新処理
void Player::UpdateJumpState(float elapsedTime)
{
    //移動入力処理
    InputMove(elapsedTime);

    //ジャンプ入力処理
    InputJump();

    //弾丸入力処理
    InputProjectile();

    //アニメーション再生終了したらFallingにする
    if (!model->IsPlayAnimation())model->PlayAnimation(Anim_Falling, true);
}

//着地ステートへ遷移
void Player::TransitionLandState()
{
    state = State::Land;

    //ジャンプアニメーション再生
    model->PlayAnimation(Anim_Landing, false);
}

//着地ステート更新処理
void Player::UpdateLandState(float elapsedTime)
{
    //ジャンプ入力処理
    if (InputJump())TransitionJumpState();

    //弾丸入力処理
    InputProjectile();

    //アニメーション再生終了したらFallingにする
    if (!model->IsPlayAnimation())TransitionIdleState();
}

//攻撃ステートへ遷移
void Player::TransitionAttackState()
{
    state = State::Attack;

    //ジャンプアニメーション再生
    model->PlayAnimation(Anim_Attack, false);
}

//攻撃ステート更新処理
void Player::UpdateAttackState(float elapsedTime)
{
    //任意のアニメーション再生区間でのみ衝突判定処理をする
    float animationTime = model->GetCurrentAnimationSecoonds();
    attackCollisionFlag = animationTime > 0.31f;
    if (attackCollisionFlag)
    {
        //左手ノードとエネミーの衝突処理
        CollisionNodeVsEnemies("mixamorig:LeftHand", leftHandRadius);
    }

    //アニメーション再生終了したら待機にする
    if (!model->IsPlayAnimation())
    {
        TransitionIdleState();
        attackCollisionFlag = false;
    }
}

//ダメージステートへ遷移
void Player::TransitionDamageState()
{
    state = State::Damage;

    //ダメージアニメーション再生
    model->PlayAnimation(Anim_GetHit1, false);
}

//ダメージステート更新処理
void Player::UpdateDamageState(float elapsedTime)
{
    //アニメーション再生終了したら待機にする
    if (!model->IsPlayAnimation())
    {
        TransitionIdleState();
    }
}

//死亡ステートへ遷移
void Player::TransitionDeathState()
{
    state = State::Death;

    //死亡アニメーション再生
    model->PlayAnimation(Anim_Death, false);
}

//死亡ステート更新処理
void Player::UpdateDeathState(float elapsedTime)
{
    if (!model->IsPlayAnimation())
    {
        //ボタンを押したら復活
        GamePad& gamePad = Input::Instance().GetGamePad();
        if (gamePad.GetButtonDown() & GamePad::BTN_A)
        {
            TransitionReviveState();
        }
    }
}

//復活ステートへ遷移
void Player::TransitionReviveState()
{
    state = State::Revive;

    //体力回復
    health = maxHealth;

    //復活アニメーション再生
    model->PlayAnimation(Anim_Revive, false);
}

//復活ステート更新処理
void Player::UpdateReviveState(float elapsedTime)
{
    if (!model->IsPlayAnimation())
    {
        TransitionIdleState();
    }
}