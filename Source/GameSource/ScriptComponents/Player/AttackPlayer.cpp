#include "AttackPlayer.h"

#include "PlayerCom.h"
#include "Components\ColliderCom.h"
#include "Components\RendererCom.h"
#include "Components\TransformCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "SystemStruct\QuaternionStruct.h"
#include "Components/ParticleComManager.h"
#include "../Weapon\WeaponCom.h"

#include "Graphics/Graphics.h"
#include "Input/Input.h"
#include <imgui.h>

void AttackPlayer::Update(float elapsedTime)
{
    //コンボ継続確認処理
    ComboJudge();
    
    //先行入力時はtrue
    bool isLeadInput = false;
    if (attackLeadInputKey_ != ATTACK_KEY::NULL_KEY)
    {
        if (DoComboAttack())
            isLeadInput = true;
    }


    //入力を見て処理をする
    if (IsAttackInput(elapsedTime) || isLeadInput)
    {
        if (isLeadInput)
        {
            attackLeadInputKey_ = ATTACK_KEY::NULL_KEY;
        }

        switch (attackKey_)
        {
        case AttackPlayer::ATTACK_KEY::SQUARE:
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP_FALL)break;
            if (!isSquareAttack_)break;
            SquareInput();
            break;
        case AttackPlayer::ATTACK_KEY::TRIANGLE:
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP_FALL)break;
            TriangleInput();
            break;
        case AttackPlayer::ATTACK_KEY::NULL_KEY:
            break;
        }

    }

    //コンボ処理
    ComboProcess(elapsedTime);

    //攻撃中の動き
    AttackMoveUpdate(elapsedTime);
}

void AttackPlayer::OnGui()
{
    ImGui::Checkbox("isNormalAttack_", &isNormalAttack_);
    ImGui::Checkbox("isComboJudge_", &isComboJudge_);
    ImGui::InputInt("SquareCount", &comboSquareCount_, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputInt("TriangleCount", &comboTriangleCount_, ImGuiInputTextFlags_ReadOnly);
    ImGui::Checkbox("isSquareAttack", &isSquareAttack_);
    ImGui::Checkbox("isJumpSquareInput_", &isJumpSquareInput_);
    ImGui::DragFloat("jumpAttackComboWaitTimer_", &jumpAttackComboWaitTimer_);
}

//コンボ継続確認処理
void AttackPlayer::ComboJudge()
{
    //現在のアニメインデックス
    int currentAnimIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    //コンボ継続判定処理
    if (isComboJudge_)
    {
        static int oldSquareCount = 0;
        static int oldTriangleCount = 0;
        static int oldAnim = 0;
        if (comboSquareCount_ > 0 || comboTriangleCount_ > 0) {
            //前フレームのコンボとアニメーションを見て
            //コンボ終了か見る
            if (oldSquareCount == comboSquareCount_ && oldTriangleCount == comboTriangleCount_ || comboJudgeEnd_)
            {
                if (oldAnim != currentAnimIndex || comboJudgeEnd_)
                {
                    //コンボ終了
                    AttackFlagEnd();

                    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

                    //空中かの判定
                    if (playerObj->GetComponent<MovementCom>()->OnGround())
                    {
                        //着地時に空中攻撃した時の回避
                        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP)
                            playerObj->GetComponent<AnimatorCom>()->SetTriggerOn("idle");

                        //プレイヤーステータスの更新
                        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::IDLE);
                    }
                    else
                    {
                        //プレイヤーステータスの更新
                        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUMP);
                    }

                    //□攻撃できなくする
                    if (isJumpSquareInput_)
                    {
                        isSquareAttack_ = false;
                        isJumpSquareInput_ = false;
                    }
                    comboJudgeEnd_ = false;
                }
            }
        }
        oldSquareCount = comboSquareCount_;
        oldTriangleCount = comboTriangleCount_;
        oldAnim = currentAnimIndex;
    }

    //コンボ終了処理がONの場合は切る
    if (comboJudgeEnd_)
    {
        comboJudgeEnd_ = false;
    }

    //コンボ中処理
    if (comboSquareCount_ > 0 || comboTriangleCount_ > 0)
    {
        player_.lock()->GetMovePlayer()->SetIsInputMove(false);
    }
}

