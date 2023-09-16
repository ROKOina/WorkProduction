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

//�A�j���[�V�������X�g
enum ANIMATION_PLAYER
{
    WALK_RUNRUN_1,
    IDEL_1,
    IDEL_2,
    JUMP_1,
    JUMP_2,
    RUN_HARD_1,
    RUN_HARD_2,
    RUN_SOFT_1,
    RUN_SOFT_2,
    WALK_RUNRUN_2,
    PUNCH,
    BIGSWORD_UP,
    BIGSWORD_LEFT,
    BIGSWORD_RIGHT,
    BIGSWORD_DOWN,
    DASH_ANIM,
    BIGSWORD_COM1_01,
    BIGSWORD_COM1_02,
    BIGSWORD_COM1_03,
    BIGSWORD_COM2_01,
    BIGSWORD_COM2_02,
    BIGSWORD_COM2_03,
    BIGSWORD_DASH,
    JUMP_IN,
    DODGE_BACK,
    DODGE_FRONT,
    DODGE_LEFT,
    DODGE_RIGHT,
};

// �J�n����
void PlayerCom::Start()
{
    //�J�������v���C���[�Ƀt�H�[�J�X����
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");

    //�v���C���[�̃��[���h�|�W�V�������擾
    DirectX::XMFLOAT3 wp = GetGameObject()->transform_->GetWorldPosition();
    wp.z -= 10;
    wp.y += 6;
    cameraObj->transform_->SetLocalPosition(wp);

    //������傫��
    GetGameObject()->GetComponent<CapsuleColliderCom>()->SetRadius(0.17f);

    //����X�e�[�^�X������
    std::shared_ptr<WeaponCom> weapon = GetGameObject()->GetChildFind("greatSword")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_RIGHT, 1, 30,0.7f,0.3f);
    weapon->SetAttackStatus(BIGSWORD_UP, 1, 30, 0.2f, 0.8f);
    weapon->SetAttackStatus(BIGSWORD_COM1_01, 1, 15, 0.8f, 0.2f, 1.5f);
    weapon->SetAttackStatus(BIGSWORD_COM1_02, 1, 15, 0.3f, 0.7f);
    weapon->SetAttackStatus(BIGSWORD_COM1_03, 1, 15, 0.9f, 0.1f, 2.0f);
    weapon->SetAttackStatus(BIGSWORD_DASH, 1, 100, 0.9f, 0.1f);

    //�U���Ǘ���������
    attackPlayer_ = std::make_shared<AttackPlayer>(GetGameObject()->GetComponent<PlayerCom>());
    movePlayer_ = std::make_shared<MovePlayer>(GetGameObject()->GetComponent<PlayerCom>());
    justAvoidPlayer_ = std::make_shared<JustAvoidPlayer>(GetGameObject()->GetComponent<PlayerCom>());

    //�A�j���[�V����������
    AnimationInitialize();

}

