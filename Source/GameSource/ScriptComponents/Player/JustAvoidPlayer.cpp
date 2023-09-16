#include "JustAvoidPlayer.h"

#include "PlayerCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "Components\TransformCom.h"
#include "Components\RendererCom.h"
#include "Components\CameraCom.h"
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


//ジャスト回避初期化
void JustAvoidPlayer::JustInisialize()
{
    justAvoidState_ = -1;
    isJustJudge_ = false;

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
        player_.lock()->GetMovePlayer()->moveParamType_ = MovePlayer::MOVE_PARAM::JUSTDASH;
        move->SetMoveMaxSpeed(player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::JUSTDASH].moveMaxSpeed);
        move->SetMoveAcceleration(player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::JUSTDASH].moveAcceleration);
        justAvoidTimer_ = justAvoidTime_;

        player_.lock()->GetMovePlayer()->isInputMove_ = false;
        player_.lock()->GetAttackPlayer()->isNormalAttack_ = false;
        player_.lock()->GetMovePlayer()->isDash_ = false;

        //入力方向をみてアニメーション再生
        DirectX::XMVECTOR Input = DirectX::XMLoadFloat3(&player_.lock()->GetMovePlayer()->inputMoveVec_);

        //アニメーション再生
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

        //animator->SetAnimationSpeedOffset(0.3f);

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
            justAnim->SetAnimationSpeed(0.2f);  //アニメ速度スローに
            Model* justModel = justPico[i]->GetComponent<RendererCom>()->GetModel();
            justModel->SetMaterialColor({ 1.2f,1.2f,0.1f,0.65f });  //色初期化

            //出す
            justPico[i]->SetEnabled(true);
            if (inputFlag)
                justAnim->PlayAnimation(25, false);
            else
                justAnim->PlayAnimation(24, false);
        }

        //敵の方を向く
        player_.lock()->GetGameObject()->transform_->LookAtTransform(justHitEnemy_->transform_->GetWorldPosition());

        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUST);



        ////カメラシェイク
        //GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()->CameraShake(0.1f, 0.25f);

        //ヒットストップ
        GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()->HitStop(0.2f);

        justAvoidState_++;
        break;
    }
    //分身移動処理＆アニメスピード戻す
    case 1:
    {
        bool endFlag = false;
        DirectX::XMFLOAT3 justPicoPos[4];
        for (int i = 0; i < 4; ++i)
        {
            justPicoPos[i] = justPico[i]->transform_->GetLocalPosition();
        }
        //縦
        for (int f = 0; f < 2; ++f)
        {
            float g = 1;
            if (f == 1)g = -1;
            if (justPicoPos[f].z * justPicoPos[f].z < 100 * 100)
            {
                justPicoPos[f].z += 500 * elapsedTime * g;
                justPico[f]->transform_->SetLocalPosition(justPicoPos[f]);
            }
        }
        //横
        for (int r = 0; r < 2; ++r)
        {
            float g = 1;
            if (r == 1)g = -1;
            if (justPicoPos[r + 2].x * justPicoPos[r + 2].x < 100 * 100)
            {
                justPicoPos[r + 2].x += 500 * elapsedTime * g;
                justPico[r + 2]->transform_->SetLocalPosition(justPicoPos[r + 2]);
            }
            else
                endFlag = true;
        }

        //ステート終わりにアニメスピードを戻す
        if (endFlag)
        {
            for (int i = 0; i < 4; ++i)
            {
                std::shared_ptr<AnimationCom> justAnim = justPico[i]->GetComponent<AnimationCom>();
                //justAnim->SetAnimationSpeed(1.0f);
            }
            justAvoidState_++;
        }

        break;
    }
    //分身消す
    //プレイヤー出す
    case 2:
    {
        bool endFlag = false;
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
        }
        if (endFlag)
            justAvoidState_++;

        break;
    }
    //反撃受け入れ
    case 3:
    {
        //アニメ終了で反撃終了
        if (!player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->IsPlayAnimation())
        {
            int i = 0;
            for (int i = 0; i < 4; ++i)
            {
                std::shared_ptr<AnimationCom> justAnim = justPico[i]->GetComponent<AnimationCom>();
                justAnim->SetAnimationSpeed(1.0f);
            }

            std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
            animator->SetAnimationSpeedOffset(1.0f);
        }

        //ジャスト回避終了タイマー
        justAvoidTimer_ -= elapsedTime;
        if (justAvoidTimer_ < 0)
        {
            JustInisialize();
            player_.lock()->GetMovePlayer()->moveParamType_ = MovePlayer::MOVE_PARAM::RUN;
            move->SetMoveMaxSpeed(player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::RUN].moveMaxSpeed);
            move->SetMoveAcceleration(player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::RUN].moveAcceleration);
            player_.lock()->GetMovePlayer()->isInputMove_ = true;
            player_.lock()->GetAttackPlayer()->isNormalAttack_ = true;
            player_.lock()->GetMovePlayer()->isDash_ = true;
            break;
        }

        break;
    }
    }

    //移動出来るか
    int animIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();
    if (animIndex != 25 && animIndex != 24)
    {
        //移動方向を決める（ジャスト回避中は勝手に移動する）
        DirectX::XMFLOAT3 Direction;

        DirectX::XMFLOAT3 inputVec = player_.lock()->GetMovePlayer()->inputMoveVec_;
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
        Direction.x *= player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::JUSTDASH].moveSpeed;
        Direction.z *= player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::JUSTDASH].moveSpeed;
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
    if (gamePad.GetButtonDown() & GamePad::BTN_Y)
    {
        JustInisialize();
        player_.lock()->GetMovePlayer()->moveParamType_ = MovePlayer::MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::DASH].moveMaxSpeed);
        move->SetMoveAcceleration(player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::DASH].moveAcceleration);

        justAvoidKey_ = JUST_AVOID_KEY::SQUARE;

        player_.lock()->GetMovePlayer()->isInputTrun_ = false;

        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);

        //敵の方を向く
        player_.lock()->GetGameObject()->transform_->LookAtTransform(justHitEnemy_->transform_->GetWorldPosition());
    }
}

//□反撃
void JustAvoidPlayer::JustAvoidanceSquare(float elapsedTime)
{
    //敵に接近する
    {
        std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

        DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&player_.lock()->GetGameObject()->transform_->GetWorldPosition());
        DirectX::XMVECTOR EnemyPos = DirectX::XMLoadFloat3(&justHitEnemy_->transform_->GetWorldPosition());
        DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(EnemyPos, Pos);
        float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
        //敵の近くまで移動する
        if (length < 1.5f)
        {
            JustInisialize();
            player_.lock()->GetMovePlayer()->moveParamType_ = MovePlayer::MOVE_PARAM::RUN;
            move->SetMoveMaxSpeed(player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::RUN].moveMaxSpeed);
            move->SetMoveAcceleration(player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::RUN].moveAcceleration);
            justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
            player_.lock()->GetMovePlayer()->isInputMove_ = true;
            player_.lock()->GetAttackPlayer()->isNormalAttack_ = true;
            player_.lock()->GetMovePlayer()->isDash_ = true;

            //アタック処理に引き継ぐ
            player_.lock()->GetAttackPlayer()->animFlagName_ = "squareJust";
        }

        DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(PE);
        Dir = DirectX::XMVectorScale(Dir, player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::DASH].moveSpeed);
        DirectX::XMFLOAT3 dir;
        DirectX::XMStoreFloat3(&dir, Dir);

        //力に加える
        move->AddForce(dir);

    }
}