//入力情報を保存
bool AttackPlayer::IsAttackInput(float elapsedTime)
{
    //先行入力時は上書きする
    if (attackLeadInputKey_ == ATTACK_KEY::NULL_KEY)
        attackKey_ = ATTACK_KEY::NULL_KEY;
    else
        attackKey_ = attackLeadInputKey_;

    if (!isNormalAttack_)return false;

    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    if (animFlagName_.size() > 0)
    {
        animator->SetTriggerOn(animFlagName_);
        comboSquareCount_++;
        animFlagName_ = "";
    }
    else
    {
        GamePad& gamePad = Input::Instance().GetGamePad();
        if (gamePad.GetButtonDown() & GamePad::BTN_X)   //□
        {
            attackKey_ = ATTACK_KEY::SQUARE;
            return true;
        }

        if (gamePad.GetButtonDown() & GamePad::BTN_Y)   //△
        {
            attackKey_ = ATTACK_KEY::TRIANGLE;
            return true;
        }
    }

    return false;
}

//□入力時処理
void AttackPlayer::SquareInput()
{
    //攻撃中は先行入力に保存
    if (InAttackJudgeNow())
    {
        attackLeadInputKey_ = ATTACK_KEY::SQUARE;
        return;
    }

    //アニメインデックスで、コンボカウントリセット
    AttackComboCountReset();

    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //今のコンボ状態で遷移を変える
    if (comboSquareCount_ == 0) //コンボ0
    {
        attackLeadInputKey_ = ATTACK_KEY::NULL_KEY;

        std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
        //空中攻撃
        if (!playerObj->GetComponent<MovementCom>()->OnGround())
        {
            animator->ResetParameterList();
            animator->SetTriggerOn("squareJump");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);

            isJumpSquareInput_ = true;
        }
        //地上攻撃
        else
        {
            isJumpSquareInput_ = false;
            //ダッシュ攻撃か判定
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
            {
                animator->ResetParameterList();
                animator->SetTriggerOn("triangleDash");
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_DASH);
                NormalAttack(true);
            }
            else
            {
                animator->ResetParameterList();
                animator->SetTriggerOn("squareIdle");
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);
                NormalAttack(true);

                //最初に△入力を0にする
                comboTriangleCount_ = 0;
            }
        }

        comboSquareCount_++;

        //ダッシュ終了フラグ
        player_.lock()->GetMovePlayer()->DashEndFlag();

        return;
    }

    //コンボ1〜
    if (comboSquareCount_ >= 3)return;

    if (DoComboAttack())    //コンボ受付判定
    {
        //仮エフェクト
        ParticleComManager::Instance().SetEffect(ParticleComManager::DOOM_SWEETS
            , player_.lock()->GetGameObject()->transform_->GetWorldPosition());

        std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
        //空中攻撃
        if (!playerObj->GetComponent<MovementCom>()->OnGround())
        {
            animator->ResetParameterList();
            animator->SetTriggerOn("square");
            NormalAttack();
            comboSquareCount_++;
            jumpAttackComboWaitTimer_ = 0;

            isJumpSquareInput_ = true;


            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);
            return;
        }
        //地上攻撃
        else
        {
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK)  //通常攻撃時のコンボ
            {
                animator->SetTriggerOn("square");
                NormalAttack();
                comboSquareCount_++;

                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);
                return;
            }
            //ダッシュ攻撃
            else if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH)
            {
                animator->SetTriggerOn("square");
                DashAttack(2);
                comboSquareCount_++;

                return;
            }
        }
    }
}

