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

#include "Graphics\Shaders\PostEffect.h"

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
    //Candy
    std::shared_ptr<WeaponCom> weapon = GetGameObject()->GetChildFind("Candy")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_COM1_01, 5, 15, 0.8f, 0.2f, 1.5f);
    weapon->SetAttackStatus(BIGSWORD_DASH, 2, 100, 0.9f, 0.1f);
    //�W�����v�U��
    weapon->SetAttackStatus(JUMP_ATTACK_UPPER, 4, 20, 0.0f, 1.0f, 1.5f, ATTACK_SPECIAL_TYPE::JUMP_START | ATTACK_SPECIAL_TYPE::UNSTOP);
    weapon->SetAttackStatus(JUMP_ATTACK_01, 3, 20, 0.0f, 1.0f, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);
    weapon->SetAttackStatus(JUMP_ATTACK_DOWN_DO, 8, 1, 0.0f, -1.0f);

    //CandyCircle
    weapon = GetGameObject()->GetChildFind("CandyCircle")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_COM1_02, 5, 15, 0.3f, 0.7f);
    weapon->SetAttackStatus(BIGSWORD_COM2_02, 3, 10, 1.0f, 0.0f);
    weapon->SetAttackStatus(JUMP_ATTACK_02, 3, 20, 0.0f, 1.0f, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);

    //CandyStick
    weapon = GetGameObject()->GetChildFind("CandyStick")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_COM1_03, 5, 15, 0.9f, 0.1f, 2.0f);
    weapon->SetAttackStatus(JUMP_ATTACK_03, 3, 20, 0.0f, 1.0f, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);

    std::shared_ptr<PlayerCom> myCom = GetGameObject()->GetComponent<PlayerCom>();
    //�U���Ǘ���������
    attackPlayer_ = std::make_shared<AttackPlayer>(myCom);
    //�ړ��Ǘ���������
    movePlayer_ = std::make_shared<MovePlayer>(myCom);
    //�W���X�g����Ǘ���������
    justAvoidPlayer_ = std::make_shared<JustAvoidPlayer>(myCom);

    //�A�j���[�V����������
    AnimationInitialize();

    //�X�e�[�^�X�ݒ�
    std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
    status->SetMaxHP(8);
    status->SetHP(8);

    //UI������
    startUI_ = false;
    hpSprite_[0] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonutsOne.png");
    hpSprite_[1] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts2.png");
    hpSprite_[2] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts3.png");
    hpSprite_[3] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts4.png");
    hpSprite_[4] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts5.png");
    hpSprite_[5] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts6.png");
    hpSprite_[6] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts7.png");
    hpSprite_[7] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts8.png");

    hpDonutsPos_[0] = { 48.3f,3.1f };
    hpDonutsPos_[1] = { 6.0f,44.0f };
    hpDonutsPos_[2] = { 5.9f,107.3f };
    hpDonutsPos_[3] = { 49.0f,146.5f };
    hpDonutsPos_[4] = { 110.9f,145.3f };
    hpDonutsPos_[5] = { 153.9f,101.4f };
    hpDonutsPos_[6] = { 153.9f,42.9f };
    hpDonutsPos_[7] = { 112.2f,7.3f };

}

