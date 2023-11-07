#include "JustAvoidPlayer.h"

#include "PlayerCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "Components\TransformCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\CameraCom.h"
#include "../CharacterStatusCom.h"

#include "Input/Input.h"

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
    JustAvoidDirection(elapsedTime);

    //反撃処理更新
    switch (justAvoidKey_)
    {
        //□反撃
    case JUST_AVOID_KEY::SQUARE:
        JustAvoidanceSquare(elapsedTime);
        break;
    case JUST_AVOID_KEY::TRIANGLE:
        break;
    }
}

void JustAvoidPlayer::OnGui()
{

}


//ジャスト回避初期化
void JustAvoidPlayer::JustInisialize()
{
    justAvoidState_ = -1;
    isJustJudge_ = false;
    Graphics::Instance().SetWorldSpeed(1);
    //カラーグレーディング戻す
    Graphics::Instance().shaderParameter3D_.colorGradingData.saturation = 1.5f;

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

        //プレイヤー透明に
        player_.lock()->GetGameObject()->GetComponent<RendererCom>()->
            GetModel()->SetMaterialColor({ 1,1,1,0 });

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

        //敵の方を向く
        if (!justHitEnemy_.expired())
            player_.lock()->GetGameObject()->transform_->LookAtTransform(justHitEnemy_.lock()->transform_->GetWorldPosition());

        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUST);


        //カラーグレーディング
        Graphics::Instance().shaderParameter3D_.colorGradingData.saturation = 0.4f;

        ////カメラシェイク
        //GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()->CameraShake(0.1f, 0.25f);

        //ヒットストップ
        GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()
            ->HitStop(0.2f);
        hitStopEnd_ = false;

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
    if (justAvoidState_ < 3)return;

    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    //ボタンで反撃変える
    GamePad& gamePad = Input::Instance().GetGamePad();

    //□の場合
    if (gamePad.GetButtonDown() & GamePad::BTN_X)
    {
        JustInisialize();
        player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::DASH);

        justAvoidKey_ = JUST_AVOID_KEY::SQUARE;

        player_.lock()->GetMovePlayer()->SetIsInputMove(false);

        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);

        //敵の方を向く
        player_.lock()->GetGameObject()->transform_->LookAtTransform(justHitEnemy_.lock()->transform_->GetWorldPosition());
    }
}

//□反撃
void JustAvoidPlayer::JustAvoidanceSquare(float elapsedTime)
{
    //敵に接近する
    {
        std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

        DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&player_.lock()->GetGameObject()->transform_->GetWorldPosition());
        DirectX::XMVECTOR EnemyPos = Pos;
        if (!justHitEnemy_.expired())
            EnemyPos = DirectX::XMLoadFloat3(&justHitEnemy_.lock()->transform_->GetWorldPosition());
        DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(EnemyPos, Pos);
        float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
        //敵の近くまで移動する
        if (length < 1.5f)
        {
            JustInisialize();
            player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::RUN);
            justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
            player_.lock()->GetMovePlayer()->SetIsInputMove(true);
            player_.lock()->GetAttackPlayer()->SetIsNormalAttack(true);
            player_.lock()->GetMovePlayer()->SetIsDash(true);

            //アタック処理に引き継ぐ
            player_.lock()->GetAttackPlayer()->SetAnimFlagName("squareJust");
        }

        DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(PE);
        Dir = DirectX::XMVectorScale(Dir, player_.lock()->GetMovePlayer()->GetMoveParam(MovePlayer::MOVE_PARAM::DASH).moveSpeed);
        DirectX::XMFLOAT3 dir;
        DirectX::XMStoreFloat3(&dir, Dir);

        //力に加える
        move->AddForce(dir);

    }
}

//ジャスト回避出来たか判定
void JustAvoidPlayer::JustAvoidJudge()
{
    //ジャスト回避の当たり判定
    std::vector<HitObj> hitGameObj = player_.lock()->GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
    //ジャスト回避したエネミーを保存
    std::weak_ptr<GameObject> enemy;
    for (auto& hitObj : hitGameObj)
    {
        if (COLLIDER_TAG::JustAvoid != hitObj.gameObject.lock()->GetComponent<Collider>()->GetMyTag())continue;

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
    //無敵時間
    player_.lock()->GetGameObject()->GetComponent<CharacterStatusCom>()
        ->SetInvincibleNonDamage(2);
    isJustJudge_ = true;
    justAvoidState_ = 0;
}

void JustAvoidPlayer::JustAvoidDirection(float elapsedTime)
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