//△入力時処理
void AttackPlayer::TriangleInput()
{
    //攻撃中は先行入力に保存
    if (InAttackJudgeNow())
    {
        attackLeadInputKey_ = ATTACK_KEY::TRIANGLE;
        return;
    }

    //アニメインデックスで、コンボカウントリセット
    AttackComboCountReset();

    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    //プレイヤーステータス取得
    PlayerCom::PLAYER_STATUS playerStatus = player_.lock()->GetPlayerStatus();

    //ジャンプ中処理
    {
        //ジャンプ中切りおろし
        if ((playerStatus == PlayerCom::PLAYER_STATUS::JUMP
            || playerStatus == PlayerCom::PLAYER_STATUS::JUMP_BACK_DASH
            || playerStatus == PlayerCom::PLAYER_STATUS::JUMP_DASH
            || playerStatus == PlayerCom::PLAYER_STATUS::ATTACK_JUMP)
            && comboSquareCount_ <= 3
            )
        {
            //コンボ中ならreturn
            if (playerStatus == PlayerCom::PLAYER_STATUS::ATTACK_JUMP)
                if (!DoComboAttack())//コンボ受付判定
                    return;

            animator->ResetParameterList();
            animator->SetTriggerOn("triangleJumpDown");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);
            comboSquareCount_ = 10;
            isJumpSquareInput_ = false;
            isComboJudge_ = false;
            jumpAttackComboWaitTimer_ = 0;

            player_.lock()->GetGameObject()->GetComponent<MovementCom>()->ZeroVelocity();

            //切りおろし中は移動不可に
            player_.lock()->GetMovePlayer()->SetIsInputMove(false);
            player_.lock()->GetMovePlayer()->SetIsInputTurn(false);
            player_.lock()->GetMovePlayer()->DashEndFlag();
            isJumpFall_ = true;
        }
    }

    //コンボカウントを見て攻撃を変える
    if (comboSquareCount_ == 0) //コンボ0
    {
        attackLeadInputKey_ = ATTACK_KEY::NULL_KEY;

        //切り上げ攻撃
        if(player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::IDLE
            || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::MOVE)
        {
            animator->ResetParameterList();
            animator->SetTriggerOn("triangleJump");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP);
            NormalAttack();

            player_.lock()->GetMovePlayer()->DashEndFlag();

            //ジャンプカウント減らす
            player_.lock()->GetMovePlayer()->SetJumpCount(player_.lock()->GetMovePlayer()->GetJumpCount() - 1);

            comboTriangleCount_++;
        }

        return;
    }
    else if (comboSquareCount_ <= 3&& comboTriangleCount_<3)
    {
        if (DoComboAttack())
        {
            animator->ResetParameterList();
            animator->SetTriggerOn("triangle");

            NormalAttack();
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);
            comboTriangleCount_++;
        }
    }


}


