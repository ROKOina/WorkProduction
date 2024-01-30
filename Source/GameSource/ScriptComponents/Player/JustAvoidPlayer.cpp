#include "JustAvoidPlayer.h"

#include "PlayerCom.h"
#include "PlayerCameraCom.h"
#include "PlayerWeapon/CandyPushCom.h"
#include "../Weapon/WeaponCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "Components\TransformCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\CameraCom.h"
#include "Components\ParticleSystemCom.h"
#include "Components\ParticleComManager.h"
#include "../CharacterStatusCom.h"
#include "../Score.h"

#include "../Enemy/EnemyManager.h"
#include "../Enemy/EnemyCom.h"

#include "Graphics/Sprite/Sprite.h"

#include "Input/Input.h"
#include <imgui.h>

JustAvoidPlayer::JustAvoidPlayer(std::shared_ptr<PlayerCom> player)
    :player_(player)
{
    justSprite_ = std::make_unique<Sprite>("./Data/Sprite/justBlur.png");
    justSprite_->SetDissolveSRV("./Data/Sprite/justBlurMask.png");
    Sprite::DissolveConstans& dissolveData = justSprite_->GetEffectSpriteData().dissolveConstant;

    dissolveData.isDissolve = true;
    dissolveData.dissolveThreshold = 0;
    dissolveData.edgeThreshold = 1;
    dissolveData.edgeColor = { 1,0,1,1 };
}

JustAvoidPlayer::~JustAvoidPlayer()
{
}

void JustAvoidPlayer::Update(float elapsedTime)
{
    //ジャスト回避判定時
    if (isJustJudge_)
    {
        //ジャスト回避移動処理
        JustAvoidanceMove(elapsedTime);

        //ジャスト回避後反撃入力確認
        JustAvoidanceAttackInput();
    }

    //ジャスト回避演出
    JustAvoidDirectionEnd(elapsedTime);

    //ジャスト回避世界色演出
    JustDirectionUpdate(elapsedTime);

    //反撃処理更新
    switch (justAvoidKey_)
    {
        //□反撃
    case JUST_AVOID_KEY::SQUARE:
        JustAvoidanceSquare(elapsedTime);
        break;
        //△反撃
    case JUST_AVOID_KEY::TRIANGLE:
        JustAvoidanceTriangle(elapsedTime);
        break;
    }
}

void JustAvoidPlayer::OnGui()
{
    Sprite::DissolveConstans& dissolveData = justSprite_->GetEffectSpriteData().dissolveConstant;
    
    bool isD = false;
    if(dissolveData.isDissolve>0.1f)isD = true;
    if (ImGui::Checkbox("isDissolve", &isD))
    {
        if (isD)dissolveData.isDissolve = 1;
        else dissolveData.isDissolve = 0;
    }

    ImGui::DragFloat("dissolveThreshold", &dissolveData.dissolveThreshold, 0.01f, 0, 1);
    ImGui::DragFloat("edgeThreshold", &dissolveData.edgeThreshold, 0.01f, 0, 1);
    ImGui::DragFloat4("edgeColor", &dissolveData.edgeColor.x, 0.01f, 0, 1);
}

void JustAvoidPlayer::Render2D(float elapsedTime)
{
    ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();

    //ロックオンがノーマルの時
    if (player_.lock()->GetLockOn() == PlayerCom::LOCK_TARGET::JUST_LOCK)
    {
        //ロックオン画像出す
        Sprite* lockOnSprite = player_.lock()->GetLockOnSprite();

        //ロックオンする敵を探す
        std::shared_ptr<GameObject> enemy = lockTriangleEnemy_.lock();

        if (enemy)
        {
            DirectX::XMFLOAT3 enemyPos = enemy->transform_->GetWorldPosition();
            if (std::strcmp(enemy->GetName(), "appleNear") == 0)
                enemyPos.y += 0.5f;
            else
                enemyPos.y += 1;
            std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
            enemyPos = Graphics::Instance().WorldToScreenPos(enemyPos, camera);

            float size = 150;
            lockOnSprite->Render(dc, enemyPos.x - size / 2.0f, enemyPos.y - size / 2.0f
                , size, size
                , 0, 0, static_cast<float>(lockOnSprite->GetTextureWidth()), static_cast<float>(lockOnSprite->GetTextureHeight())
                , 0, 2, 2, 0, 1);
        }
    }
}

void JustAvoidPlayer::AudioRelease()
{
    justSE_->AudioRelease();
    dashSE_->AudioRelease();
    triangleJustAttackSE_->AudioRelease();
    triangleCursorSE_->AudioRelease();
    squareSlowSE_->AudioRelease();
    squareSlowInOutSE_->AudioRelease();
}


//ジャスト回避初期化
void JustAvoidPlayer::JustInisialize()
{
    justAvoidLeadKey_ = JUST_AVOID_KEY::NULL_KEY;
    justAvoidState_ = -1;
    isJustJudge_ = false;
    Graphics::Instance().SetWorldSpeed(1);
    //カラーグレーディング戻す
    Graphics::Instance().shaderParameter3D_.colorGradingData.saturation = 1.5f;
    //シルエット有効に
    std::shared_ptr<RendererCom> renderCom = player_.lock()->GetGameObject()->GetComponent<RendererCom>();
    renderCom->SetSilhouetteFlag(true);

    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();
    move->SetGravity(GRAVITY_NORMAL);

    for (int i = 0; i < 4; ++i)
    {
        std::string s = "picoJust" + std::to_string(i);
        player_.lock()->GetGameObject()->GetChildFind(s.c_str())->SetEnabled(false);
    }
}

