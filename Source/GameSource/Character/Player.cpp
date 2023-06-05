//#include <imgui.h>
//#include "Player.h"
//#include "Input/Input.h"
//#include "Graphics/Graphics.h"
//#include "GameSource\Math\Collision.h"
//
//static Player* instance = nullptr;
//
////インスタンス取得
//Player& Player::Instance()
//{
//    return *instance;
//}
//
////コンストラクタ
//Player::Player(CameraController* camera)
//    :cameraController(camera)
//{
//    //インスタンスポインタ設定
//    instance = this;
//
//    model = new Model("Data/Model/pico/picoAnim.mdl");
//    model->PlayAnimation(0,true);
//
//    //モデルが大きいのでスケーリング
//    scale.x = scale.y = scale.z = 0.02f;
//
//    //ヒットエフェクト読み込み
//    hitEffect = new Effect("Data/Effect/Hit.efk");
//
//    w = std::make_unique<Weapon>("Data/Model/Swords/BigSword.mdl", DirectX::XMFLOAT3{ 0.01f,0.01f,0.01f });
//    w->SetAngle({ -90,0,0 });
//
//    //待機ステートへ遷移
//    TransitionIdleState();
//}
//
////デストラクタ
//Player::~Player()
//{
//    delete hitEffect;
//    delete model;
//}
//
//
////スティック入力値
//DirectX::XMFLOAT3 Player::GetMoveVec() const
//{
//    //入力情報を取得
//    GamePad& gamePad = Input::Instance().GetGamePad();
//    float ax = gamePad.GetAxisLX();
//    float ay = gamePad.GetAxisLY();
//
//    //カメラ方向とスティックの入力値によって進行方向を計算する
//    std::shared_ptr<CameraCom> camera = cameraController->GetCamera();
//    const DirectX::XMFLOAT3& cameraRight = camera->GetRight();
//    const DirectX::XMFLOAT3& cameraFront = camera->GetFront();
//
//    //移動ベクトルはXZ平面に水平なベクトルになるようにする
//
//    //カメラ右方向ベクトルをXZ単位ベクトルに変換
//    float cameraRightX = cameraRight.x;
//    float cameraRightZ = cameraRight.z;
//    float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
//    if (cameraRightLength > 0.0f)
//    {
//        //単位ベクトル化
//        cameraRightX /= cameraRightLength;
//        cameraRightZ /= cameraRightLength;
//    }
//
//    //カメラ前方向ベクトルをXZ単位ベクトルに変換
//    float cameraFrontX = cameraFront.x;
//    float cameraFrontZ = cameraFront.z;
//    float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
//    if (cameraFrontLength > 0.0f)
//    {
//        //単位ベクトル化
//        cameraFrontX /= cameraFrontLength;
//        cameraFrontZ /= cameraFrontLength;
//    }
//
//    //スティックの水平入力値をカメラ右方向に反映し、
//    //スティックの垂直入力値をカメラ前方向に反映し、
//    //進行ベクトルを計算する
//    DirectX::XMFLOAT3 vec;
//    vec.x = cameraFrontX * ay + cameraRightX * ax;
//    vec.z = cameraFrontZ * ay + cameraRightZ * ax;
//    //Y軸方向には移動しない
//    vec.y = 0.0f;
//
//    return vec;
//}
//
////更新処理
//void Player::Update(float elapsedTime)
//{
//    //ステート毎の処理
//    switch (state)
//    {
//    case State::Idle:
//        UpdateIdleState(elapsedTime);
//        break;
//
//    case State::Move:
//        UpdateMoveState(elapsedTime);
//        break;
//
//    case State::Jump:
//        UpdateJumpState(elapsedTime);
//        break;
//
//    case State::Land:
//        UpdateLandState(elapsedTime);
//        break;
//
//    case State::Attack:
//        UpdateAttackState(elapsedTime);
//        break;
//
//    case State::Damage:
//        UpdateDamageState(elapsedTime);
//        break;
//
//    case State::Death:
//        UpdateDeathState(elapsedTime);
//        break;
//
//    case State::Revive:
//        UpdateReviveState(elapsedTime);
//        break;
//
//    }
//
//    //速力処理更新
//    UpdateVelocity(elapsedTime);
//
//    //無敵時間更新
//    UpdateInvincibleTimer(elapsedTime);
//
//    //オブジェクトの行列を更新
//    UpdateTransform();
//
//    //モデルアニメーション更新処理
//    model->UpdateAnimation(elapsedTime);
//
//    //モデル行列更新
//    model->UpdateTransform(transform);  //位置行列を渡す
//
//
//    Model::Node* rightHandBone = model->FindNode("RightHand");
//    w->ParentTransform(rightHandBone->worldTransform);
//    w->Update(elapsedTime);
//}
//
//
//bool Player::InputMove(float elapsedTime)
//{
//    //進行ベクトル取得
//    DirectX::XMFLOAT3 moveVec = GetMoveVec();
//
//    //移動処理
//    Move(moveVec.x, moveVec.z, moveSpeed);
//
//    //旋回処理
//    Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);
//
//    //進行ベクトルがゼロベクトルでない場合は入力された
//    return moveVec.x * moveVec.x + moveVec.y * moveVec.y + moveVec.z * moveVec.z;
//}
//
////ジャンプ入力処理
//bool Player::InputJump()
//{
//    GamePad& gamePad = Input::Instance().GetGamePad();
//    if (gamePad.GetButtonDown() & GamePad::BTN_A) 
//    {
//        if (JumpCount < JumpLimit)  //ジャンプ回数制限
//        {
//            Jump(jumpSpeed);
//            ++JumpCount;
//
//            //ジャンプ入力した
//            return true;
//        }
//    }
//    return false;
//}
//
////攻撃入力処理
//bool Player::InputAttack()
//{
//    GamePad& gamePad = Input::Instance().GetGamePad();
//
//    if (gamePad.GetButtonDown() & GamePad::BTN_B)
//    {
//        return true;
//    }
//    return false;
//}
//
////着地した時に呼ばれる
//void Player::OnLanding()
//{
//    JumpCount = 0;
//
//    //ダメージ、死亡ステート時は着地した時にステート遷移しないようにする
//    if (state != State::Damage && state != State::Death)
//    {
//        //着地ステートへ遷移
//        TransitionLandState();
//    }
//}
//
////ダメージを受けた時に呼ばれる
//void Player::OnDamage()
//{
//    //ダメージステートへ遷移
//    TransitionDamageState();
//}
//
////死亡した時に呼ばれる
//void Player::OnDead()
//{
//    //死亡した時に呼ばれる
//    TransitionDeathState();
//}
//
////描画更新
//void Player::Render(ID3D11DeviceContext* dc, Shader* shader)
//{
//    shader->Draw(dc, model);
//    w->Render(dc, shader);
//}
//
////デバッグプリミティブ描画
//void Player::DrawDebugPrimitive()
//{
//    if (!VisibleDebugPrimitive)return;
//    DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
//
//    //衝突判定用のデバッグ円柱を描画
//    debugRenderer->DrawCylinder(position, radius, height, DirectX::XMFLOAT4(0, 0, 0, 1));  
//
//    //攻撃衝突用の左手ノードのデバッグ球を描画
//    if (attackCollisionFlag)
//    {
//        Model::Node* leftHandBone = model->FindNode("mixamorig:LeftHand");
//        debugRenderer->DrawSphere(DirectX::XMFLOAT3(
//            leftHandBone->worldTransform._41,
//            leftHandBone->worldTransform._42,
//            leftHandBone->worldTransform._43),
//            leftHandRadius,
//            DirectX::XMFLOAT4(1, 0, 0, 1));
//    }
//}
//
////デバッグ用GUI描画
//void Player::DrawDebugGUI()
//{
//    if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
//    {
//        ImGui::Checkbox("VisibleDebugPrimitive", &VisibleDebugPrimitive);
//            //位置
//            ImGui::InputFloat3("Position", &position.x);
//            //回転
//            DirectX::XMFLOAT3 a;
//            a.x = DirectX::XMConvertToDegrees(angle.x);
//            a.y = DirectX::XMConvertToDegrees(angle.y);
//            a.z = DirectX::XMConvertToDegrees(angle.z);
//            ImGui::InputFloat3("Angle", &a.x);
//            angle.x = DirectX::XMConvertToRadians(a.x);
//            angle.y = DirectX::XMConvertToRadians(a.y);
//            angle.z = DirectX::XMConvertToRadians(a.z);
//            //スケール
//            ImGui::InputFloat3("Scale", &scale.x);
//            //speed
//            ImGui::InputFloat3("velocity", &velocity.x);
//            ImGui::SliderFloat("MoveSpeed", &moveSpeed, 0, 100);
//            ImGui::SliderFloat("TurnSpeed", &turnSpeed, 0, 100);
//            ImGui::InputFloat("Radius", &radius);
//            ImGui::InputFloat("Height", &height);
//            ImGui::InputFloat("Friction", &friction);
//            ImGui::InputFloat("Acceleration", &acceleration);
//            ImGui::InputFloat("AirControl", &airControl);
//    }
//}
//
////アニメーション
////待機ステートへ遷移
//void Player::TransitionIdleState()
//{
//    state = State::Idle;
//    
//    //待機アニメーション再生
//    model->PlayAnimation(Anim_spin, true);
//}
//
////待機ステート更新処理
//void Player::UpdateIdleState(float elapsedTime)
//{
//    //移動入力処理
//    if (InputMove(elapsedTime))TransitionMoveState();
//
//    //ジャンプ入力処理
//    if (InputJump())TransitionJumpState();
//
//    //攻撃入力処理
//    if (InputAttack())TransitionAttackState();
//}
//
////移動ステートへ遷移
//void Player::TransitionMoveState()
//{
//    state = State::Move;
//
//    //走りアニメーション再生
//    model->PlayAnimation(Anim_happyWalk, true);
//}
//
////移動ステート更新処理
//void Player::UpdateMoveState(float elapsedTime)
//{
//    //移動入力処理
//    if (!InputMove(elapsedTime))TransitionIdleState();
//
//    //ジャンプ入力処理
//    if (InputJump())TransitionJumpState();
//
//    //攻撃入力処理
//    if (InputAttack())TransitionAttackState();
//}
//
////ジャンプステートへ遷移
//void Player::TransitionJumpState()
//{
//    state = State::Jump;
//
//    //ジャンプアニメーション再生
//    model->PlayAnimation(Anim_ninjaWalk, false);
//}
//
////ジャンプステート更新処理
//void Player::UpdateJumpState(float elapsedTime)
//{
//    //移動入力処理
//    InputMove(elapsedTime);
//
//    //ジャンプ入力処理
//    InputJump();
//
//    //アニメーション再生終了したらFallingにする
//    if (!model->IsPlayAnimation())model->PlayAnimation(Anim_happyWalk, true);
//}
//
////着地ステートへ遷移
//void Player::TransitionLandState()
//{
//    state = State::Land;
//
//    //ジャンプアニメーション再生
//    model->PlayAnimation(Anim_happyWalk, false);
//}
//
////着地ステート更新処理
//void Player::UpdateLandState(float elapsedTime)
//{
//    //ジャンプ入力処理
//    if (InputJump())TransitionJumpState();
//
//    //アニメーション再生終了したらFallingにする
//    if (!model->IsPlayAnimation())TransitionIdleState();
//}
//
////攻撃ステートへ遷移
//void Player::TransitionAttackState()
//{
//    state = State::Attack;
//
//    //ジャンプアニメーション再生
//    model->PlayAnimation(Anim_happyWalk, false);
//}
//
////攻撃ステート更新処理
//void Player::UpdateAttackState(float elapsedTime)
//{
//    //任意のアニメーション再生区間でのみ衝突判定処理をする
//    float animationTime = model->GetCurrentAnimationSecoonds();
//    attackCollisionFlag = animationTime > 0.31f;
//
//    //アニメーション再生終了したら待機にする
//    if (!model->IsPlayAnimation())
//    {
//        TransitionIdleState();
//        attackCollisionFlag = false;
//    }
//}
//
////ダメージステートへ遷移
//void Player::TransitionDamageState()
//{
//    state = State::Damage;
//
//    //ダメージアニメーション再生
//    model->PlayAnimation(Anim_happyWalk, false);
//}
//
////ダメージステート更新処理
//void Player::UpdateDamageState(float elapsedTime)
//{
//    //アニメーション再生終了したら待機にする
//    if (!model->IsPlayAnimation())
//    {
//        TransitionIdleState();
//    }
//}
//
////死亡ステートへ遷移
//void Player::TransitionDeathState()
//{
//    state = State::Death;
//
//    //死亡アニメーション再生
//    model->PlayAnimation(Anim_happyWalk, false);
//}
//
////死亡ステート更新処理
//void Player::UpdateDeathState(float elapsedTime)
//{
//    if (!model->IsPlayAnimation())
//    {
//        //ボタンを押したら復活
//        GamePad& gamePad = Input::Instance().GetGamePad();
//        if (gamePad.GetButtonDown() & GamePad::BTN_A)
//        {
//            TransitionReviveState();
//        }
//    }
//}
//
////復活ステートへ遷移
//void Player::TransitionReviveState()
//{
//    state = State::Revive;
//
//    //体力回復
//    health = maxHealth;
//
//    //復活アニメーション再生
//    model->PlayAnimation(Anim_happyWalk, false);
//}
//
////復活ステート更新処理
//void Player::UpdateReviveState(float elapsedTime)
//{
//    if (!model->IsPlayAnimation())
//    {
//        TransitionIdleState();
//    }
//}