//コンボ処理
void AttackPlayer::ComboProcess(float elapsedTime)
{
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

    //ジャンプ攻撃更新処理
    {
        std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

        //コンボ設定
        {
            if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP)
            {
                //コンボ受付時間設定
                if (isJumpSquareInput_)
                {
                    std::shared_ptr<AnimationCom> animCom = playerObj->GetComponent<AnimationCom>();
                    if (jumpAttackComboWaitTimer_ <= 0
                        && attackKey_ == ATTACK_KEY::NULL_KEY)
                    {
                        for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
                        {
                            //頭がAutoCollisionなら
                            if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;
                            //アニメーションイベントのエンドフレーム後なら
                            if (!animCom->GetCurrentAnimationEventIsEnd(animEvent.name.c_str()))continue;

                            jumpAttackComboWaitTimer_ = jumpAttackComboWaitTime_;
                            isComboJudge_ = false;
                        }
                    }
                }
            }
            else
            {
                isJumpSquareInput_ = false;
            }

            //コンボ受付時間処理
            if (jumpAttackComboWaitTimer_ > 0)
            {
                jumpAttackComboWaitTimer_ -= elapsedTime;
                if (jumpAttackComboWaitTimer_ <= 0)
                {
                    isComboJudge_ = true;
                    comboJudgeEnd_ = true;
                }
            }
        }

        //重力設定
        {
            static bool isSetGravity = false;
            //アニメーションイベントを見て重力を設定する
            if (player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("ZeroGravity", DirectX::XMFLOAT3())
                && player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP
                || jumpAttackComboWaitTimer_ > 0
                )
            {
                playerObj->GetComponent<MovementCom>()->ZeroVelocityY();
                playerObj->GetComponent<MovementCom>()->SetGravity(GRAVITY_ZERO);
                isSetGravity = true;
            }
            else if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_JUMP_FALL)    //切り下ろし攻撃時
            {
                playerObj->GetComponent<MovementCom>()->SetGravity(GRAVITY_FALL);
                isSetGravity = true;
            }
            else
            {
                if (isSetGravity)   //ここで変えた時だけ反応するように
                {
                    playerObj->GetComponent<MovementCom>()->SetGravity(GRAVITY_NORMAL);
                    isSetGravity = false;
                }
            }
        }

        int index = playerObj->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();
        //切り下ろし処理
        {
            if (index == ANIMATION_PLAYER::JUMP_ATTACK_DOWN_DO)
            {
                player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_JUMP_FALL);
            }

            //切り下ろし後処理
            static bool isJumpAttackEnd = false;
            if (index == ANIMATION_PLAYER::JUMP_ATTACK_DOWN_END && isJumpFall_)
            {
                isJumpFall_ = false;
                player_.lock()->GetMovePlayer()->SetIsInputMove(false);
                isJumpAttackEnd = true;
            }
            else if (index != ANIMATION_PLAYER::JUMP_ATTACK_DOWN_END && isJumpAttackEnd)
            {
                isComboJudge_ = true;
                comboJudgeEnd_ = true;
                player_.lock()->GetMovePlayer()->SetIsInputMove(true);
                isJumpAttackEnd = false;
            }
        }
    }
}



//攻撃動き処理
void AttackPlayer::AttackMoveUpdate(float elapsedTime)
{
    //state初期化
    if (EndAttackState())state_ = -1;

    //攻撃の種類によって動きを変える
    switch (attackFlagState_)
    {
    case ATTACK_FLAG::Normal:
        if (NormalAttackUpdate(elapsedTime) == ATTACK_CODE::EnterAttack)
            attackFlagState_ = ATTACK_FLAG::Null;
        break;
    case ATTACK_FLAG::Dash:
        if (DashAttackUpdate(elapsedTime) == ATTACK_CODE::EnterAttack)
            attackFlagState_ = ATTACK_FLAG::Null;
        break;
    }


    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    //武器取得
    std::shared_ptr<WeaponCom> weapon = playerObj->GetChildFind("Candy")->GetComponent<WeaponCom>();

    //一回でも攻撃が当たっているなら次の攻撃が来るまで、trueにする
    if (weapon->GetOnHit())
    {
        onHitEnemy_ = true;
    }


    //攻撃中の時
    if (comboSquareCount_ > 0)
    {
        //回転不能
        player_.lock()->GetMovePlayer()->SetIsInputTurn(false);

        //攻撃フレーム前だけ回転可能に
        std::shared_ptr<AnimationCom> animCom = playerObj->GetComponent<AnimationCom>();
        for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
        {
            //頭がAutoCollisionなら
            if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;

            //アニメーションイベントのスタートフレーム前なら回転可能
            if (!animCom->GetCurrentAnimationEventIsStart(animEvent.name.c_str()))continue;

            //回転可能
            player_.lock()->GetMovePlayer()->SetIsInputTurn(true);
        }
    }


}

