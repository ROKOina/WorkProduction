#include "PlayerCom.h"
#include "Input/Input.h"
#include <imgui.h>
#include "Components\CameraCom.h"
#include "Components\TransformCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"

#include "../Enemy/EnemyCom.h"
#include "../Weapon\WeaponCom.h"
#include "../CharacterStatusCom.h"

// 開始処理
void PlayerCom::Start()
{
    //カメラをプレイヤーにフォーカスする
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");

    //プレイヤーのワールドポジションを取得
    DirectX::XMFLOAT3 wp = GetGameObject()->transform_->GetWorldPosition();
    wp.z -= 10;
    wp.y += 6;
    cameraObj->transform_->SetLocalPosition(wp);

    //当たり大きさ
    GetGameObject()->GetComponent<CapsuleColliderCom>()->SetRadius(0.17f);

    //武器ステータス初期化
    std::shared_ptr<WeaponCom> weapon = GetGameObject()->GetChildFind("Candy")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_RIGHT, 1, 30,0.7f,0.3f);
    weapon->SetAttackStatus(BIGSWORD_UP, 1, 30, 0.2f, 0.8f);
    weapon->SetAttackStatus(BIGSWORD_COM1_01, 1, 15, 0.8f, 0.2f, 1.5f);
    weapon->SetAttackStatus(BIGSWORD_COM1_02, 1, 15, 0.3f, 0.7f);
    weapon->SetAttackStatus(BIGSWORD_COM1_03, 1, 15, 0.9f, 0.1f, 2.0f);
    weapon->SetAttackStatus(BIGSWORD_DASH, 1, 100, 0.9f, 0.1f);
    //空中攻撃
    {
        weapon->SetAttackStatus(JUMP_ATTACK_UPPER, 1, 25, 0.0f, 1.0f, 1.5f, ATTACK_SPECIAL_TYPE::JUMP_START | ATTACK_SPECIAL_TYPE::UNSTOP);
        weapon->SetAttackStatus(JUMP_ATTACK_01, 1, 0, 0.0f, 0, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);
        weapon->SetAttackStatus(JUMP_ATTACK_02, 1, 0, 0.0f, 0, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);
        weapon->SetAttackStatus(JUMP_ATTACK_03, 1, 0, 0.0f, 0, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);
    }

    //攻撃管理を初期化
    attackPlayer_ = std::make_shared<AttackPlayer>(GetGameObject()->GetComponent<PlayerCom>());
    //移動管理を初期化
    movePlayer_ = std::make_shared<MovePlayer>(GetGameObject()->GetComponent<PlayerCom>());
    //ジャスト回避管理を初期化
    justAvoidPlayer_ = std::make_shared<JustAvoidPlayer>(GetGameObject()->GetComponent<PlayerCom>());

    //アニメーション初期化
    AnimationInitialize();

}

// 更新処理
void PlayerCom::Update(float elapsedTime)
{
    //移動
    movePlayer_->Update(elapsedTime);

    //ジャスト回避
    justAvoidPlayer_->Update(elapsedTime);

    //攻撃
    attackPlayer_->Update(elapsedTime);

    //カプセル当たり判定設定
    {
        std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
        //足元から頭までカプセルを設定
        capsule->SetPosition1({ 0,0,0 });
        Model::Node* headNode = GetGameObject()->GetComponent<RendererCom>()->GetModel()->FindNode("Head");
        DirectX::XMMATRIX PWorld = DirectX::XMLoadFloat4x4(&headNode->parent->worldTransform);
        DirectX::XMFLOAT3 pos;
        DirectX::XMStoreFloat3(&pos, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&headNode->translate), PWorld));
        DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetWorldPosition();
        capsule->SetPosition2({ pos.x - playerPos.x,pos.y - playerPos.y,pos.z - playerPos.z });
    }

    //壁判定
    {
        DirectX::XMFLOAT3 kabePlus = GameObjectManager::Instance().Find("kabePlus")->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 kabeMinas = GameObjectManager::Instance().Find("kabeMinas")->transform_->GetWorldPosition();

        DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
        if (pos.x > kabePlus.x)
            pos.x = kabePlus.x;
        if (pos.x < kabeMinas.x)
            pos.x = kabeMinas.x;
        if (pos.z > kabePlus.z)
            pos.z = kabePlus.z;
        if (pos.z < kabeMinas.z)
            pos.z = kabeMinas.z;

        GetGameObject()->transform_->SetWorldPosition(pos);
    }

    ////自分と敵の押し返し
    //std::vector<HitObj> hitGameObj = GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
    //for (auto& hitObj : hitGameObj)
    //{
    //    //タグ検索
    //    //敵との当たり（仮）
    //    if (COLLIDER_TAG::Enemy == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
    //    {
    //        //押し返し
    //        DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetWorldPosition();
    //        DirectX::XMFLOAT3 hitPos = hitObj.gameObject->transform_->GetWorldPosition();

    //        DirectX::XMVECTOR PlayerPos = { playerPos.x,0,playerPos.z };
    //        DirectX::XMVECTOR HitPos = { hitPos.x,0, hitPos.z };

    //        DirectX::XMVECTOR ForceNormal = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(HitPos, PlayerPos));
    //        ForceNormal = DirectX::XMVectorScale(ForceNormal, 4);

    //        DirectX::XMFLOAT3 power;
    //        DirectX::XMStoreFloat3(&power, ForceNormal);

    //        hitObj.gameObject->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);
    //    }
    //}

}