//ジャスト回避移動処理
void JustAvoidPlayer::JustAvoidanceMove(float elapsedTime)
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    //演出取得
    std::shared_ptr<GameObject> justPico[4];
    for (int i = 0; i < 4; ++i)
    {
        std::string s = "picoJust" + std::to_string(i);
        justPico[i] = player_.lock()->GetGameObject()->GetChildFind(s.c_str());
    }

    switch (justAvoidState_)
    {
    case 0:
    {
        //ジャスト回避に変更

        player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::JUSTDASH);
        justAvoidTimer_ = justAvoidTime_;

        player_.lock()->GetMovePlayer()->SetIsInputMove(false);
        player_.lock()->GetAttackPlayer()->SetIsNormalAttack(false);
        player_.lock()->GetMovePlayer()->SetIsDash(false);

        //入力方向をみてアニメーション再生
        DirectX::XMVECTOR Input = DirectX::XMLoadFloat3(&player_.lock()->GetMovePlayer()->GetInputMoveVec());

        //アニメーション再生
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

        //animator->SetAnimationSpeedOffset(0.3f);

        std::shared_ptr<RendererCom> renderCom = player_.lock()->GetGameObject()->GetComponent<RendererCom>();
        //プレイヤー透明に
        renderCom->GetModel()->SetMaterialColor({ 1,1,1,0 });
        //シルエット切る
        renderCom->SetSilhouetteFlag(false);

        //世界色変化演出
        justSpriteState_ = 0;

        //入力によりアニメーションを変える
        bool inputFlag = false;
        if (DirectX::XMVectorGetX(DirectX::XMVector3Length(Input)) > 0.1f)
        {
            animator->SetTriggerOn("justFront");
            inputFlag = true;
        }
        else
        {
            animator->SetTriggerOn("justBack");
        }

        //分身出現
        for (int i = 0; i < 4; ++i)
        {
            std::shared_ptr<AnimationCom> justAnim = justPico[i]->GetComponent<AnimationCom>();
            //初期化
            justPico[i]->transform_->SetLocalPosition({ 0,0,0 });
            Model* justModel = justPico[i]->GetComponent<RendererCom>()->GetModel();
            justModel->SetMaterialColor({ 0.4f,0.3f,0.1f,0.65f });  //色初期化

            //出す
            justPico[i]->SetEnabled(true);
            if (inputFlag)
                justAnim->PlayAnimation(ANIMATION_PLAYER::DODGE_FRONT, false);
            else
                justAnim->PlayAnimation(ANIMATION_PLAYER::DODGE_BACK, false);
        }

        //空中回避の場合は重力0に
        onGroundAvoid_ = true;
        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP_DASH
            || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP_BACK_DASH)
        {
            move->SetGravity(GRAVITY_ZERO);
            move->ZeroVelocityY();
            onGroundAvoid_ = false;
        }

        //敵の方を向く
        if (!justHitEnemy_.expired())
        {
            DirectX::XMFLOAT3 enemyPos = justHitEnemy_.lock()->transform_->GetWorldPosition();
            enemyPos.y = player_.lock()->GetGameObject()->transform_->GetWorldPosition().y;
            player_.lock()->GetGameObject()->transform_->LookAtTransform(enemyPos);
        }

        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUST);


        //カラーグレーディング
        Graphics::Instance().shaderParameter3D_.colorGradingData.saturation = 0.4f;

        ////カメラシェイク
        //GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()->CameraShake(0.1f, 0.25f);

        //ヒットストップ
        GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()
            ->HitStop(0.2f);
        hitStopEnd_ = false;

        //カメラを近づける
        player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>()->SetChangeRange(0.5f, NearRange);

        justAvoidState_++;
    }
    break;
    //分身移動処理＆アニメスピード戻す
    case 1:
    {
        //ヒットストップ後スロー
        if (!GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()->GetIsHitStop() && !hitStopEnd_)
        {
            hitStopEnd_ = true;
            Graphics::Instance().SetWorldSpeed(0.5f);
        }

        //ステートエンドフラグ
        bool endFlag = false;


        //演出用ピコポジション
        DirectX::XMFLOAT3 justPicoPos[4];
        for (int i = 0; i < 4; ++i)
        {
            justPicoPos[i] = justPico[i]->transform_->GetLocalPosition();

            //アニメスピードをプレイヤーに合わせる
            std::shared_ptr<AnimationCom> justAnim = justPico[i]->GetComponent<AnimationCom>();
            std::shared_ptr<AnimationCom> picoAnim = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
            justAnim->SetAnimationSpeed(picoAnim->GetAnimationSpeed());
        }
        float speed = 250;
        float moveRange = 100;
        //縦位置更新
        for (int f = 0; f < 2; ++f)
        {
            float g = 1;
            if (f == 1)g = -1;
            if (justPicoPos[f].z * justPicoPos[f].z < moveRange * moveRange)
            {
                justPicoPos[f].z += speed * elapsedTime * g;
                justPico[f]->transform_->SetLocalPosition(justPicoPos[f]);
            }
        }
        //横位置更新
        for (int r = 0; r < 2; ++r)
        {
            float g = 1;
            if (r == 1)g = -1;
            if (justPicoPos[r + 2].x * justPicoPos[r + 2].x < moveRange * moveRange)
            {
                justPicoPos[r + 2].x += speed * elapsedTime * g;
                justPico[r + 2]->transform_->SetLocalPosition(justPicoPos[r + 2]);
            }
            else
                endFlag = true;
        }

        //カラーグレーディング戻す
        Graphics::Instance().shaderParameter3D_.colorGradingData.saturation += elapsedTime;

        //ステート終わり
        if (endFlag)
        {
            //プレイヤー少し光らせる
            player_.lock()->GetGameObject()->GetComponent<RendererCom>()->
                GetModel()->SetMaterialColor({ 1,2.0f,1,1.5f });
            playerDirection_ = true;    //戻す処理をJustAvoidDirection関数でする

            //カメラを引く
            player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>()->SetChangeRange(1.0f, FarRange);

            justAvoidState_++;
        }

    }
    break;
    //分身消す
    //プレイヤー出す
    case 2:
    {
        //ステートエンドフラグ
        bool endFlag = false;

        //徐々に透明に
        for (int i = 0; i < 4; ++i)
        {
            Model* justModel = justPico[i]->GetComponent<RendererCom>()->GetModel();
            DirectX::XMFLOAT4 color = justModel->GetMaterialColor();
            color.w -= elapsedTime;
            justModel->SetMaterialColor(color);
            if (color.w < 0.4f) //透明になったら消す（0.5f以下でほぼ透明なので0.4fで透明判定）
            {
                justPico[i]->SetEnabled(false);
                endFlag = true;
            }

            //アニメスピードをプレイヤーに合わせる
            std::shared_ptr<AnimationCom> justAnim = justPico[i]->GetComponent<AnimationCom>();
            std::shared_ptr<AnimationCom> picoAnim = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
            justAnim->SetAnimationSpeed(picoAnim->GetAnimationSpeed());  
        }

        //カラーグレーディング戻す
        Graphics::Instance().shaderParameter3D_.colorGradingData.saturation += elapsedTime;

        if (endFlag)
            justAvoidState_++;
    }
    break;
    //反撃受け入れ
    case 3:
    {
        //アニメ終了で反撃終了
        if (!player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->IsPlayAnimation())
        {
            //プレイヤーアニメスピード戻す
            std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
            animator->SetAnimationSpeedOffset(1.0f);

            //世界の色戻す
            justSpriteState_ = 20;

            justAvoidTimer_ = -1;
        }

        //スロー和らげる
        float slow = Graphics::Instance().GetWorldSpeed();
        slow += 0.5f * elapsedTime;
        Graphics::Instance().SetWorldSpeed(slow);

        //カラーグレーディング戻す
        Graphics::Instance().shaderParameter3D_.colorGradingData.saturation += elapsedTime;

        //ジャスト回避終了タイマー
        justAvoidTimer_ -= elapsedTime;
        if (justAvoidTimer_ < 0)
        {
            JustInisialize();

            //世界の色戻す
            justSpriteState_ = 20;
            player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::RUN);
            player_.lock()->GetMovePlayer()->SetIsInputMove(true);
            player_.lock()->GetAttackPlayer()->SetIsNormalAttack(true);
            player_.lock()->GetMovePlayer()->SetIsDash(true);
            break;
        }

    }
    break;
    }

    //カラーグレーディング制限
    if (Graphics::Instance().shaderParameter3D_.colorGradingData.saturation > 1.5f)
    {
        Graphics::Instance().shaderParameter3D_.colorGradingData.saturation = 1.5f;
    }

    //移動出来るか
    int animIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();
    if (animIndex != ANIMATION_PLAYER::DODGE_FRONT && animIndex != ANIMATION_PLAYER::DODGE_BACK)
    {
        //移動方向を決める（ジャスト回避中は勝手に移動する）
        DirectX::XMFLOAT3 Direction;

        DirectX::XMFLOAT3 inputVec = player_.lock()->GetMovePlayer()->GetInputMoveVec();
        if (inputVec.x * inputVec.x + inputVec.z * inputVec.z > 0.1f)
        {
            Direction = { inputVec.x ,0,inputVec.z };
        }
        else
        {
            Direction = player_.lock()->GetGameObject()->transform_->GetWorldFront();
            //yを消して逆向きにする
            Direction.x *= -1;
            Direction.y = 0;
            Direction.z *= -1;
            //正規化
            DirectX::XMStoreFloat3(&Direction, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&Direction)));
        }

        player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::JUSTDASH);
        //力に加える
        move->AddForce(Direction);
    }
}

