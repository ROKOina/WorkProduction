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
    //Candy
    std::shared_ptr<WeaponCom> weapon = GetGameObject()->GetChildFind("Candy")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_COM1_01, 1, 15, 0.8f, 0.2f, 1.5f);
    weapon->SetAttackStatus(BIGSWORD_DASH, 1, 100, 0.9f, 0.1f);
    //ジャンプ攻撃
    weapon->SetAttackStatus(JUMP_ATTACK_UPPER, 1, 20, 0.0f, 1.0f, 1.5f, ATTACK_SPECIAL_TYPE::JUMP_START | ATTACK_SPECIAL_TYPE::UNSTOP);
    weapon->SetAttackStatus(JUMP_ATTACK_01, 1, 20, 0.0f, 1.0f, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);
    weapon->SetAttackStatus(JUMP_ATTACK_DOWN_DO, 1, 1, 0.0f, -1.0f);

    //CandyCircle
    weapon = GetGameObject()->GetChildFind("CandyCircle")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_COM1_02, 1, 15, 0.3f, 0.7f);
    weapon->SetAttackStatus(BIGSWORD_COM2_02, 1, 10, 1.0f, 0.0f);
    weapon->SetAttackStatus(JUMP_ATTACK_02, 1, 20, 0.0f, 1.0f, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);

    //CandyStick
    weapon = GetGameObject()->GetChildFind("CandyStick")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_COM1_03, 1, 15, 0.9f, 0.1f, 2.0f);
    weapon->SetAttackStatus(JUMP_ATTACK_03, 1, 20, 0.0f, 1.0f, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);

    std::shared_ptr<PlayerCom> myCom = GetGameObject()->GetComponent<PlayerCom>();
    //攻撃管理を初期化
    attackPlayer_ = std::make_shared<AttackPlayer>(myCom);
    //移動管理を初期化
    movePlayer_ = std::make_shared<MovePlayer>(myCom);
    //ジャスト回避管理を初期化
    justAvoidPlayer_ = std::make_shared<JustAvoidPlayer>(myCom);

    //アニメーション初期化
    AnimationInitialize();

    //ステータス設定
    std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
    status->SetMaxHP(10);
    status->SetHP(10);

    maskSprite_ = std::make_unique<Sprite>("./Data/Sprite/rect.png");
}