// GUI描画
void PlayerCom::OnGUI()
{
    int currentAnimIndex = GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    ImGui::InputInt("currentAnimIndex", &currentAnimIndex);

    //入力情報を取得
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();
    ImGui::DragFloat("x", &ax);
    ImGui::DragFloat("y", &ay);

    int stats = (int)playerStatus_;
    ImGui::InputInt("status", &stats);

    if(ImGui::TreeNode("Attack"))
    {
        attackPlayer_->OnGui();
        ImGui::TreePop();
    }
    if(ImGui::TreeNode("JustAvo"))
    {
        justAvoidPlayer_->OnGui();
        ImGui::TreePop();
    }
    if(ImGui::TreeNode("Move"))
    {
        movePlayer_->OnGui();
        ImGui::TreePop();
    }
}


//アニメーション初期化設定
void PlayerCom::AnimationInitialize()
{
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //初期のアニメーション
    animator->SetFirstTransition(ANIMATION_PLAYER::IDEL_2);
    animator->SetLoopAnimation(ANIMATION_PLAYER::IDEL_2, true);

    //アニメーションパラメーター追加
    animator->AddTriggerParameter("idle");
    animator->AddTriggerParameter("jump");
    animator->AddTriggerParameter("jumpFall");
    animator->AddTriggerParameter("punch");
    animator->AddTriggerParameter("dash");
    animator->AddTriggerParameter("dashBack");
    animator->AddTriggerParameter("runTurn");
    animator->AddTriggerParameter("runStop");

    animator->AddTriggerParameter("square");    //□
    animator->AddTriggerParameter("triangle");  //△
    //入力無し
    animator->AddTriggerParameter("squareIdle");
    animator->AddTriggerParameter("squareDash");
    animator->AddTriggerParameter("squareJump");

    animator->AddTriggerParameter("triangleIdle");
    animator->AddTriggerParameter("triangleDash");
    animator->AddTriggerParameter("triangleJump");
    animator->AddTriggerParameter("triangleJumpDown");

    animator->AddTriggerParameter("jumpAttackEnd");

    //dashコンボの待機
    animator->AddTriggerParameter("dashComboWait");

    //ジャスト時
    animator->AddTriggerParameter("squareJust");
    animator->AddTriggerParameter("triangleJust");
    //ジャスト回避
    animator->AddTriggerParameter("justBack");
    animator->AddTriggerParameter("justFront");
    animator->AddTriggerParameter("justLeft");
    animator->AddTriggerParameter("justRight");

    animator->AddFloatParameter("moveSpeed");

    //アニメーション遷移とパラメーター設定をする決める
    {
        //idle
        animator->AddAnimatorTransition(IDEL_2, WALK_RUNRUN_2, false, 0.5f);
        animator->SetFloatTransition(IDEL_2, WALK_RUNRUN_2,
            "moveSpeed", 0.1f, PATAMETER_JUDGE::GREATER);

        //walk
        animator->AddAnimatorTransition(WALK_RUNRUN_2, IDEL_2);
        animator->SetFloatTransition(WALK_RUNRUN_2, IDEL_2,
            "moveSpeed", 0.1f, PATAMETER_JUDGE::LESS);
        animator->SetLoopAnimation(WALK_RUNRUN_2, true);

        animator->AddAnimatorTransition(WALK_RUNRUN_2, RUN_HARD_2);
        animator->SetFloatTransition(WALK_RUNRUN_2, RUN_HARD_2,
            "moveSpeed", movePlayer_->GetMoveParam(MovePlayer::MOVE_PARAM::WALK).moveMaxSpeed + 1, PATAMETER_JUDGE::GREATER);

        //run
        animator->AddAnimatorTransition(RUN_HARD_2, WALK_RUNRUN_2);
        animator->SetFloatTransition(RUN_HARD_2, WALK_RUNRUN_2,
            "moveSpeed", movePlayer_->GetMoveParam(MovePlayer::MOVE_PARAM::WALK).moveMaxSpeed + 1, PATAMETER_JUDGE::LESS);
        animator->SetLoopAnimation(RUN_HARD_2, true);

        //run切り替えし
        animator->AddAnimatorTransition(RUN_HARD_2, RUN_TURN_FORWARD, false, 0.0f);
        animator->SetTriggerTransition(RUN_HARD_2, RUN_TURN_FORWARD, "runTurn");
        animator->AddAnimatorTransition(RUN_TURN_FORWARD, RUN_HARD_2, true);

        //run止まり
        animator->AddAnimatorTransition(RUN_STOP);
        //animator->AddAnimatorTransition(RUN_HARD_2, RUN_STOP);
        animator->SetTriggerTransition(RUN_STOP, "runStop");
        //animator->SetTriggerTransition(RUN_HARD_2, RUN_STOP, "runStop");
        animator->AddAnimatorTransition(RUN_STOP, IDEL_2, true);



        {   //dashコンボ
            //dash切り
            animator->AddAnimatorTransition(BIGSWORD_DASH);
            animator->SetTriggerTransition(BIGSWORD_DASH, "triangleDash");
            animator->AddAnimatorTransition(BIGSWORD_DASH, IDEL_2, true, 3.5f);

            //コンボ2(攻撃待機
            animator->AddAnimatorTransition(BIGSWORD_DASH_3);
            animator->SetTriggerTransition(BIGSWORD_DASH_3, "dashComboWait");
            animator->AddAnimatorTransition(BIGSWORD_DASH_3, IDEL_2, true);


            //コンボ3
            animator->AddAnimatorTransition(BIGSWORD_DASH_3, BIGSWORD_COM1_01);
            animator->SetTriggerTransition(BIGSWORD_DASH_3, BIGSWORD_COM1_01, "square");
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, IDEL_2, true, 3.5f);


        }

        //どこからでも遷移する

        //jump
        animator->AddAnimatorTransition(JUMP_IN);
        animator->SetTriggerTransition(JUMP_IN, "jump");
        animator->AddAnimatorTransition(JUMP_IN, IDEL_2);
        animator->SetTriggerTransition(JUMP_IN, IDEL_2, "idle");

        //jumpFall
        animator->AddAnimatorTransition(JUMP_FALL, false, 1.0f);
        animator->SetTriggerTransition(JUMP_FALL, "jumpFall");
        animator->AddAnimatorTransition(JUMP_FALL, IDEL_2);
        animator->SetTriggerTransition(JUMP_FALL, IDEL_2, "idle");
        animator->SetLoopAnimation(JUMP_FALL, true);

        //punch
        animator->AddAnimatorTransition(PUNCH);
        animator->SetTriggerTransition(PUNCH, "punch");
        animator->AddAnimatorTransition(PUNCH, IDEL_2, true);

        //dash
        animator->AddAnimatorTransition(DASH_ANIM);
        animator->SetTriggerTransition(DASH_ANIM, "dash");
        animator->AddAnimatorTransition(DASH_ANIM, IDEL_2, true);

        //dash_back
        animator->AddAnimatorTransition(DASH_BACK);
        animator->SetTriggerTransition(DASH_BACK, "dashBack");
        animator->AddAnimatorTransition(DASH_BACK, IDEL_2);
        animator->SetTriggerTransition(DASH_BACK, IDEL_2, "idle");

        //ジャスト回避回避
        //back
        animator->AddAnimatorTransition(DODGE_BACK);
        animator->SetTriggerTransition(DODGE_BACK, "justBack");
        animator->AddAnimatorTransition(DODGE_BACK, IDEL_2, true);
        //front
        animator->AddAnimatorTransition(DODGE_FRONT);
        animator->SetTriggerTransition(DODGE_FRONT, "justFront");
        animator->AddAnimatorTransition(DODGE_FRONT, IDEL_2, true);
        //left
        animator->AddAnimatorTransition(DODGE_LEFT);
        animator->SetTriggerTransition(DODGE_LEFT, "justLeft");
        animator->AddAnimatorTransition(DODGE_LEFT, IDEL_2, true);
        //right
        animator->AddAnimatorTransition(DODGE_RIGHT);
        animator->SetTriggerTransition(DODGE_RIGHT, "justRight");
        animator->AddAnimatorTransition(DODGE_RIGHT, IDEL_2, true);

        {   //□□□
            //combo01
            animator->AddAnimatorTransition(BIGSWORD_COM1_01);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, "squareIdle");
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, IDEL_2, true, 3.5f);
            //combo2に行く
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, BIGSWORD_COM1_02);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, BIGSWORD_COM1_02, "square");

            //combo02
            animator->AddAnimatorTransition(BIGSWORD_COM1_02);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, "squareJust");
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, IDEL_2, true, 3.5f);
            //combo3に行く
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, BIGSWORD_COM1_03);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, BIGSWORD_COM1_03, "square");

            //combo03
            animator->AddAnimatorTransition(BIGSWORD_COM1_03);
            //animator->SetTriggerTransition(BIGSWORD_COM1_03, "squareJust");
            animator->AddAnimatorTransition(BIGSWORD_COM1_03, IDEL_2, true, 3.5f);
        }

        //ジャンプコンボ
        {   //J△□□
            //combo01
            animator->AddAnimatorTransition(JUMP_ATTACK_UPPER);
            animator->SetTriggerTransition(JUMP_ATTACK_UPPER, "triangleJump");
            animator->AddAnimatorTransition(JUMP_ATTACK_UPPER, IDEL_2, true, 3.5f);

            //combo02
            animator->AddAnimatorTransition(JUMP_ATTACK_UPPER, JUMP_ATTACK_01);
            animator->SetTriggerTransition(JUMP_ATTACK_UPPER, JUMP_ATTACK_01, "square");
        }

        //ジャンプ中下強攻撃
        {   //J△
            animator->AddAnimatorTransition(JUMP_ATTACK_DOWN_START);
            animator->SetTriggerTransition(JUMP_ATTACK_DOWN_START, "triangleJumpDown");
            animator->AddAnimatorTransition(JUMP_ATTACK_DOWN_START, JUMP_ATTACK_DOWN_DO, true);

            animator->SetLoopAnimation(JUMP_ATTACK_DOWN_DO, true);

            animator->AddAnimatorTransition(JUMP_ATTACK_DOWN_DO, JUMP_ATTACK_DOWN_END);
            animator->SetTriggerTransition(JUMP_ATTACK_DOWN_DO, JUMP_ATTACK_DOWN_END, "idle");

            animator->AddAnimatorTransition(JUMP_ATTACK_DOWN_END, IDEL_2, true);
        }

        //ジャンプ攻撃
        {
            animator->AddAnimatorTransition(JUMP_ATTACK_01);
            animator->SetTriggerTransition(JUMP_ATTACK_01, "squareJump");
            animator->AddAnimatorTransition(JUMP_ATTACK_01, IDEL_2, true);
            animator->SetTriggerTransition(JUMP_ATTACK_01, IDEL_2, "idle");

            animator->AddAnimatorTransition(JUMP_ATTACK_01, JUMP_ATTACK_02);
            animator->SetTriggerTransition(JUMP_ATTACK_01, JUMP_ATTACK_02, "square");
            animator->AddAnimatorTransition(JUMP_ATTACK_02, IDEL_2, true);
            animator->SetTriggerTransition(JUMP_ATTACK_02, IDEL_2, "idle");

            animator->AddAnimatorTransition(JUMP_ATTACK_02, JUMP_ATTACK_03);
            animator->SetTriggerTransition(JUMP_ATTACK_02, JUMP_ATTACK_03, "square");
            animator->AddAnimatorTransition(JUMP_ATTACK_03, IDEL_2, true);
            animator->SetTriggerTransition(JUMP_ATTACK_03, IDEL_2, "idle");
        }


    }

}
