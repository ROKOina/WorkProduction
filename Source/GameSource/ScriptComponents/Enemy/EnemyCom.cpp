#include "EnemyCom.h"
#include "Components\AnimationCom.h"
#include "Components\TransformCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimatorCom.h"
#include "Components\RendererCom.h"
#include "Components\CameraCom.h"
#include "Components\ParticleSystemCom.h"
#include "Components\ParticleComManager.h"

#include "BehaviorTree/JudgmentDerived.h"
#include "BehaviorTree/ActionDerived.h"
#include "GameSource/Math/Mathf.h"

#include "../Player/PlayerCom.h"
#include "../CharacterStatusCom.h"

#include "Graphics\Shaders\PostEffect.h"

#include <imgui.h>


// �J�n����
void EnemyCom::Start()
{
}

// �X�V����
void EnemyCom::Update(float elapsedTime)
{
    //�Q�[���J�n�t���O
    if (!GameObjectManager::Instance().GetIsSceneGameStart())
        return;

    //���S�m�F
    {
        //�X�e�[�^�X�ݒ�
        std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
        if (0 >= status->GetHP())
        {
            GameObjectManager::Instance().Remove(GetGameObject());
        }
    }

    //�����オ�胂�[�V��������
    StandUpUpdate();

    // ���ݎ��s����Ă���m�[�h���������
    if (activeNode_ == nullptr && !isAnimDamage_ && !isJumpDamage_ && !isStandUpMotion_)
    {
        // ���Ɏ��s����m�[�h�𐄘_����B
        activeNode_ = aiTree_->ActiveNodeInference(behaviorData_);
    }
    // ���ݎ��s����m�[�h�������
    if (activeNode_ != nullptr)
    {
         //�r�w�C�r�A�c���[����m�[�h�����s�B
        std::shared_ptr<NodeBase> n = aiTree_->Run(activeNode_, behaviorData_, elapsedTime);
        if (!n)
        {
            //����
            activeNode_.reset();
        }
        else
        {
            if (n->GetId() != activeNode_->GetId())
                activeNode_.swap(n);
        }
    }

    //�d�͐ݒ�
    GravityProcess(elapsedTime);

    //�_���[�W����
    DamageProcess(elapsedTime);

    //�W���X�g���p����o��
    justColliderProcess();

    //�A�j���[�V�����ݒ�
    AnimationSetting();

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
}

// GUI�`��
void EnemyCom::OnGUI()
{
    if (ImGui::Button("delete"))
    {
        GameObjectManager::Instance().Remove(GetGameObject());
    }

    ImGui::DragFloat("walkSpeed", &moveDataEnemy_.walkSpeed);
    ImGui::DragFloat("walkMaxSpeed", &moveDataEnemy_.walkMaxSpeed);
    ImGui::DragFloat("runSpeed", &moveDataEnemy_.runSpeed);
    ImGui::DragFloat("runMaxSpeed", &moveDataEnemy_.runMaxSpeed);

    std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
    DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
    float speed= DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity));
    ImGui::DragFloat("moveSpeed", &speed);

    ImGui::DragFloat2("PP", &sP.x);
    ImGui::DragFloat2("saP", &saP.x,0.01f);
}

void EnemyCom::Render2D(float elapsedTime)
{
    //Graphics& graphics = Graphics::Instance();
    //ID3D11DeviceContext* dc = graphics.GetDeviceContext();

    ////�r���[�|�[�g
    //D3D11_VIEWPORT viewport;
    //UINT numViewports = 1;
    //dc->RSGetViewports(&numViewports, &viewport);

    ////�ϊ��s��
    //DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&graphics.shaderParameter3D_.view);
    //DirectX::XMMATRIX Projection = DirectX::XMLoadFloat4x4(&graphics.shaderParameter3D_.projection);
    //DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

    //DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    //pos.y += 2;
    //DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&pos);

    ////���[���h���W����X�N���[�����W�ɕϊ�
    //Pos = DirectX::XMVector3Project(
    //    Pos,
    //    viewport.TopLeftX, viewport.TopLeftY,
    //    viewport.Width, viewport.Height,
    //    viewport.MinDepth, viewport.MaxDepth,
    //    Projection, View, World
    //);

    //DirectX::XMStoreFloat3(&pos, Pos);
    //DirectX::XMFLOAT2 size{100, 20};
 
    //hpSprite_->Render(dc, pos.x- size.x/2, pos.y - size.y / 2, size.x, size.y
    //    , 0, 0, hpSprite_->GetTextureWidth(), hpSprite_->GetTextureHeight()
    //    , 0, 1, 1, 1, 1);
}