//ジャスト回避反撃入力確認
void JustAvoidPlayer::JustAvoidanceAttackInput()
{
    //ボタンで反撃変える
    GamePad& gamePad = Input::Instance().GetGamePad();

    if (justAvoidState_ < 3)
    {
        //先行入力
        if (gamePad.GetButtonDown() & GamePad::BTN_X)
            justAvoidLeadKey_ = JUST_AVOID_KEY::SQUARE;
        if (gamePad.GetButtonDown() & GamePad::BTN_Y)
            justAvoidLeadKey_ = JUST_AVOID_KEY::TRIANGLE;

        return;
    }

    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();


    //□の場合
    if ((gamePad.GetButtonDown() & GamePad::BTN_X) || (justAvoidLeadKey_ == JUST_AVOID_KEY::SQUARE))
    {
        JustInisialize();
        player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::DASH);

        justAvoidKey_ = JUST_AVOID_KEY::SQUARE;

        player_.lock()->GetMovePlayer()->SetIsInputMove(false);

        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);

        //敵の方を向く
        DirectX::XMFLOAT3 enemyPos = justHitEnemy_.lock()->transform_->GetWorldPosition();
        enemyPos.y = player_.lock()->GetGameObject()->transform_->GetWorldPosition().y;
        player_.lock()->GetGameObject()->transform_->LookAtTransform(enemyPos);

        //敵をスローにする
        EnemyManager::Instance().SetEnemySpeed(0.1f, 5.0f);

        //SE
        squareSlowInOutSE_->Play(false,3.0f);
        squareSlowSE_->Play(true, 3.0f);

        //世界色演出
        justSpriteState_ = 10;
        //パーティクル演出起動
        SetJustUnderParticle(true);
    }
    //△の場合
    else if ((gamePad.GetButtonDown() & GamePad::BTN_Y) || (justAvoidLeadKey_ == JUST_AVOID_KEY::TRIANGLE))
    {
        JustInisialize();
        triangleState_ = 0;
        justAvoidKey_ = JUST_AVOID_KEY::TRIANGLE;

        //世界色演出
        justSpriteState_ = 20;
    }
}