// �X�V����
void PlayerCom::Update(float elapsedTime)
{
    //�ړ�
    movePlayer_->Update(elapsedTime);

    //�W���X�g���
    {
        ////�X�V
        //JustAvoidanceAttackUpdate(elapsedTime);
        justAvoidPlayer_->Update(elapsedTime);
    }

    //�U��
    {
        ////�U���X�V
        //AttackUpdate();

        attackPlayer_->Update(elapsedTime);

        ////�����蔻��
        //AttackJudgeCollision();
    }

    //�J�v�Z�������蔻��ݒ�
    {
        std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
        //�������瓪�܂ŃJ�v�Z����ݒ�
        capsule->SetPosition1({ 0,0,0 });
        Model::Node* headNode = GetGameObject()->GetComponent<RendererCom>()->GetModel()->FindNode("Head");
        DirectX::XMMATRIX PWorld = DirectX::XMLoadFloat4x4(&headNode->parent->worldTransform);
        DirectX::XMFLOAT3 pos;
        DirectX::XMStoreFloat3(&pos, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&headNode->translate), PWorld));
        DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetWorldPosition();
        capsule->SetPosition2({ pos.x - playerPos.x,pos.y - playerPos.y,pos.z - playerPos.z });
    }

    //�����ƓG�̉����Ԃ�
    std::vector<HitObj> hitGameObj = GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
    for (auto& hitObj : hitGameObj)
    {
        //�^�O����
        //�G�Ƃ̓�����i���j
        if (COLLIDER_TAG::Enemy == hitObj.gameObject->GetComponent<Collider>()->GetMyTag())
        {
            //�����Ԃ�
            DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetWorldPosition();
            DirectX::XMFLOAT3 hitPos = hitObj.gameObject->transform_->GetWorldPosition();

            DirectX::XMVECTOR PlayerPos = { playerPos.x,0,playerPos.z };
            DirectX::XMVECTOR HitPos = { hitPos.x,0, hitPos.z };

            DirectX::XMVECTOR ForceNormal = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(HitPos, PlayerPos));
            ForceNormal = DirectX::XMVectorScale(ForceNormal, 4);

            DirectX::XMFLOAT3 power;
            DirectX::XMStoreFloat3(&power, ForceNormal);

            hitObj.gameObject->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);
        }
    }

}

int oo = 0;
// GUI�`��
void PlayerCom::OnGUI()
{
    int currentAnimIndex = GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    ImGui::InputInt("currentAnimIndex", &currentAnimIndex);

    //ImGui::InputInt("nowMoveParamType", &moveParamType_);
    //ImGui::SliderInt("moveParamType", &oo, 0, MOVE_PARAM::MAX - 1);
    //ImGui::DragFloat("moveMaxSpeed", &moveParam_[oo].moveMaxSpeed, 0.1f);
    //ImGui::DragFloat("moveSpeed", &moveParam_[oo].moveSpeed, 0.1f);
    //ImGui::DragFloat("moveAcceleration", &moveParam_[oo].moveAcceleration, 0.1f);

    //ImGui::InputInt("state", &dashState_);
    //ImGui::InputInt("comboAttackCount_", &comboAttackCount_);

    //ImGui::DragFloat("jumpSpeed", &jumpSpeed_, 0.1f);

    //ImGui::DragFloat3("up", &up_.x);

    //ImGui::Checkbox("dash", &isDashJudge_);

    //���͏����擾
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();
    ImGui::DragFloat("x", &ax);
    ImGui::DragFloat("y", &ay);

    int stats = (int)playerStatus_;
    ImGui::InputInt("status", &stats);
}