//アニメインデックスで、コンボカウントリセット
void AttackPlayer::AttackComboCountReset()
{
    //現在のアニメインデックス
    int currentAnimIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    //ジャンプ攻撃初段時
    if (currentAnimIndex == ANIMATION_PLAYER::JUMP_ATTACK_01)
    {
        comboSquareCount_ = 1;
    }

    //ダッシュ攻撃終了時
    if (currentAnimIndex == ANIMATION_PLAYER::BIGSWORD_COM2_02)
    {
        //ここにダッシュ切り終わってからにする
        if (DoComboAttack())
            comboSquareCount_ = 0;
    }

    //△３段目終了後
    if (currentAnimIndex == ANIMATION_PLAYER::TRIANGLE_ATTACK_03)
    {
        if (DoComboAttack())
        comboSquareCount_ = 0;
    }
}


void AttackPlayer::NormalAttack(bool middleAssist)
{
    state_ = 0;
    onHitEnemy_ = false;    //攻撃が入力されたらfalseに
    isMiddleAssist_ = middleAssist; //中距離アシスト
    attackFlagState_ = ATTACK_FLAG::Normal;
}

int AttackPlayer::NormalAttackUpdate(float elapsedTime)
{
    if (enemyCopy_.expired() && state_ > 0)
    {
        state_ = ATTACK_CODE::EnterAttack;
        return state_;
    }

    switch (state_)
    {
    case 0:
        //範囲内に敵はいるか
        enemyCopy_ = AssistGetNearEnemy();
        if (!enemyCopy_.lock()) //敵がいない場合
        {
            //ノーマル攻撃の時
            if (isMiddleAssist_)
            {
                //中距離でも敵を探す
                enemyCopy_ = AssistGetMediumEnemy();
                if (enemyCopy_.lock()) //敵がいない場合
                {
                    state_ = 5;
                    break;
                }
            }

            state_ = ATTACK_CODE::EnterAttack;
            break;
        }
        state_++;
        break;

        //近距離アシスト
    case 1:
        //回転
        if (ForcusEnemy(elapsedTime, enemyCopy_.lock(), 50))
            state_++;
        break;

    case 2:
        //接近
        if (ApproachEnemy(enemyCopy_.lock(), 1.5f, 3) || onHitEnemy_)
            state_ = ATTACK_CODE::EnterAttack;

        break;

        //中距離アシスト
    case 5:
    {
        //アニメーター
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

        //ダッシュアニメーション再生
        animator->ResetParameterList();
        animator->SetTriggerOn("dash");
        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK_DASH);
        isComboJudge_ = false;

        state_++;
    }
    break;
    case 6:
        //回転
        if (ForcusEnemy(elapsedTime, enemyCopy_.lock(), 50))
            state_++;
        break;

    case 7:
        //接近
        if (ApproachEnemy(enemyCopy_.lock(), 1.5f, 10))
        {
            isComboJudge_ = true;
            comboSquareCount_++;

            //アニメーター
            std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

            //ダッシュアニメーション再生
            animator->ResetParameterList();
            animator->SetTriggerOn("triangleDash");

            state_ = ATTACK_CODE::EnterAttack;
        }
        break;

    }

    return state_;
}

void AttackPlayer::DashAttack(int comboNum)
{
    if (comboNum == 1)
        state_ = 0;
    if (comboNum == 2)
        state_ = 10;

    onHitEnemy_ = false;    //攻撃が入力されたらfalseに
    attackFlagState_ = ATTACK_FLAG::Dash;
}

int AttackPlayer::DashAttackUpdate(float elapsedTime)
{
    if (enemyCopy_.expired() && state_ > 0)
    {
        state_ = ATTACK_CODE::EnterAttack;
        return state_;
    }

    switch (state_)
    {
    //    //コンボ１
    //case 0:
    //    //範囲内に敵はいるか
    //    enemyCopy_ = AssistGetNearEnemy();
    //    if (!enemyCopy_.lock())
    //    {
    //        state_ = ATTACK_CODE::EnterAttack;
    //        break;
    //    }
    //    state_++;
    //    break;

    //case 1:
    //    //回転
    //    if (ForcusEnemy(elapsedTime, enemyCopy_.lock(), 10))
    //        state_ = ATTACK_CODE::EnterAttack;
    //    break;

        //コンボ2
    case 10:
        //接近
        if (!enemyCopy_.lock())
        {
            state_ = ATTACK_CODE::EnterAttack;
            break;
        }

        if (ApproachEnemy(enemyCopy_.lock(), 1.5f, 10))
            state_ = ATTACK_CODE::EnterAttack;
        break;
    }

    return state_;
}