//□反撃
void JustAvoidPlayer::JustAvoidanceSquare(float elapsedTime)
{
    //敵に接近する
    {
        std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

        //空中の場合
        if (!onGroundAvoid_)
        {
            move->SetGravity(GRAVITY_ZERO);
        }

        DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&player_.lock()->GetGameObject()->transform_->GetWorldPosition());
        DirectX::XMVECTOR EnemyPos = Pos;
        if (!justHitEnemy_.expired())
            EnemyPos = DirectX::XMLoadFloat3(&justHitEnemy_.lock()->transform_->GetWorldPosition());
        DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(EnemyPos, Pos);
        float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
        //敵の近くまで移動する
        if (length < 1.5f || length > 5)
        {
            JustInisialize();
            player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::RUN);
            justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
            player_.lock()->GetMovePlayer()->SetIsInputMove(true);
            player_.lock()->GetAttackPlayer()->SetIsNormalAttack(true);
            player_.lock()->GetMovePlayer()->SetIsDash(true);

            //アタック処理に引き継ぐ
            if (onGroundAvoid_)
                player_.lock()->GetAttackPlayer()->SetAnimFlagName("squareJust");
            else
            {
                player_.lock()->GetAttackPlayer()->SetAnimFlagName("squareJustSky");
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);
                player_.lock()->GetMovePlayer()->SetJumpDashCount(player_.lock()->GetMovePlayer()->GetJumpDashCount() + 1);
                player_.lock()->GetMovePlayer()->SetJumpCount(1);
            }
        }

        DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(PE);
        Dir = DirectX::XMVectorScale(Dir, player_.lock()->GetMovePlayer()->GetMoveParam(MovePlayer::MOVE_PARAM::DASH).moveSpeed);
        DirectX::XMFLOAT3 dir;
        DirectX::XMStoreFloat3(&dir, Dir);

        //力に加える
        move->AddForce(dir);

    }
}