void EnemyCom::MaskRender(PostEffect* postEff, std::shared_ptr<CameraCom> maskCamera)
{
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();

    //�r���[�|�[�g
    D3D11_VIEWPORT viewport;
    UINT numViewports = 1;
    dc->RSGetViewports(&numViewports, &viewport);

    //�ϊ��s��
    std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
    DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&camera->GetView());
    DirectX::XMMATRIX Projection = DirectX::XMLoadFloat4x4(&camera->GetProjection());
    DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();

    //�\���J�����O

    //�J�����̌���
    DirectX::XMFLOAT3 cameraDir = GameObjectManager::Instance().Find("Camera")->transform_->GetWorldFront();
    DirectX::XMFLOAT3 cameraPos = GameObjectManager::Instance().Find("Camera")->transform_->GetWorldPosition();
    DirectX::XMVECTOR CameraToEnemy = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&cameraPos)));
    float dot = DirectX::XMVector3Dot(DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&cameraDir)), CameraToEnemy).m128_f32[0];
    if (dot < 0.5f)return;

    //����
    float len = DirectX::XMVector3Length(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&playerPos))).m128_f32[0];
    float drawLen = 10; //�\���͈�
    if (len > drawLen)return;
    len /= drawLen;
    len = (len - 1.0f) * -1.0f;

    pos.y += 2;
    DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&pos);

    //���[���h���W����X�N���[�����W�ɕϊ�
    Pos = DirectX::XMVector3Project(
        Pos,
        viewport.TopLeftX, viewport.TopLeftY,
        viewport.Width, viewport.Height,
        viewport.MinDepth, viewport.MaxDepth,
        Projection, View, World
    );

    DirectX::XMStoreFloat3(&pos, Pos);

    DirectX::XMFLOAT2 size{50 * len, 50 * len};
    pos.x = pos.x - size.x / 2;
    pos.y = pos.y - size.y / 2;

    //���C�v�w�i
    hpBackSprite_->Render(dc, pos.x, pos.y, size.x, size.y
        , 0, 0, hpBackSprite_->GetTextureWidth(), hpBackSprite_->GetTextureHeight()
        , 0, 1, 1, 1, 1);

    //HP
    {
        //�}�X�N���鑤�`��
        postEff->CacheMaskBuffer(maskCamera);

        std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
        float hpRatio = float(status->GetHP()) / float(status->GetMaxHP());


        //hpmask
        hpMaskSprite_->Render(dc, pos.x + size.x, pos.y + size.x * 0.98f, size.x, size.y * 0.95f * hpRatio
            , 0, 0, hpMaskSprite_->GetTextureWidth(), hpMaskSprite_->GetTextureHeight()
            , 180, 1, 1, 1, 0.001f);

        //�}�X�N����鑤�`��
        postEff->StartBeMaskBuffer();

        //hp
        hpSprite_->Render(dc, pos.x, pos.y, size.x, size.y
            , 0, 0, hpSprite_->GetTextureWidth(), hpSprite_->GetTextureHeight()
            , 0, 1, 1, 1, 1);

        //�}�X�N�����I������
        postEff->RestoreMaskBuffer();

        postEff->DrawMask();
    }

}

// �^�[�Q�b�g�ʒu�������_���ݒ�
void EnemyCom::SetRandomTargetPosition()
{
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    pos.x += Mathf::RandomRange(-3, 3);
    pos.z += Mathf::RandomRange(-3, 3);
    targetPosition_ = pos;
}

//�^�[�Q�b�g�ʒu�Ɉړ��A��]
void EnemyCom::GoTargetMove(bool isMove, bool isTurn)
{
    // �ړI�n�_�܂ł�XZ���ʂł̋�������
    DirectX::XMFLOAT3 position = GetGameObject()->transform_->GetWorldPosition();

    // �ړI�n�_�ֈړ�
    DirectX::XMVECTOR Pos = { position.x,0,position.z };
    DirectX::XMVECTOR TPos = { targetPosition_.x,0,targetPosition_.z };
    DirectX::XMFLOAT3 force;
    DirectX::XMStoreFloat3(&force, DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(TPos, Pos)), moveDataEnemy_.runSpeed));

    if (isMove)
    {
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        move->AddForce(force);
    }

    if (isTurn)
    {
        //��]����
        QuaternionStruct myQ = GetGameObject()->transform_->GetRotation();
        QuaternionStruct focusQ = QuaternionStruct::LookRotation(force);
        //�⊮����
        DirectX::XMStoreFloat4(&myQ.dxFloat4, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&myQ.dxFloat4), DirectX::XMLoadFloat4(&focusQ.dxFloat4), 0.1f));
        GetGameObject()->transform_->SetRotation(myQ.dxFloat4);
    }
}