//コンボ出来るか判定
bool AttackPlayer::DoComboAttack()
{
    //ジャンプ攻撃時コンボ受付時間でtrueに
    if (jumpAttackComboWaitTimer_ > 0)
        return true;

    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    std::shared_ptr<AnimationCom> animCom = playerObj->GetComponent<AnimationCom>();
    for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
    {
        //頭がAutoCollisionなら
        if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;
        //アニメーションイベントのエンドフレーム後ならコンボ可能
        if (!animCom->GetCurrentAnimationEventIsEnd(animEvent.name.c_str()))continue;

        return true;
    }
    return false;
}

//攻撃判定中か判定
bool AttackPlayer::InAttackJudgeNow()
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    std::shared_ptr<AnimationCom> animCom = playerObj->GetComponent<AnimationCom>();
    for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
    {
        //頭がAutoCollisionなら
        if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;
        //アニメーションイベント外ならfalse
        if (!animCom->GetCurrentAnimationEvent(animEvent.name.c_str(), DirectX::XMFLOAT3()))continue;

        return true;
    }
    return false;
}

//アシスト範囲を索敵して近い敵を返す
std::shared_ptr<GameObject> AttackPlayer::AssistGetNearEnemy()
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
    std::shared_ptr<GameObject> enemyNearObj;   //一番近い敵を入れる

    //アシスト判定取得
    std::shared_ptr<Collider> assistColl = playerObj->GetChildFind("attackAssistNear")->GetComponent<Collider>();
    std::vector<HitObj> hitGameObj = assistColl->OnHitGameObject();
    for (auto& hitObj : hitGameObj)
    {
        if (COLLIDER_TAG::Enemy == hitObj.gameObject.lock()->GetComponent<Collider>()->GetMyTag())
        {
            //最初はそのまま入れる
            if (!enemyNearObj)enemyNearObj = hitObj.gameObject.lock();
            //一番近い敵を見つける
            else
            {
                DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemyNearObj->transform_->GetWorldPosition());
                DirectX::XMVECTOR EN = DirectX::XMLoadFloat3(&hitObj.gameObject.lock()->transform_->GetWorldPosition());
                DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());

                DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(E, P);
                DirectX::XMVECTOR PEN = DirectX::XMVectorSubtract(EN, P);

                float lenE = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
                float lenEN = DirectX::XMVectorGetX(DirectX::XMVector3Length(PEN));
                if (lenE > lenEN)enemyNearObj = hitObj.gameObject.lock();
            }
        }
    }

    //敵がいない場合return
    return enemyNearObj;
}

std::shared_ptr<GameObject> AttackPlayer::AssistGetMediumEnemy()
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();
    std::shared_ptr<GameObject> enemyNearObj;   //一番近い敵を入れる

    //アシスト判定取得
    std::shared_ptr<Collider> assistColl = playerObj->GetChildFind("attackAssistMedium")->GetComponent<Collider>();
    std::vector<HitObj> hitGameObj = assistColl->OnHitGameObject();
    for (auto& hitObj : hitGameObj)
    {
        if (COLLIDER_TAG::Enemy == hitObj.gameObject.lock()->GetComponent<Collider>()->GetMyTag())
        {
            //最初はそのまま入れる
            if (!enemyNearObj)enemyNearObj = hitObj.gameObject.lock();
            //一番近い敵を見つける
            else
            {
                DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemyNearObj->transform_->GetWorldPosition());
                DirectX::XMVECTOR EN = DirectX::XMLoadFloat3(&hitObj.gameObject.lock()->transform_->GetWorldPosition());
                DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());

                DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(E, P);
                DirectX::XMVECTOR PEN = DirectX::XMVectorSubtract(EN, P);

                float lenE = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
                float lenEN = DirectX::XMVectorGetX(DirectX::XMVector3Length(PEN));
                if (lenE > lenEN)enemyNearObj = hitObj.gameObject.lock();
            }
        }
    }

    //敵がいない場合return
    return enemyNearObj;
}