//△反撃
void JustAvoidPlayer::JustAvoidanceTriangle(float elapsedTime)
{
    //無敵時間
    player_.lock()->GetGameObject()->GetComponent<CharacterStatusCom>()
        ->SetInvincibleNonDamage(0.5f);

    //指定のカメラポスを返す
    auto GetCameraPos = [&]()
    {
        DirectX::XMFLOAT3 playerPos = player_.lock()->GetGameObject()->transform_->GetWorldPosition();

        DirectX::XMFLOAT3 cameraPos;
        DirectX::XMVECTOR f = DirectX::XMLoadFloat3(&player_.lock()->GetGameObject()->transform_->GetWorldFront());
        DirectX::XMVECTOR r = DirectX::XMLoadFloat3(&player_.lock()->GetGameObject()->transform_->GetWorldRight());
        DirectX::XMVECTOR u = DirectX::XMLoadFloat3(&player_.lock()->GetGameObject()->transform_->GetWorldUp());

        DirectX::XMStoreFloat3(&cameraPos,
            DirectX::XMVectorAdd(
                DirectX::XMLoadFloat3(&playerPos), DirectX::XMVectorAdd(
                    DirectX::XMVectorScale(r, 0.2f),
                    DirectX::XMVectorAdd(
                        DirectX::XMVectorScale(u, 1.1f),
                        DirectX::XMVectorScale(f, -0.5f)
                    )
                )));

        return cameraPos;
    };

    //カメラポスからプレイヤーポスを逆算
    auto GetCameraFromPlayerPos = [&]()
    {
        std::shared_ptr<GameObject> camera = GameObjectManager::Instance().Find("Camera");
        DirectX::XMFLOAT3 cameraPos = camera->transform_->GetWorldPosition();

        DirectX::XMFLOAT3 playerPos;
        DirectX::XMVECTOR f = DirectX::XMLoadFloat3(&camera->transform_->GetWorldFront());
        DirectX::XMVECTOR r = DirectX::XMLoadFloat3(&camera->transform_->GetWorldRight());
        DirectX::XMVECTOR u = DirectX::XMLoadFloat3(&camera->transform_->GetWorldUp());

        DirectX::XMStoreFloat3(&playerPos,
            DirectX::XMVectorAdd(
                DirectX::XMLoadFloat3(&cameraPos), DirectX::XMVectorAdd(
                    DirectX::XMVectorScale(r, -0.2f),
                    DirectX::XMVectorAdd(
                        DirectX::XMVectorScale(u, -1.1f),
                        DirectX::XMVectorScale(f, 0.5f)
                    )
                )));

        return playerPos;
    };

    static float pushSeconds = 0;   //アニメーションイベントまでの時間
    static float lockEnemySeconds = 0;  //ロックオン変更時に使う
    static DirectX::XMFLOAT3 focusEnemy{0, 0, 0};
    switch (triangleState_)
    {
        //アニメーション再生
    case 0:
    {
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("triangleJust");

        lockTriangleEnemy_ = justHitEnemy_;
        pushSeconds = 0.0f;

        //ロックオンターゲット変える
        player_.lock()->SetLockOn(PlayerCom::LOCK_TARGET::JUST_LOCK);

        dashSE_->Play(false);

        triangleState_++;
    }
    break;
    //時を止める、カメラ動かす
    case 1:
    {
        //アニメーションイベントまでの時間の割合
        float animEventStartTime = 0.4f;
        pushSeconds += elapsedTime;
        if (pushSeconds > animEventStartTime)pushSeconds = animEventStartTime;
        float startRatio = pushSeconds / animEventStartTime;

        focusEnemy = lockTriangleEnemy_.lock()->transform_->GetWorldPosition();

        //カメラ変更
        {
            //カメラコントローラー取得
            std::shared_ptr<PlayerCameraCom> playerCameraCom = player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>();
            playerCameraCom->SetIsJust(true);

            //補間してカメラ移動
            DirectX::XMVECTOR CameraPos = DirectX::XMLoadFloat3(&GameObjectManager::Instance().Find("Camera")->transform_->GetWorldPosition());
            DirectX::XMFLOAT3 cameraPos;
            DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorLerp(CameraPos, DirectX::XMLoadFloat3(&GetCameraPos()), startRatio));
            playerCameraCom->SetJustPos(cameraPos);

            //補間してカメラフォーカスポス移動
            DirectX::XMVECTOR FocusPos = DirectX::XMLoadFloat3(&playerCameraCom->GetForcusPos());
            DirectX::XMFLOAT3 focus;
            DirectX::XMStoreFloat3(&focus, DirectX::XMVectorLerp(FocusPos, DirectX::XMLoadFloat3(&focusEnemy), startRatio));
            playerCameraCom->SetForcusPos(focus);

            //補間してプレイヤーポス移動
            DirectX::XMFLOAT3 pPos = player_.lock()->GetGameObject()->transform_->GetWorldPosition();
            DirectX::XMVECTOR PPos = DirectX::XMLoadFloat3(&pPos);
            pPos.y = 5;
            DirectX::XMStoreFloat3(&pPos, DirectX::XMVectorLerp(PPos, DirectX::XMLoadFloat3(&pPos), startRatio));
            player_.lock()->GetGameObject()->transform_->SetWorldPosition(pPos);

            //カメラの向きにプレイヤー向かせる
            QuaternionStruct cameraQuaternion = GameObjectManager::Instance().Find("Camera")->transform_->GetRotation();

            //カメラの向きにプレイヤーを合わせる
            static DirectX::XMFLOAT4 rota = cameraQuaternion.dxFloat4;
            DirectX::XMVECTOR CA = DirectX::XMLoadFloat4(&cameraQuaternion.dxFloat4);
            DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&rota), CA, elapsedTime * 5));

            player_.lock()->GetGameObject()->transform_->SetRotation(rota);

        }

        //アニメーションイベントを見て次のステートへ
        std::shared_ptr<AnimationCom> picoAnim = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
        if (picoAnim->GetCurrentAnimationEvent("trianglePushStop", DirectX::XMFLOAT3(0, 0, 0)))
        {
            Graphics::Instance().SetWorldSpeed(0);
            EnemyManager::Instance().SetIsUpdateFlag(false);

            player_.lock()->GetMovePlayer()->SetIsInputMove(false);
            player_.lock()->GetAttackPlayer()->SetIsNormalAttack(false);
            player_.lock()->GetMovePlayer()->SetIsDash(false);
            player_.lock()->GetMovePlayer()->SetJumpFlag(false);

            std::shared_ptr<WeaponCom> weapon = GameObjectManager::Instance().Find("CandyPush")->GetComponent<WeaponCom>();
            weapon->SetNodeParent(player_.lock()->GetGameObject());
            weapon->SetNodeName("RightHandMiddle2");
            weapon->SetIsForeverUse(true);
            lockEnemySeconds = 0;

            triangleState_++;
        }
    }
    break;
    //照準を決める
    case 2:
    {
        //カメラの向きにプレイヤー向かせる
        QuaternionStruct cameraQuaternion = GameObjectManager::Instance().Find("Camera")->transform_->GetRotation();

        //カメラコントローラー取得
        std::shared_ptr<PlayerCameraCom> playerCameraCom = player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>();


        //指定エネミー取得
        std::shared_ptr<GameObject> lockEnemy = lockTriangleEnemy_.lock();
        DirectX::XMFLOAT3 enemyPos = lockEnemy->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 playerPos = player_.lock()->GetGameObject()->transform_->GetWorldPosition();

        //カメラ
        DirectX::XMStoreFloat3(&focusEnemy, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&focusEnemy), DirectX::XMLoadFloat3(&enemyPos), elapsedTime * 2));
        playerCameraCom->SetForcusPos(focusEnemy);

        //カメラの向きにプレイヤーを合わせる
        static DirectX::XMFLOAT4 rota = cameraQuaternion.dxFloat4;
        DirectX::XMVECTOR CA = DirectX::XMLoadFloat4(&cameraQuaternion.dxFloat4);
        DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&rota), CA, elapsedTime * 5));

        player_.lock()->GetGameObject()->transform_->SetRotation(rota);
        player_.lock()->GetGameObject()->transform_->SetWorldPosition(GetCameraFromPlayerPos());

        //入力情報を取得
        GamePad& gamePad = Input::Instance().GetGamePad();
        float ax = gamePad.GetAxisRX();
        float ay = gamePad.GetAxisRY();

        //入力されている場合
        //ロックオン変更
        lockEnemySeconds += elapsedTime;
        if (ax * ax + ay * ay > 0.01f && lockEnemySeconds > 0.5f)
        {
            //ロックオン中の敵のポス
            DirectX::XMVECTOR lockEnemyPos = DirectX::XMLoadFloat3(&lockTriangleEnemy_.lock()->transform_->GetWorldPosition());

            //内積して角度見て近い敵をターゲットに
            DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(lockEnemyPos, DirectX::XMLoadFloat3(&playerPos));

            DirectX::XMVECTOR PEForward = DirectX::XMVector3Normalize(PE);
            DirectX::XMVECTOR PERight = DirectX::XMVector3Normalize(DirectX::XMVector3Cross({ 0,1,0 }, PEForward));


            DirectX::XMVECTOR InputVec = DirectX::XMVector3Normalize({ ax ,0,ay });
            float len = FLT_MAX;
            std::shared_ptr<GameObject> enemyNear;
            std::shared_ptr<GameObject> enemyFar;

            for (auto& enemy : EnemyManager::Instance().GetNearEnemies())
            {
                if (lockTriangleEnemy_.lock()->GetComponent<EnemyCom>()->GetID()
                    == enemy.enemy.lock()->GetComponent<EnemyCom>()->GetID())
                    continue;

                //内積して角度見て近い敵をターゲットに
                DirectX::XMFLOAT3 nearEnemyPos = enemy.enemy.lock()->transform_->GetWorldPosition();
                DirectX::XMVECTOR NextEnemyVec = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&nearEnemyPos), lockEnemyPos);
                DirectX::XMVECTOR NextEnemyVecNorm = DirectX::XMVector3Normalize(NextEnemyVec);

                //入力ベクトルをエネミーの位置からに変換
                DirectX::XMVECTOR InputEnemyVec = DirectX::XMVectorAdd(DirectX::XMVectorScale(PEForward, ay), DirectX::XMVectorScale(PERight, ax));
                InputEnemyVec = DirectX::XMVector3Normalize(DirectX::XMVectorSetY(InputEnemyVec, 0));


                //内積で角度見る
                if (DirectX::XMVector3Dot(InputEnemyVec, NextEnemyVecNorm).m128_f32[0] > 0.5f)
                {
                    float dist = DirectX::XMVector3Length(NextEnemyVec).m128_f32[0];
                    if (len > dist)
                    {
                        len = dist;
                        enemyNear = enemy.enemy.lock();
                    }
                }
            }

            for (auto& enemy : EnemyManager::Instance().GetFarEnemies())
            {
                if (lockTriangleEnemy_.lock()->GetComponent<EnemyCom>()->GetID()
                    == enemy.enemy.lock()->GetComponent<EnemyCom>()->GetID())
                    continue;

                //内積して角度見て近い敵をターゲットに
                DirectX::XMFLOAT3 farEnemyPos = enemy.enemy.lock()->transform_->GetWorldPosition();
                DirectX::XMVECTOR NextEnemyVec = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&farEnemyPos), lockEnemyPos);
                DirectX::XMVECTOR NextEnemyVecNorm = DirectX::XMVector3Normalize(NextEnemyVec);

                //入力ベクトルをエネミーの位置からに変換
                DirectX::XMVECTOR InputEnemyVec = DirectX::XMVectorAdd(DirectX::XMVectorScale(PEForward, ay), DirectX::XMVectorScale(PERight, ax));
                InputEnemyVec = DirectX::XMVector3Normalize(DirectX::XMVectorSetY(InputEnemyVec, 0));


                //内積で角度見る
                if (DirectX::XMVector3Dot(InputEnemyVec, NextEnemyVecNorm).m128_f32[0] > 0.5f)
                {
                    float dist = DirectX::XMVector3Length(NextEnemyVec).m128_f32[0];
                    if (len > dist)
                    {
                        len = dist;
                        enemyFar = enemy.enemy.lock();
                    }
                }
            }

            //nearとfarで判定
            {
                if (enemyNear || enemyFar)
                {
                    bool endJudge = false;
                    //どちらかが入っていないとき
                    if (!enemyNear)
                    {
                        lockTriangleEnemy_ = enemyFar;
                        endJudge = true;
                    }
                    if (!enemyFar)
                    {
                        lockTriangleEnemy_ = enemyNear;
                        endJudge = true;
                    }

                    if (!endJudge)
                    {
                        //近い方を入れる
                        float nearLen = DirectX::XMVector3Length(DirectX::XMVectorSubtract(lockEnemyPos, DirectX::XMLoadFloat3(&enemyNear->transform_->GetWorldPosition()))).m128_f32[0];
                        float farLen = DirectX::XMVector3Length(DirectX::XMVectorSubtract(lockEnemyPos, DirectX::XMLoadFloat3(&enemyFar->transform_->GetWorldPosition()))).m128_f32[0];

                        if (nearLen < farLen)
                            lockTriangleEnemy_ = enemyNear;
                        else
                            lockTriangleEnemy_ = enemyFar;
                    }
                }
            }
            
            lockEnemySeconds = 0;

            //SE
            triangleCursorSE_->Stop();
            triangleCursorSE_->Play(false);
        }

        //△で次のステート
        if (gamePad.GetButtonDown() & GamePad::BTN_Y)
        {
            Graphics::Instance().SetWorldSpeed(1);
            EnemyManager::Instance().SetIsUpdateFlag(true);

            //SE
            triangleJustAttackSE_->Play(false,2.0f);

            triangleState_++;
        }
    }
    break;
    //武器を飛ばす
    case 3:
    {
        //アニメーションを見て武器を投げる
        std::shared_ptr<AnimationCom> picoAnim = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
        if (picoAnim->GetCurrentAnimationEventIsEnd("trianglePushStop"))
        {
            DirectX::XMFLOAT3 cameraPos = GetCameraPos();

            std::shared_ptr<GameObject> pushObj = GameObjectManager::Instance().Find("Push");
            std::shared_ptr<GameObject> candyObj = GameObjectManager::Instance().Find("CandyPush");
            std::shared_ptr<WeaponCom> weapon = candyObj->GetComponent<WeaponCom>();
            weapon->SetNodeParent(pushObj);
            weapon->SetNodeName("");
            pushObj->transform_->SetWorldPosition(candyObj->transform_->GetWorldPosition());
            pushObj->GetComponent<PushWeaponCom>()->MoveStart(lockTriangleEnemy_.lock()->transform_->GetWorldPosition(), cameraPos);

            //ブラー
            player_.lock()->BlurStartPlayer(2.5f, 1, "", {}, lockTriangleEnemy_.lock());

            triangleState_++;
        }
    }
    break;
    //武器の位置にプレイヤー行く、カメラ武器についていく
    case 4:
    {
        player_.lock()->GetGameObject()->transform_->SetWorldPosition(GetCameraFromPlayerPos());

        std::shared_ptr<GameObject> pushObj = GameObjectManager::Instance().Find("Push");
        //武器が敵に到達した時
        if (!pushObj->GetComponent<PushWeaponCom>()->IsMove())
        {
            JustInisialize();

            //指定エネミー取得
            std::shared_ptr<GameObject> lockEnemy = lockTriangleEnemy_.lock();
            DirectX::XMFLOAT3 enemyPos = lockEnemy->transform_->GetWorldPosition();
            DirectX::XMFLOAT3 playerPos = player_.lock()->GetGameObject()->transform_->GetWorldPosition();

            //エネミーからプレイヤーの正規化ベクトル
            DirectX::XMVECTOR EPNorm = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&enemyPos)));


            player_.lock()->GetMovePlayer()->SetIsInputMove(true);
            player_.lock()->GetAttackPlayer()->SetIsNormalAttack(true);
            player_.lock()->GetMovePlayer()->SetIsDash(true);
            player_.lock()->GetMovePlayer()->SetJumpFlag(true);

            //アタック処理に引き継ぐ
            player_.lock()->GetAttackPlayer()->SetAnimFlagName("squareIdle");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);

            //プレイヤを敵の前に移動
            DirectX::XMFLOAT3 movePos;
            DirectX::XMStoreFloat3(&movePos, EPNorm);

            movePos.x = enemyPos.x + movePos.x * 2;
            movePos.y = enemyPos.y + movePos.y * 2;
            movePos.z = enemyPos.z + movePos.z * 2;

            player_.lock()->GetGameObject()->transform_->SetWorldPosition(movePos);

            //プレイヤーからエネミー
            DirectX::XMFLOAT3 pe;
            
            DirectX::XMStoreFloat3(&pe, 
                DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSetY(EPNorm, 0)), -1));

            player_.lock()->GetGameObject()->transform_->SetRotation(QuaternionStruct::LookRotation(pe));

            //カメラコントローラー取得
            std::shared_ptr<PlayerCameraCom> playerCameraCom = player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>();

            playerCameraCom->SetIsJust(false);

            std::shared_ptr<GameObject> candyObj = GameObjectManager::Instance().Find("CandyPush");
            std::shared_ptr<WeaponCom> weapon = candyObj->GetComponent<WeaponCom>();
            weapon->SetIsForeverUse(false);

            //ロックオンターゲット変える
            player_.lock()->SetLockOn(PlayerCom::LOCK_TARGET::NORMAL_LOCK);

            justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
        }
    }
    break;
    }
}