//�v���C���[���߂���true
bool EnemyCom::SearchPlayer()
{
    DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();

    float vx = playerPos.x - pos.x;
    float vy = playerPos.y - pos.y;
    float vz = playerPos.z - pos.z;
    float dist = sqrtf(vx * vx + vy * vy + vz * vz);

    if (dist < searchRange_)
    {
        return true;
    }
    return false;
}

//�_���[�W�m�F
bool EnemyCom::OnDamageEnemy()
{
    return GetGameObject()->GetComponent<CharacterStatusCom>()->GetFrameDamage();
}

//��e�[�������オ�胂�[�V����
void EnemyCom::SetStandUpMotion()
{
    //���n�A�j���[�V���������
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetTriggerOn("damageFallEnd");
    isStandUpMotion_ = true;
    playStandUp_ = false;
    //�U�����󂯂Ă��ړ����Ȃ��悤��
    GetGameObject()->GetComponent<CharacterStatusCom>()->SetAttackNonMove(true);
}
void EnemyCom::StandUpUpdate()
{
    if (isStandUpMotion_)
    {
        std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
        int index = animation->GetCurrentAnimationIndex();
        if (index == FALL_STAND_UP) //�N���オ�胂�[�V������
        {
            playStandUp_ = true;
        }
        if (playStandUp_)
        {
            if (!animation->IsPlayAnimation())
            {
                playStandUp_ = false;
                isStandUpMotion_ = false;
                GetGameObject()->GetComponent<CharacterStatusCom>()->SetAttackNonMove(false);
            }
        }
    }
}

//�_���[�W����
void EnemyCom::DamageProcess(float elapsedTime)
{
    //�󒆃_���[�W���d�͂������̊�0�ɂ���
    {
        if (!isAnimDamage_ && oldAnimDamage_)
        {
            if (!GetGameObject()->GetComponent<MovementCom>()->OnGround())
            {
                skyGravityZeroTimer_ = 0.3f;
            }
        }
        oldAnimDamage_ = isAnimDamage_;
    }

    //�_���[�W�A�j���[�V��������
    if (isAnimDamage_)
    {
        isAnimDamage_ = GetGameObject()->GetComponent<CharacterStatusCom>()->GetDamageAnimation();
    }

    //�W�����v��e���̒��n����
    if (isJumpDamage_)
    {
        if (!isAnimDamage_)
        {
            if (GetGameObject()->GetComponent<MovementCom>()->OnGround())
            {
                isJumpDamage_ = false;
                //�����オ�胂�[�V�����N��
                SetStandUpMotion();
            }
        }
    }

    //�_���[�W�G�t�F�N�g�Ԋu�Ǘ�
    damageEffTimer_ -= elapsedTime;

    //�_���[�W����
    if (OnDamageEnemy())
    {
        bool endTree = false;   //���̑J�ڂ��I��点�Ĕ�e�A�j���[�V��������Ȃ�true

        //�A���X�g�b�p�u����e�Ȃ炻�̂܂܃A�j���[�V����
        ATTACK_SPECIAL_TYPE attackType = GetGameObject()->GetComponent<CharacterStatusCom>()->GetDamageType();
        if (attackType == ATTACK_SPECIAL_TYPE::UNSTOP)
            endTree = true;
        //�W�����v���͖��h��
        if (attackType == ATTACK_SPECIAL_TYPE::JUMP_NOW)
            endTree = true;


        //��e����s�������f
        for (int& id : damageAnimAiTreeId_)
        {
            //�A�N�V�����m�[�h���Ȃ��ꍇ
            if (!activeNode_)
                endTree = true;

            if (endTree)break;


            NodeBase* node = activeNode_.get();
            //��e���ɃA�j���[�V�������邩�m�F
            while (1)
            {
                //���Ȃ�ID�Ȃ�endTree��true��
                if (node->GetId() == id)
                {
                    endTree = true;
                    break;
                }

                //�e���m�F����
                node = node->GetParent().get();
                if (!node)break;
            }

        }

            //�J�ڂ��I��点�ăA�j���[�V��������
        if (endTree)
        {
            if (activeNode_)
            {
                activeNode_->EndActionSetStep();
                activeNode_->Run(GetGameObject()->GetComponent<EnemyCom>(), elapsedTime);
                //����
                activeNode_.reset();
            }
        }

        //�_���[�W����
        if (endTree)
        {
            isAnimDamage_ = true;
            std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
            animator->ResetParameterList();

            //�A�j���[�V�����̎�ނ𔻒�
            if (isStandUpMotion_)   //�����オ�胂�[�V��������D��
            {
                SetStandUpMotion();
            }
            else if (attackType == ATTACK_SPECIAL_TYPE::JUMP_START)  //�؂�グ��e��
            {
                animator->SetTriggerOn("damageGoFly");
                isJumpDamage_ = true;

            }
            else if (isJumpDamage_)    //�󒆔�e��
            {
                animator->SetTriggerOn("damageInAir");
            }
            else
            {
                animator->SetTriggerOn("damage");
            }
        }

        //�G�t�F�N�g
        if (damageEffTimer_ < 0)
        {
            GetGameObject()->GetChildFind("Particle")->GetComponent<ParticleSystemCom>()->Restart();
            damageEffTimer_ = 0.5f;
        }
    }
}