//�A�j���[�V�����������ݒ�
void PlayerCom::AnimationInitialize()
{
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //�����̃A�j���[�V����
    animator->SetFirstTransition(ANIMATION_PLAYER::IDEL_2);
    animator->SetLoopAnimation(ANIMATION_PLAYER::IDEL_2, true);

    //�A�j���[�V�����p�����[�^�[�ǉ�
    animator->AddTriggerParameter("idle");
    animator->AddTriggerParameter("jump");
    animator->AddTriggerParameter("punch");
    animator->AddTriggerParameter("dash");

    animator->AddTriggerParameter("square");    //��
    animator->AddTriggerParameter("triangle");  //��
    //���͖���
    animator->AddTriggerParameter("squareIdle");
    animator->AddTriggerParameter("squareDash");
    animator->AddTriggerParameter("triangleIdle");
    animator->AddTriggerParameter("triangleDash");
    //�W���X�g��
    animator->AddTriggerParameter("squareJust");
    animator->AddTriggerParameter("triangleJust");
    //�W���X�g���
    animator->AddTriggerParameter("justBack");
    animator->AddTriggerParameter("justFront");
    animator->AddTriggerParameter("justLeft");
    animator->AddTriggerParameter("justRight");

    animator->AddFloatParameter("moveSpeed");

    //�A�j���[�V�����J�ڂƃp�����[�^�[�ݒ�����錈�߂�
    {
        //idle
        animator->AddAnimatorTransition(IDEL_2, WALK_RUNRUN_2,false,0.5f);
        animator->SetFloatTransition(IDEL_2, WALK_RUNRUN_2,
            "moveSpeed", 0.1f, PATAMETER_JUDGE::GREATER);

        //walk
        animator->AddAnimatorTransition(WALK_RUNRUN_2, IDEL_2);
        animator->SetFloatTransition(WALK_RUNRUN_2, IDEL_2,
            "moveSpeed", 0.1f, PATAMETER_JUDGE::LESS);
        animator->SetLoopAnimation(WALK_RUNRUN_2, true);

        animator->AddAnimatorTransition(WALK_RUNRUN_2, RUN_HARD_2);
        animator->SetFloatTransition(WALK_RUNRUN_2, RUN_HARD_2,
            "moveSpeed", movePlayer_->moveParam_[MovePlayer::MOVE_PARAM::WALK].moveMaxSpeed + 1, PATAMETER_JUDGE::GREATER);

        //run
        animator->AddAnimatorTransition(RUN_HARD_2, WALK_RUNRUN_2);
        animator->SetFloatTransition(RUN_HARD_2, WALK_RUNRUN_2,
            "moveSpeed", movePlayer_->moveParam_[MovePlayer::MOVE_PARAM::WALK].moveMaxSpeed + 1, PATAMETER_JUDGE::LESS);
        animator->SetLoopAnimation(RUN_HARD_2, true);

        {   //dash�R���{
            //dash�؂�
            animator->AddAnimatorTransition(BIGSWORD_DASH);
            animator->SetTriggerTransition(BIGSWORD_DASH, "triangleDash");
            animator->AddAnimatorTransition(BIGSWORD_DASH, IDEL_2, true, 3.5f);

            //�R���{3
            animator->AddAnimatorTransition(JUMP_IN, BIGSWORD_COM1_01);
            animator->SetTriggerTransition(JUMP_IN, BIGSWORD_COM1_01, "square");
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, IDEL_2, true, 3.5f);


        }

        //�ǂ�����ł��J�ڂ���

        //jump
        animator->AddAnimatorTransition(JUMP_IN);
        animator->SetTriggerTransition(JUMP_IN, "jump");
        animator->AddAnimatorTransition(JUMP_IN, IDEL_2);
        animator->SetTriggerTransition(JUMP_IN, IDEL_2, "idle");

        //punch
        animator->AddAnimatorTransition(PUNCH);
        animator->SetTriggerTransition(PUNCH, "punch");
        animator->AddAnimatorTransition(PUNCH, IDEL_2, true);

        //dash
        animator->AddAnimatorTransition(DASH_ANIM);
        animator->SetTriggerTransition(DASH_ANIM, "dash");
        animator->AddAnimatorTransition(DASH_ANIM, IDEL_2, true);

        //�W���X�g������
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

        {   //������
            //combo01
            animator->AddAnimatorTransition(BIGSWORD_COM1_01);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, "squareIdle");
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, IDEL_2, true, 3.5f);
            //combo2�ɍs��
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, BIGSWORD_COM1_02);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, BIGSWORD_COM1_02, "square");

            //combo02
            animator->AddAnimatorTransition(BIGSWORD_COM1_02);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, "squareJust");
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, IDEL_2, true, 3.5f);
            //combo3�ɍs��
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, BIGSWORD_COM1_03);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, BIGSWORD_COM1_03, "square");

            //combo03
            animator->AddAnimatorTransition(BIGSWORD_COM1_03);
            //animator->SetTriggerTransition(BIGSWORD_COM1_03, "squareJust");
            animator->AddAnimatorTransition(BIGSWORD_COM1_03, IDEL_2, true, 3.5f);
        }

    }

}