void JustAvoidPlayer::JustDirectionUpdate(float elapsedTime)
{
    //世界の色変える
    switch (justSpriteState_)
    {
    case 0:
    {
        isJustSprite_ = true;
        Sprite::DissolveConstans& dissolveData = justSprite_->GetEffectSpriteData().dissolveConstant;
        dissolveData.dissolveThreshold = 0;
        dissolveData.edgeThreshold = 1;

        justSpriteState_++;
    }
    break;
    case 1:
    {
        Sprite::DissolveConstans& dissolveData = justSprite_->GetEffectSpriteData().dissolveConstant;
        dissolveData.dissolveThreshold += 0.5f * elapsedTime;
        if (dissolveData.dissolveThreshold > 0.5f)justSpriteState_++;
    }
    break;
    case 2:
    {
        //待機用
    }
    break;
    case 10:
    {
        //□ジャスト攻撃
        Sprite::DissolveConstans& dissolveData = justSprite_->GetEffectSpriteData().dissolveConstant;
        dissolveData.dissolveThreshold += 0.5f * elapsedTime;
        if (dissolveData.dissolveThreshold > 1)
        {
            dissolveData.dissolveThreshold = 1;
            justSpriteState_++;
        }
    }
    break;
    case 11:
    {
        //時間戻るまで待機
        if (!EnemyManager::Instance().GetIsSlow())
        {
            //パーティクル演出終了
            SetJustUnderParticle(false);
            justSpriteState_ = 20;

            //SE
            squareSlowInOutSE_->Play(false, 3.0f);
            squareSlowSE_->Stop();
        }
    }
    break;
    case 20:
    {
        //戻す
        Sprite::DissolveConstans& dissolveData = justSprite_->GetEffectSpriteData().dissolveConstant;
        dissolveData.dissolveThreshold -= 1.5f * elapsedTime;
        if (dissolveData.dissolveThreshold < 0)justSpriteState_++;
    }
    break;
    case 21:
    {
        //終了処理
        //パーティクル
        std::shared_ptr<ParticleSystemCom> particle = GameObjectManager::Instance().Find("justUnderParticle")->GetComponent<ParticleSystemCom>();
        particle->GetGameObject()->SetEnabled(false);

        //スプライト
        isJustSprite_ = false;
        justSpriteState_ = -1;

        //カメラを戻す
        player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>()->SetChangeRange(0.1f, DefaultRange);
    }
    break;
    }

    if (justSpriteState_ > 1 && justSpriteState_ <= 20)
    {
        //カメラを引く
        player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>()->SetChangeRange(10.0f, FarRange - 1.0f);
    }
}