//�W���X�g���p����o��
void EnemyCom::justColliderProcess()
{
    //�W���X�g��𓖂��蔻���؂�A�A�^�b�N�����蔻������Ă���
    DirectX::XMFLOAT3 pos;
    //�W���X�g
    std::shared_ptr<GameObject> justChild = GetGameObject()->GetChildFind("picolaboAttackJust");

    //�W���X�g�����蔻���؂��Ă���
    justChild->GetComponent<Collider>()->SetEnabled(false);

    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
    for (auto& animEvent : animation->GetCurrentAnimationEventsData())
    {
        //just�������Ă���Ȃ�Ȃ�
        if (animEvent.name.find("just") == std::string::npos)continue;
        if (isJustAvoid_)justChild->GetComponent<Collider>()->SetEnabled(true);
        //�C�x���g���Ȃ瓖���蔻����o��
        if (animation->GetCurrentAnimationEvent(animEvent.name.c_str(), pos))
        {
            isJustAvoid_ = false;
            justChild->GetComponent<Collider>()->SetEnabled(true);
        }
    }
}

//�d�͐ݒ�
void EnemyCom::GravityProcess(float elapsedTime)
{
    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();

    //�A�j���[�V�����C�x���g�����ďd�͂��Ȃ���
    if (animation->GetCurrentAnimationEvent("ZeroGravity", DirectX::XMFLOAT3()) || skyGravityZeroTimer_ > 0)
    {
        skyGravityZeroTimer_ -= elapsedTime;
        isSetGravity_ = true;
        GetGameObject()->GetComponent<MovementCom>()->SetGravity(GRAVITY_ZERO);
        GetGameObject()->GetComponent<MovementCom>()->ZeroVelocityY();
    }
    else
    {
        if (isSetGravity_)   //�d�͐ݒ����������������
        {
            isSetGravity_ = false;
            GetGameObject()->GetComponent<MovementCom>()->SetGravity(GRAVITY_NORMAL);
            GetGameObject()->GetComponent<MovementCom>()->ZeroVelocityY();
        }
    }
}

bool EnemyCom::OnMessage(const Telegram& msg)
{
    return false;
}

//�A�j���[�V�����̍X�V
void EnemyCom::AnimationSetting()
{
    //�ړ��A�j���[�V�����ݒ�
    {
        //�ړ��X�s�[�h���
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&move->GetVelocity());
        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetFloatValue("moveSpeed", DirectX::XMVectorGetX(DirectX::XMVector3Length(Velocity)));
    }

}




//��e���ɃA�j���[�V�������鎞��AITREE�����߂�
template<typename... Args>
void EnemyCom::OnDamageAnimAI_TREE(Args... args)
{
    for (AI_TREE i : std::initializer_list<AI_TREE>{ args... }) {
        damageAnimAiTreeId_.emplace_back(static_cast<int>(i));
    }
}