// 更新処理
void PlayerCom::Update(float elapsedTime)
{
    std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
    //ダメージ時以外
    if (!status->GetDamageAnimation())
    {
        //移動
        movePlayer_->Update(elapsedTime);

        //ジャスト回避
        justAvoidPlayer_->Update(elapsedTime);

        //攻撃
        attackPlayer_->Update(elapsedTime);
    }

    //ダメージ
    if (status->GetFrameDamage())
    {
        //アニメーター
        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
        animator->ResetParameterList();
        animator->SetTriggerOn("damageFront");
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        attackPlayer_->AttackFlagEnd();
        movePlayer_->DashEndFlag(true);
    }

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

void PlayerCom::Render2D(float elapsedTime)
{
    ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
    ////hp
    //{
    //    faceFrameUI_->Render(dc, 10, 10, faceFrameUI_->GetTextureWidth() * 1.8f, faceFrameUI_->GetTextureHeight() * 0.2f
    //        , 0, 0, faceFrameUI_->GetTextureWidth(), faceFrameUI_->GetTextureHeight()
    //        , 0, 0.0f, 0.5f, 0.9f, 1);
    //    std::shared_ptr<CharacterStatusCom> stats = GetGameObject()->GetComponent<CharacterStatusCom>();
    //    float hpRatio = float(stats->GetHP()) / float(stats->GetMaxHP());
    //    faceFrameUI_->Render(dc, 22.5f, 22.5f, faceFrameUI_->GetTextureWidth() * 1.7f * hpRatio
    //        , faceFrameUI_->GetTextureHeight() * 0.1f
    //        , 0, 0, faceFrameUI_->GetTextureWidth(), faceFrameUI_->GetTextureHeight()
    //        , 0, 1, 0.9f, 0, 1);
    //}
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
    //トリガーで判定
    {
        animator->AddTriggerParameter("idle");
        animator->AddTriggerParameter("jump");
        animator->AddTriggerParameter("jumpFall");
        animator->AddTriggerParameter("dash");
        animator->AddTriggerParameter("dashBack");
        animator->AddTriggerParameter("runTurn");
        animator->AddTriggerParameter("runStop");

        animator->AddTriggerParameter("damageFront");

        animator->AddTriggerParameter("square");    //□
        animator->AddTriggerParameter("squareIdle");
        animator->AddTriggerParameter("squareDash");
        animator->AddTriggerParameter("squareJump");

        animator->AddTriggerParameter("triangle");  //△
        animator->AddTriggerParameter("triangleDash");
        animator->AddTriggerParameter("triangleJump");
        animator->AddTriggerParameter("triangleJumpDown");

        //ジャスト時
        animator->AddTriggerParameter("squareJust");
        animator->AddTriggerParameter("triangleJust");
        //ジャスト回避
        animator->AddTriggerParameter("justBack");
        animator->AddTriggerParameter("justFront");
        animator->AddTriggerParameter("justLeft");
        animator->AddTriggerParameter("justRight");
    }

    //フロートで判定
    {
        animator->AddFloatParameter("moveSpeed");
    }

    //アニメーション遷移とパラメーター設定を決める
    {
        //idle
        animator->AddAnimatorTransition(IDEL_2, WALK_RUNRUN_2, false, 0.5f);
        animator->SetFloatTransition(IDEL_2, WALK_RUNRUN_2,
            "moveSpeed", 0.1f, PARAMETER_JUDGE::GREATER);

        //walk
        animator->AddAnimatorTransition(WALK_RUNRUN_2, IDEL_2);
        animator->SetFloatTransition(WALK_RUNRUN_2, IDEL_2,
            "moveSpeed", 0.1f, PARAMETER_JUDGE::LESS);
        animator->SetLoopAnimation(WALK_RUNRUN_2, true);

        animator->AddAnimatorTransition(WALK_RUNRUN_2, RUN_HARD_2);
        animator->SetFloatTransition(WALK_RUNRUN_2, RUN_HARD_2,
            "moveSpeed", movePlayer_->GetMoveParam(MovePlayer::MOVE_PARAM::WALK).moveMaxSpeed + 1, PARAMETER_JUDGE::GREATER);

        //run
        animator->AddAnimatorTransition(RUN_HARD_2, WALK_RUNRUN_2);
        animator->SetFloatTransition(RUN_HARD_2, WALK_RUNRUN_2,
            "moveSpeed", movePlayer_->GetMoveParam(MovePlayer::MOVE_PARAM::WALK).moveMaxSpeed + 1, PARAMETER_JUDGE::LESS);
        animator->SetLoopAnimation(RUN_HARD_2, true);

        //run切り替えし
        animator->AddAnimatorTransition(RUN_HARD_2, RUN_TURN_FORWARD, false, 0.0f);
        animator->SetTriggerTransition(RUN_HARD_2, RUN_TURN_FORWARD, "runTurn");
        animator->AddAnimatorTransition(RUN_TURN_FORWARD, RUN_HARD_2, true);

        //run止まり
        animator->AddAnimatorTransition(RUN_STOP);
        animator->SetTriggerTransition(RUN_STOP, "runStop");
        animator->AddAnimatorTransition(RUN_STOP, IDEL_2, true);

        //jump
        animator->AddAnimatorTransition(JUMP_IN);
        animator->SetTriggerTransition(JUMP_IN, "jump");
        animator->AddAnimatorTransition(JUMP_IN, IDEL_2);
        animator->SetTriggerTransition(JUMP_IN, IDEL_2, "idle");

        //jumpFall
        animator->AddAnimatorTransition(JUMP_FALL, false, 1.0f);
        animator->SetTriggerTransition(JUMP_FALL, "jumpFall");
        animator->AddAnimatorTransition(JUMP_FALL, IDEL_2, false, 0.5f);
        animator->SetTriggerTransition(JUMP_FALL, IDEL_2, "idle");
        animator->SetLoopAnimation(JUMP_FALL, true);

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
        {
            //back
            animator->AddAnimatorTransition(DODGE_BACK);
            animator->SetTriggerTransition(DODGE_BACK, "justBack");
            animator->AddAnimatorTransition(DODGE_BACK, IDEL_2, true);
            //front
            animator->AddAnimatorTransition(DODGE_FRONT);
            animator->SetTriggerTransition(DODGE_FRONT, "justFront");
            animator->AddAnimatorTransition(DODGE_FRONT, IDEL_2, true,0.5f);
            //left
            animator->AddAnimatorTransition(DODGE_LEFT);
            animator->SetTriggerTransition(DODGE_LEFT, "justLeft");
            animator->AddAnimatorTransition(DODGE_LEFT, IDEL_2, true);
            //right
            animator->AddAnimatorTransition(DODGE_RIGHT);
            animator->SetTriggerTransition(DODGE_RIGHT, "justRight");
            animator->AddAnimatorTransition(DODGE_RIGHT, IDEL_2, true);
        }

        {   //中距離コンボ
            //コンボ2
            animator->AddAnimatorTransition(BIGSWORD_DASH);
            animator->SetTriggerTransition(BIGSWORD_DASH, "triangleDash");
            animator->AddAnimatorTransition(BIGSWORD_DASH, IDEL_2, true, 3.5f);

            //コンボ3
            animator->AddAnimatorTransition(BIGSWORD_DASH, BIGSWORD_COM2_02);
            animator->SetTriggerTransition(BIGSWORD_DASH, BIGSWORD_COM2_02, "square");
            animator->AddAnimatorTransition(BIGSWORD_COM2_02, IDEL_2, true, 3.5f);
        }

        //攻撃
        {  
            //□
            //combo01
            animator->AddAnimatorTransition(BIGSWORD_COM1_01);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, "squareIdle");
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, IDEL_2, true, 3.5f);

            //combo2
            //□ (前□）
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, BIGSWORD_COM1_02);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, BIGSWORD_COM1_02, "square");
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, IDEL_2, true, 3.5f);

            //□ (前△）
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_01, BIGSWORD_COM1_02);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_01, BIGSWORD_COM1_02, "square");

            //△
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, TRIANGLE_ATTACK_01, false, 1.0f);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, TRIANGLE_ATTACK_01, "triangle");
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_01, IDEL_2, true, 3.5f);

            //combo3
            //□ (前□)
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, BIGSWORD_COM1_03);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, BIGSWORD_COM1_03, "square");
            animator->AddAnimatorTransition(BIGSWORD_COM1_03, IDEL_2, true, 3.5f);

            //□ (前△)
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_02, BIGSWORD_COM1_03);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_02, BIGSWORD_COM1_03, "square");

            //△ (前□)
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, TRIANGLE_ATTACK_02, false, 1.0f);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, TRIANGLE_ATTACK_02, "triangle");
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_02, IDEL_2, true, 3.5f);

            //△ (前△)
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_01, TRIANGLE_ATTACK_02, false, 1.0f);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_01, TRIANGLE_ATTACK_02, "triangle");

            //最後△
            //△ (前□)
            animator->AddAnimatorTransition(BIGSWORD_COM1_03, TRIANGLE_ATTACK_03);
            animator->SetTriggerTransition(BIGSWORD_COM1_03, TRIANGLE_ATTACK_03, "triangle");
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_03, IDEL_2, true, 3.5f);

            //△ (前△)
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_02, TRIANGLE_ATTACK_03);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_02, TRIANGLE_ATTACK_03, "triangle");


            //ジャスト回避時□
            animator->AddAnimatorTransition(BIGSWORD_COM1_02);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, "squareJust");

            //ジャスト回避時△
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_PUSH);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_PUSH, "triangleJust");
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

        //ジャンプ中、下強攻撃
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
            animator->AddAnimatorTransition(JUMP_ATTACK_01, IDEL_2);
            animator->SetTriggerTransition(JUMP_ATTACK_01, IDEL_2, "idle");

            animator->AddAnimatorTransition(JUMP_ATTACK_01, JUMP_ATTACK_02);
            animator->SetTriggerTransition(JUMP_ATTACK_01, JUMP_ATTACK_02, "square");
            animator->AddAnimatorTransition(JUMP_ATTACK_02, IDEL_2);
            animator->SetTriggerTransition(JUMP_ATTACK_02, IDEL_2, "idle");

            animator->AddAnimatorTransition(JUMP_ATTACK_02, JUMP_ATTACK_03);
            animator->SetTriggerTransition(JUMP_ATTACK_02, JUMP_ATTACK_03, "square");
            animator->AddAnimatorTransition(JUMP_ATTACK_03, IDEL_2);
            animator->SetTriggerTransition(JUMP_ATTACK_03, IDEL_2, "idle");
        }

        //ダメージ
        {
            animator->AddAnimatorTransition(DAMAGE_FRONT);
            animator->SetTriggerTransition(DAMAGE_FRONT, "damageFront");
            animator->AddAnimatorTransition(DAMAGE_FRONT, IDEL_2, true);
        }
    }
}