void JustAvoidPlayer::SetJustUnderParticle(bool flag)
{
    std::shared_ptr<ParticleSystemCom> particle = GameObjectManager::Instance().Find("justUnderParticle")->GetComponent<ParticleSystemCom>();

    if (flag)
    {
        particle->GetGameObject()->SetEnabled(true);
        particle->SetRoop(true);
        particle->Restart();
    }
    else
    {
        particle->SetRoop(false);
    }
}

//ジャスト回避出来たか判定
void JustAvoidPlayer::JustAvoidJudge()
{
    //一番近い敵を探すラムダ式
    auto SearchMostNearEnemy = [&]()
    {
        std::shared_ptr<GameObject> obj;
        float len = FLT_MAX;
        DirectX::XMVECTOR PPos = DirectX::XMLoadFloat3(&player_.lock()->GetGameObject()->transform_->GetWorldPosition());

        for (auto& nearEnemy : EnemyManager::Instance().GetNearEnemies())
        {
            DirectX::XMVECTOR EPos = DirectX::XMLoadFloat3(&nearEnemy.enemy.lock()->transform_->GetWorldPosition());
            float dist = DirectX::XMVector3Length(DirectX::XMVectorSubtract(PPos, EPos)).m128_f32[0];
            if (len > dist)
            {
                len = dist;
                obj = nearEnemy.enemy.lock();
            }
        }

        for (auto& farEnemy : EnemyManager::Instance().GetFarEnemies())
        {
            DirectX::XMVECTOR EPos = DirectX::XMLoadFloat3(&farEnemy.enemy.lock()->transform_->GetWorldPosition());
            float dist = DirectX::XMVector3Length(DirectX::XMVectorSubtract(PPos, EPos)).m128_f32[0];
            if (len > dist)
            {
                len = dist;
                obj = farEnemy.enemy.lock();
            }
        }

        return obj;
    };

    //ジャスト回避の当たり判定
    std::vector<HitObj> hitGameObj = player_.lock()->GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
    //ジャスト回避したエネミーを保存
    std::weak_ptr<GameObject> enemy;
    for (auto& hitObj : hitGameObj)
    {
        if (COLLIDER_TAG::JustAvoid != hitObj.gameObject.lock()->GetComponent<Collider>()->GetMyTag())continue;

        //今のところ弾だけEnemyComがないので弾用
        if (!hitObj.gameObject.lock()->GetParent()->GetComponent<EnemyCom>())
        {
            if (!enemy.lock())
                enemy = SearchMostNearEnemy();
            continue;
        }

        //最初だけそのまま入れる
        if (!enemy.lock()) {
            enemy = hitObj.gameObject.lock()->GetParent();
            continue;
        }
        //一番近い敵を保存
        DirectX::XMFLOAT3 pPos = player_.lock()->GetGameObject()->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 ePos = hitObj.gameObject.lock()->GetParent()->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 eOldPos = enemy.lock()->transform_->GetWorldPosition();

        DirectX::XMVECTOR PPos = DirectX::XMLoadFloat3(&pPos);
        DirectX::XMVECTOR EPos = DirectX::XMLoadFloat3(&ePos);
        DirectX::XMVECTOR EOldPos = DirectX::XMLoadFloat3(&eOldPos);

        float currentLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(PPos, EPos)));
        float oldLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(PPos, EOldPos)));

        //比較
        if (currentLength < oldLength)
            enemy = hitObj.gameObject.lock()->GetParent();
    }

    justHitEnemy_ = enemy;
}