//敵に接近する( true:接近完了　false:接近中 )
bool AttackPlayer::ApproachEnemy(std::shared_ptr<GameObject> enemy, float dist, float speed)
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());
    DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->transform_->GetWorldPosition());

    DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(E, P);
    //距離が縮まればtrue
    if (dist > DirectX::XMVectorGetX(DirectX::XMVector3Length(PE)))
    {
        playerObj->GetComponent<MovementCom>()->ZeroNonMaxSpeedVelocity();
        return true;
    }

    //距離を詰める
    PE = DirectX::XMVector3Normalize(PE);
    DirectX::XMFLOAT3 vec;
    DirectX::XMStoreFloat3(&vec, DirectX::XMVectorScale(PE, speed));
    vec.y = 0;
    
    //プレイヤー移動
    playerObj->GetComponent<MovementCom>()->AddNonMaxSpeedForce(vec);
    return false;
}


//敵の方向へ回転する ( true:完了 )
bool AttackPlayer::ForcusEnemy(float elapsedTime, std::shared_ptr<GameObject> enemy, float rollSpeed)
{
    std::shared_ptr<GameObject> playerObj = player_.lock()->GetGameObject();

    DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&playerObj->transform_->GetWorldPosition());
    DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->transform_->GetWorldPosition());

    //プレイヤーからエネミーの方向のクォータニオンを取得
    DirectX::XMFLOAT3 peR;
    DirectX::XMStoreFloat3(&peR, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(E, P)));
    QuaternionStruct peQ;
    peQ = QuaternionStruct::LookRotation({ peR.x,0,peR.z });

    DirectX::XMVECTOR PEQ = DirectX::XMLoadFloat4(&peQ.dxFloat4);
    DirectX::XMVECTOR PQ = DirectX::XMLoadFloat4(&playerObj->transform_->GetRotation());

    float rot = DirectX::XMVectorGetX(DirectX::XMQuaternionDot(PQ, PEQ));

    if (1 - rot * rot < 0.01f)return true;

    //プレイヤー向きから、エネミーの方向へ補間する
    PQ = DirectX::XMQuaternionSlerp(PQ, PEQ, rollSpeed * elapsedTime);

    QuaternionStruct playerQuaternion;
    DirectX::XMStoreFloat4(&playerQuaternion.dxFloat4, PQ);

    playerObj->transform_->SetRotation(playerQuaternion);

    return false;
}

//着地時攻撃処理
void AttackPlayer::AttackOnGround()
{
    isSquareAttack_ = true;
}

//ジャンプ時攻撃処理
void AttackPlayer::AttackJump()
{
    isSquareAttack_ = true;
}

//強制的に攻撃を終わらせる（ジャンプ時等）
void AttackPlayer::AttackFlagEnd()
{
    jumpAttackComboWaitTimer_ = 0;
    comboSquareCount_ = 0;
    comboTriangleCount_ = 0;
    isComboJudge_ = true;
    player_.lock()->GetMovePlayer()->SetIsInputMove(true);
    player_.lock()->GetMovePlayer()->SetIsInputTurn(true);
    player_.lock()->GetMovePlayer()->SetIsDash(true);
    ResetState();
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetAnimationSpeedOffset(1);

    if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::ATTACK_DASH ||
        player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::DASH)
    {
        Graphics::Instance().SetWorldSpeed(1.0f);
    }
}