// �X�V����
void PlayerCom::Update(float elapsedTime)
{
    //�Q�[���J�n�t���O
    if (!GameObjectManager::Instance().GetIsSceneGameStart())
        return;

    std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
    //�_���[�W���ȊO
    if (!status->GetDamageAnimation())
    {
        //�ړ�
        movePlayer_->Update(elapsedTime);

        //�W���X�g���
        justAvoidPlayer_->Update(elapsedTime);

        //�U��
        attackPlayer_->Update(elapsedTime);
    }

    //�_���[�W
    if (status->GetFrameDamage())
    {
        //�A�j���[�^�[
        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
        animator->ResetParameterList();
        animator->SetTriggerOn("damageFront");
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        attackPlayer_->AttackFlagEnd();
        movePlayer_->DashEndFlag(true);
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

    //�ǔ���
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

    //UI�X�V
    {
        if (isHpDirection_)
        {
            hpGravity_ += 20 * elapsedTime;
            hpDir_.x += dirVelo_.x * 200 * elapsedTime;
            hpDir_.y += dirVelo_.y * 200 * elapsedTime + hpGravity_;
            hpDir_.z -= elapsedTime * 2;
            if (hpDir_.z < 0)isHpDirection_ = false;
        }
    }

}

// GUI�`��
void PlayerCom::OnGUI()
{
    int currentAnimIndex = GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    ImGui::InputInt("currentAnimIndex", &currentAnimIndex);

    //���͏����擾
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


void PlayerCom::MaskRender(PostEffect* postEff, std::shared_ptr<CameraCom> maskCamera)
{
    ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();

    //������
    if (!startUI_)
    {
        startUI_ = true;
        std::shared_ptr<FbxModelResource> res = GameObjectManager::Instance().Find("picoMask")->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
        for (auto& shape : res->GetMeshesEdit()[res->GetShapeIndex()].shapeData)
        {
            shape.rate = 0;
        }
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[5].rate = 1;
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[9].rate = 1;
        GameObjectManager::Instance().Find("picoMask")->GetComponent<AnimationCom>()
            ->PlayAnimation(ANIMATION_PLAYER::IDEL_2, true);

    }

    //�M
    saraSprite_->Render(dc, -101.8f, -62.6f, static_cast<float>(saraSprite_->GetTextureWidth()), static_cast<float>(saraSprite_->GetTextureHeight())
        , 0, 0, static_cast<float>(saraSprite_->GetTextureWidth()), static_cast<float>(saraSprite_->GetTextureHeight())
        , 0, 1, 1, 1, 1);

    //HP
    {
        int hpIndex = GetGameObject()->GetComponent<CharacterStatusCom>()->GetHP() - 1;
        if (hpIndex > 0) {
            hpSprite_[hpIndex]->Render(dc, 1.9f, -9.1f, static_cast<float>(hpSprite_[hpIndex]->GetTextureWidth()) * 0.3f, static_cast<float>(hpSprite_[hpIndex]->GetTextureHeight()) * 0.3f
                , 0, 0, static_cast<float>(hpSprite_[hpIndex]->GetTextureWidth()), static_cast<float>(hpSprite_[hpIndex]->GetTextureHeight())
                , 0, 1, 1, 1, 1);
        }
        if (hpIndex >= 0) {
            hpSprite_[0]->Render(dc, hpDonutsPos_[hpIndex].x, hpDonutsPos_[hpIndex].y, static_cast<float>(hpSprite_[0]->GetTextureWidth()) * 0.3f, static_cast<float>(hpSprite_[0]->GetTextureHeight()) * 0.3f
                , 0, 0, static_cast<float>(hpSprite_[0]->GetTextureWidth()), static_cast<float>(hpSprite_[0]->GetTextureHeight())
                , 0, 1, 1, 1, 1);
        }
        //���o
        static int oldHp_ = hpIndex;
        if (oldHp_ != hpIndex)
        {
            isHpDirection_ = true;
            hpDir_.x = hpDonutsPos_[oldHp_].x;
            hpDir_.y = hpDonutsPos_[oldHp_].y;
            hpDir_.z = 1;
            hpGravity_ = 0;
            oldHp_ = hpIndex;

            //�摜�̏d�S�i���̕��ɒu���j���Z�o
            static DirectX::XMVECTOR Point = { 1.9f + hpSprite_[1]->GetTextureWidth() / 2 * 0.3f
            ,-9.1f + hpSprite_[1]->GetTextureHeight() * 0.3f };
            //�d�S����̃x�N�g�����ړ������ɂ���
            DirectX::XMVECTOR HpPoint = { hpDir_.x + hpSprite_[0]->GetTextureWidth() / 2 * 0.3f
                ,hpDir_.y + hpSprite_[0]->GetTextureHeight() / 2 * 0.3f };
            DirectX::XMStoreFloat2(&dirVelo_, DirectX::XMVector2Normalize(DirectX::XMVectorSubtract(HpPoint, Point)));
        }
        if (isHpDirection_)
        {
            hpSprite_[0]->Render(dc, hpDir_.x, hpDir_.y, static_cast<float>(hpSprite_[0]->GetTextureWidth()) * 0.3f, static_cast<float>(hpSprite_[0]->GetTextureHeight()) * 0.3f
                , 0, 0, static_cast<float>(hpSprite_[0]->GetTextureWidth()), static_cast<float>(hpSprite_[0]->GetTextureHeight())
                , 0, 1, 1, 1, hpDir_.z);
        }
    }

    //���C�v�g�O
    faceFrameUI_->Render(dc, 49.5f, 41.4f, static_cast<float>(faceFrameUI_->GetTextureWidth()) * 0.552f, static_cast<float>(faceFrameUI_->GetTextureHeight()) * 0.552f
        , 0, 0, static_cast<float>(faceFrameUI_->GetTextureWidth()), static_cast<float>(faceFrameUI_->GetTextureHeight())
        , 0, 0, 1, 1, 1);

    //���C�v
    {
        //�}�X�N���鑤�`��
        postEff->CacheMaskBuffer(maskCamera);

        //���C�v�w�i
        faceFrameUI_->Render(dc, 49.5f + 9, 41.4f + 9, static_cast<float>(faceFrameUI_->GetTextureWidth()) * 0.48f, static_cast<float>(faceFrameUI_->GetTextureHeight()) * 0.48f
            , 0, 0, static_cast<float>(faceFrameUI_->GetTextureWidth()), static_cast<float>(faceFrameUI_->GetTextureHeight())
            , 0, 1, 1, 1, 1);

        //�}�X�N����鑤�`��
        postEff->StartBeMaskBuffer();

        //�}�X�N�I�u�W�F�`��
        GameObjectManager::Instance().RenderMask();


        //�}�X�N�����I������
        postEff->RestoreMaskBuffer({ -154 ,-72 }, { 0.3f,0.3f });

        postEff->DrawMask();
    }
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
    //�g���K�[�Ŕ���
    {
        animator->AddTriggerParameter("idle");
        animator->AddTriggerParameter("jump");
        animator->AddTriggerParameter("jumpFall");
        animator->AddTriggerParameter("dash");
        animator->AddTriggerParameter("dashBack");
        animator->AddTriggerParameter("runTurn");
        animator->AddTriggerParameter("runStop");

        animator->AddTriggerParameter("damageFront");

        animator->AddTriggerParameter("square");    //��
        animator->AddTriggerParameter("squareIdle");
        animator->AddTriggerParameter("squareDash");
        animator->AddTriggerParameter("squareJump");

        animator->AddTriggerParameter("triangle");  //��
        animator->AddTriggerParameter("triangleDash");
        animator->AddTriggerParameter("triangleJump");
        animator->AddTriggerParameter("triangleJumpDown");

        //�W���X�g��
        animator->AddTriggerParameter("squareJust");
        animator->AddTriggerParameter("triangleJust");
        //�W���X�g���
        animator->AddTriggerParameter("justBack");
        animator->AddTriggerParameter("justFront");
        animator->AddTriggerParameter("justLeft");
        animator->AddTriggerParameter("justRight");
    }

    //�t���[�g�Ŕ���
    {
        animator->AddFloatParameter("moveSpeed");
    }

    //�A�j���[�V�����J�ڂƃp�����[�^�[�ݒ�����߂�
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

        //run�؂�ւ���
        animator->AddAnimatorTransition(RUN_HARD_2, RUN_TURN_FORWARD, false, 0.0f);
        animator->SetTriggerTransition(RUN_HARD_2, RUN_TURN_FORWARD, "runTurn");
        animator->AddAnimatorTransition(RUN_TURN_FORWARD, RUN_HARD_2, true);

        //run�~�܂�
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

        //�W���X�g������
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

        {   //�������R���{
            //�R���{2
            animator->AddAnimatorTransition(BIGSWORD_DASH);
            animator->SetTriggerTransition(BIGSWORD_DASH, "triangleDash");
            animator->AddAnimatorTransition(BIGSWORD_DASH, IDEL_2, true, 3.5f);

            //�R���{3
            animator->AddAnimatorTransition(BIGSWORD_DASH, BIGSWORD_COM2_02);
            animator->SetTriggerTransition(BIGSWORD_DASH, BIGSWORD_COM2_02, "square");
            animator->AddAnimatorTransition(BIGSWORD_COM2_02, IDEL_2, true, 3.5f);
        }

        //�U��
        {  
            //��
            //combo01
            animator->AddAnimatorTransition(BIGSWORD_COM1_01);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, "squareIdle");
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, IDEL_2, true, 3.5f);

            //combo2
            //�� (�O���j
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, BIGSWORD_COM1_02);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, BIGSWORD_COM1_02, "square");
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, IDEL_2, true, 3.5f);

            //�� (�O���j
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_01, BIGSWORD_COM1_02);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_01, BIGSWORD_COM1_02, "square");

            //��
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, TRIANGLE_ATTACK_01, false, 1.0f);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, TRIANGLE_ATTACK_01, "triangle");
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_01, IDEL_2, true, 3.5f);

            //combo3
            //�� (�O��)
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, BIGSWORD_COM1_03);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, BIGSWORD_COM1_03, "square");
            animator->AddAnimatorTransition(BIGSWORD_COM1_03, IDEL_2, true, 3.5f);

            //�� (�O��)
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_02, BIGSWORD_COM1_03);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_02, BIGSWORD_COM1_03, "square");

            //�� (�O��)
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, TRIANGLE_ATTACK_02, false, 1.0f);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, TRIANGLE_ATTACK_02, "triangle");
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_02, IDEL_2, true, 3.5f);

            //�� (�O��)
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_01, TRIANGLE_ATTACK_02, false, 1.0f);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_01, TRIANGLE_ATTACK_02, "triangle");

            //�Ōぢ
            //�� (�O��)
            animator->AddAnimatorTransition(BIGSWORD_COM1_03, TRIANGLE_ATTACK_03);
            animator->SetTriggerTransition(BIGSWORD_COM1_03, TRIANGLE_ATTACK_03, "triangle");
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_03, IDEL_2, true, 3.5f);

            //�� (�O��)
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_02, TRIANGLE_ATTACK_03);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_02, TRIANGLE_ATTACK_03, "triangle");


            //�W���X�g�������
            animator->AddAnimatorTransition(BIGSWORD_COM1_02);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, "squareJust");

            //�W���X�g�������
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_PUSH);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_PUSH, "triangleJust");
        }

        //�W�����v�R���{
        {   //J������
            //combo01
            animator->AddAnimatorTransition(JUMP_ATTACK_UPPER);
            animator->SetTriggerTransition(JUMP_ATTACK_UPPER, "triangleJump");
            animator->AddAnimatorTransition(JUMP_ATTACK_UPPER, IDEL_2, true, 3.5f);

            //combo02
            animator->AddAnimatorTransition(JUMP_ATTACK_UPPER, JUMP_ATTACK_01);
            animator->SetTriggerTransition(JUMP_ATTACK_UPPER, JUMP_ATTACK_01, "square");
        }

        //�W�����v���A�����U��
        {   //J��
            animator->AddAnimatorTransition(JUMP_ATTACK_DOWN_START);
            animator->SetTriggerTransition(JUMP_ATTACK_DOWN_START, "triangleJumpDown");
            animator->AddAnimatorTransition(JUMP_ATTACK_DOWN_START, JUMP_ATTACK_DOWN_DO, true);

            animator->SetLoopAnimation(JUMP_ATTACK_DOWN_DO, true);

            animator->AddAnimatorTransition(JUMP_ATTACK_DOWN_DO, JUMP_ATTACK_DOWN_END);
            animator->SetTriggerTransition(JUMP_ATTACK_DOWN_DO, JUMP_ATTACK_DOWN_END, "idle");

            animator->AddAnimatorTransition(JUMP_ATTACK_DOWN_END, IDEL_2, true);
        }

        //�W�����v�U��
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

        //�_���[�W
        {
            animator->AddAnimatorTransition(DAMAGE_FRONT);
            animator->SetTriggerTransition(DAMAGE_FRONT, "damageFront");
            animator->AddAnimatorTransition(DAMAGE_FRONT, IDEL_2, true);
        }
    }
}