//ジャスト回避を開始する
void JustAvoidPlayer::StartJustAvoid()
{
    Score::Instance().AddScore(654321);

    //無敵時間
    player_.lock()->GetGameObject()->GetComponent<CharacterStatusCom>()
        ->SetInvincibleNonDamage(2);
    isJustJudge_ = true;
    justAvoidState_ = 0;

    //エフェクト
    std::shared_ptr<GameObject> particleSpark = ParticleComManager::Instance().SetEffect(ParticleComManager::JUST_SPARK
        , player_.lock()->GetGameObject()->transform_->GetWorldPosition());
    particleSpark->SetName("particleSpark");
    particleSpark->GetComponent<ParticleSystemCom>()->SetIsWorldSpeed(false);
    //エフェクト
    particleSpark = ParticleComManager::Instance().SetEffect(ParticleComManager::JUST_SPARK
        , player_.lock()->GetGameObject()->transform_->GetWorldPosition());
    particleSpark->SetName("particleSpark1");
    particleSpark->GetComponent<ParticleSystemCom>()->SetIsWorldSpeed(false);

    //ブラー
    player_.lock()->BlurStartPlayer(2.5f, 1, "Head");

    //SE
    justSE_->Stop();
    justSE_->Play(false);
}

void JustAvoidPlayer::JustAvoidDirectionEnd(float elapsedTime)
{
    if (!playerDirection_)return;

    //プレイヤー色戻す
    DirectX::XMFLOAT4 playerColor = player_.lock()->GetGameObject()->GetComponent<RendererCom>()->
        GetModel()->GetMaterialColor();
    playerColor.y -= 0.8f * elapsedTime;
    playerColor.w -= 0.5f * elapsedTime;
    //両方戻っていれば演出終了
    if (playerColor.y < 1 && playerColor.w < 1)playerDirection_ = false;
    if (playerColor.y < 1)playerColor.y = 1;
    if (playerColor.w < 1)playerColor.w = 1;
    player_.lock()->GetGameObject()->GetComponent<RendererCom>()->
        GetModel()->SetMaterialColor(playerColor);
}

void JustAvoidPlayer::justDirectionRender2D()
{
    if (!isJustSprite_)return;
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc= graphics.GetDeviceContext();

    justSprite_->Render(dc, 0, 0, static_cast<float>(graphics.GetScreenWidth()), static_cast<float>(graphics.GetScreenHeight())
        , 0, 0, static_cast<float>(justSprite_->GetTextureWidth()), static_cast<float>(justSprite_->GetTextureHeight())
        , 0, 1, 0.89f, 0, 1
    );
}
