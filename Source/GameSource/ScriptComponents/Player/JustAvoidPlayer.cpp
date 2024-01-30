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
    //�W���X�g��𔻒莞
    if (isJustJudge_)
    {
        //�W���X�g����ړ�����
        JustAvoidanceMove(elapsedTime);

        //�W���X�g����㔽�����͊m�F
        JustAvoidanceAttackInput();
    }

    //�W���X�g������o
    JustAvoidDirectionEnd(elapsedTime);

    //�W���X�g��𐢊E�F���o
    JustDirectionUpdate(elapsedTime);

    //���������X�V
    switch (justAvoidKey_)
    {
        //������
    case JUST_AVOID_KEY::SQUARE:
        JustAvoidanceSquare(elapsedTime);
        break;
        //������
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

    //���b�N�I�����m�[�}���̎�
    if (player_.lock()->GetLockOn() == PlayerCom::LOCK_TARGET::JUST_LOCK)
    {
        //���b�N�I���摜�o��
        Sprite* lockOnSprite = player_.lock()->GetLockOnSprite();

        //���b�N�I������G��T��
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


//�W���X�g���������
void JustAvoidPlayer::JustInisialize()
{
    justAvoidLeadKey_ = JUST_AVOID_KEY::NULL_KEY;
    justAvoidState_ = -1;
    isJustJudge_ = false;
    Graphics::Instance().SetWorldSpeed(1);
    //�J���[�O���[�f�B���O�߂�
    Graphics::Instance().shaderParameter3D_.colorGradingData.saturation = 1.5f;
    //�V���G�b�g�L����
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

//�W���X�g����ړ�����
void JustAvoidPlayer::JustAvoidanceMove(float elapsedTime)
{
    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    //���o�擾
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
        //�W���X�g����ɕύX

        player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::JUSTDASH);
        justAvoidTimer_ = justAvoidTime_;

        player_.lock()->GetMovePlayer()->SetIsInputMove(false);
        player_.lock()->GetAttackPlayer()->SetIsNormalAttack(false);
        player_.lock()->GetMovePlayer()->SetIsDash(false);

        //���͕������݂ăA�j���[�V�����Đ�
        DirectX::XMVECTOR Input = DirectX::XMLoadFloat3(&player_.lock()->GetMovePlayer()->GetInputMoveVec());

        //�A�j���[�V�����Đ�
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();

        //animator->SetAnimationSpeedOffset(0.3f);

        std::shared_ptr<RendererCom> renderCom = player_.lock()->GetGameObject()->GetComponent<RendererCom>();
        //�v���C���[������
        renderCom->GetModel()->SetMaterialColor({ 1,1,1,0 });
        //�V���G�b�g�؂�
        renderCom->SetSilhouetteFlag(false);

        //���E�F�ω����o
        justSpriteState_ = 0;

        //���͂ɂ��A�j���[�V������ς���
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

        //���g�o��
        for (int i = 0; i < 4; ++i)
        {
            std::shared_ptr<AnimationCom> justAnim = justPico[i]->GetComponent<AnimationCom>();
            //������
            justPico[i]->transform_->SetLocalPosition({ 0,0,0 });
            Model* justModel = justPico[i]->GetComponent<RendererCom>()->GetModel();
            justModel->SetMaterialColor({ 0.4f,0.3f,0.1f,0.65f });  //�F������

            //�o��
            justPico[i]->SetEnabled(true);
            if (inputFlag)
                justAnim->PlayAnimation(ANIMATION_PLAYER::DODGE_FRONT, false);
            else
                justAnim->PlayAnimation(ANIMATION_PLAYER::DODGE_BACK, false);
        }

        //�󒆉���̏ꍇ�͏d��0��
        onGroundAvoid_ = true;
        if (player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP_DASH
            || player_.lock()->GetPlayerStatus() == PlayerCom::PLAYER_STATUS::JUMP_BACK_DASH)
        {
            move->SetGravity(GRAVITY_ZERO);
            move->ZeroVelocityY();
            onGroundAvoid_ = false;
        }

        //�G�̕�������
        if (!justHitEnemy_.expired())
        {
            DirectX::XMFLOAT3 enemyPos = justHitEnemy_.lock()->transform_->GetWorldPosition();
            enemyPos.y = player_.lock()->GetGameObject()->transform_->GetWorldPosition().y;
            player_.lock()->GetGameObject()->transform_->LookAtTransform(enemyPos);
        }

        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUST);


        //�J���[�O���[�f�B���O
        Graphics::Instance().shaderParameter3D_.colorGradingData.saturation = 0.4f;

        ////�J�����V�F�C�N
        //GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()->CameraShake(0.1f, 0.25f);

        //�q�b�g�X�g�b�v
        GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()
            ->HitStop(0.2f);
        hitStopEnd_ = false;

        //�J�������߂Â���
        player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>()->SetChangeRange(0.5f, NearRange);

        justAvoidState_++;
    }
    break;
    //���g�ړ��������A�j���X�s�[�h�߂�
    case 1:
    {
        //�q�b�g�X�g�b�v��X���[
        if (!GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()->GetIsHitStop() && !hitStopEnd_)
        {
            hitStopEnd_ = true;
            Graphics::Instance().SetWorldSpeed(0.5f);
        }

        //�X�e�[�g�G���h�t���O
        bool endFlag = false;


        //���o�p�s�R�|�W�V����
        DirectX::XMFLOAT3 justPicoPos[4];
        for (int i = 0; i < 4; ++i)
        {
            justPicoPos[i] = justPico[i]->transform_->GetLocalPosition();

            //�A�j���X�s�[�h���v���C���[�ɍ��킹��
            std::shared_ptr<AnimationCom> justAnim = justPico[i]->GetComponent<AnimationCom>();
            std::shared_ptr<AnimationCom> picoAnim = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
            justAnim->SetAnimationSpeed(picoAnim->GetAnimationSpeed());
        }
        float speed = 250;
        float moveRange = 100;
        //�c�ʒu�X�V
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
        //���ʒu�X�V
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

        //�J���[�O���[�f�B���O�߂�
        Graphics::Instance().shaderParameter3D_.colorGradingData.saturation += elapsedTime;

        //�X�e�[�g�I���
        if (endFlag)
        {
            //�v���C���[�������点��
            player_.lock()->GetGameObject()->GetComponent<RendererCom>()->
                GetModel()->SetMaterialColor({ 1,2.0f,1,1.5f });
            playerDirection_ = true;    //�߂�������JustAvoidDirection�֐��ł���

            //�J����������
            player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>()->SetChangeRange(1.0f, FarRange);

            justAvoidState_++;
        }

    }
    break;
    //���g����
    //�v���C���[�o��
    case 2:
    {
        //�X�e�[�g�G���h�t���O
        bool endFlag = false;

        //���X�ɓ�����
        for (int i = 0; i < 4; ++i)
        {
            Model* justModel = justPico[i]->GetComponent<RendererCom>()->GetModel();
            DirectX::XMFLOAT4 color = justModel->GetMaterialColor();
            color.w -= elapsedTime;
            justModel->SetMaterialColor(color);
            if (color.w < 0.4f) //�����ɂȂ���������i0.5f�ȉ��łقړ����Ȃ̂�0.4f�œ�������j
            {
                justPico[i]->SetEnabled(false);
                endFlag = true;
            }

            //�A�j���X�s�[�h���v���C���[�ɍ��킹��
            std::shared_ptr<AnimationCom> justAnim = justPico[i]->GetComponent<AnimationCom>();
            std::shared_ptr<AnimationCom> picoAnim = player_.lock()->GetGameObject()->GetComponent<AnimationCom>();
            justAnim->SetAnimationSpeed(picoAnim->GetAnimationSpeed());  
        }

        //�J���[�O���[�f�B���O�߂�
        Graphics::Instance().shaderParameter3D_.colorGradingData.saturation += elapsedTime;

        if (endFlag)
            justAvoidState_++;
    }
    break;
    //�����󂯓���
    case 3:
    {
        //�A�j���I���Ŕ����I��
        if (!player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->IsPlayAnimation())
        {
            //�v���C���[�A�j���X�s�[�h�߂�
            std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
            animator->SetAnimationSpeedOffset(1.0f);

            //���E�̐F�߂�
            justSpriteState_ = 20;

            justAvoidTimer_ = -1;
        }

        //�X���[�a�炰��
        float slow = Graphics::Instance().GetWorldSpeed();
        slow += 0.5f * elapsedTime;
        Graphics::Instance().SetWorldSpeed(slow);

        //�J���[�O���[�f�B���O�߂�
        Graphics::Instance().shaderParameter3D_.colorGradingData.saturation += elapsedTime;

        //�W���X�g����I���^�C�}�[
        justAvoidTimer_ -= elapsedTime;
        if (justAvoidTimer_ < 0)
        {
            JustInisialize();

            //���E�̐F�߂�
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

    //�J���[�O���[�f�B���O����
    if (Graphics::Instance().shaderParameter3D_.colorGradingData.saturation > 1.5f)
    {
        Graphics::Instance().shaderParameter3D_.colorGradingData.saturation = 1.5f;
    }

    //�ړ��o���邩
    int animIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();
    if (animIndex != ANIMATION_PLAYER::DODGE_FRONT && animIndex != ANIMATION_PLAYER::DODGE_BACK)
    {
        //�ړ����������߂�i�W���X�g��𒆂͏���Ɉړ�����j
        DirectX::XMFLOAT3 Direction;

        DirectX::XMFLOAT3 inputVec = player_.lock()->GetMovePlayer()->GetInputMoveVec();
        if (inputVec.x * inputVec.x + inputVec.z * inputVec.z > 0.1f)
        {
            Direction = { inputVec.x ,0,inputVec.z };
        }
        else
        {
            Direction = player_.lock()->GetGameObject()->transform_->GetWorldFront();
            //y�������ċt�����ɂ���
            Direction.x *= -1;
            Direction.y = 0;
            Direction.z *= -1;
            //���K��
            DirectX::XMStoreFloat3(&Direction, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&Direction)));
        }

        player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::JUSTDASH);
        //�͂ɉ�����
        move->AddForce(Direction);
    }
}

//�W���X�g��𔽌����͊m�F
void JustAvoidPlayer::JustAvoidanceAttackInput()
{
    //�{�^���Ŕ����ς���
    GamePad& gamePad = Input::Instance().GetGamePad();

    if (justAvoidState_ < 3)
    {
        //��s����
        if (gamePad.GetButtonDown() & GamePad::BTN_X)
            justAvoidLeadKey_ = JUST_AVOID_KEY::SQUARE;
        if (gamePad.GetButtonDown() & GamePad::BTN_Y)
            justAvoidLeadKey_ = JUST_AVOID_KEY::TRIANGLE;

        return;
    }

    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();


    //���̏ꍇ
    if ((gamePad.GetButtonDown() & GamePad::BTN_X) || (justAvoidLeadKey_ == JUST_AVOID_KEY::SQUARE))
    {
        JustInisialize();
        player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::DASH);

        justAvoidKey_ = JUST_AVOID_KEY::SQUARE;

        player_.lock()->GetMovePlayer()->SetIsInputMove(false);

        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);

        //�G�̕�������
        DirectX::XMFLOAT3 enemyPos = justHitEnemy_.lock()->transform_->GetWorldPosition();
        enemyPos.y = player_.lock()->GetGameObject()->transform_->GetWorldPosition().y;
        player_.lock()->GetGameObject()->transform_->LookAtTransform(enemyPos);

        //�G���X���[�ɂ���
        EnemyManager::Instance().SetEnemySpeed(0.1f, 5.0f);

        //SE
        squareSlowInOutSE_->Play(false,3.0f);
        squareSlowSE_->Play(true, 3.0f);

        //���E�F���o
        justSpriteState_ = 10;
        //�p�[�e�B�N�����o�N��
        SetJustUnderParticle(true);
    }
    //���̏ꍇ
    else if ((gamePad.GetButtonDown() & GamePad::BTN_Y) || (justAvoidLeadKey_ == JUST_AVOID_KEY::TRIANGLE))
    {
        JustInisialize();
        triangleState_ = 0;
        justAvoidKey_ = JUST_AVOID_KEY::TRIANGLE;

        //���E�F���o
        justSpriteState_ = 20;
    }
}

//������
void JustAvoidPlayer::JustAvoidanceSquare(float elapsedTime)
{
    //�G�ɐڋ߂���
    {
        std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

        //�󒆂̏ꍇ
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
        //�G�̋߂��܂ňړ�����
        if (length < 1.5f || length > 5)
        {
            JustInisialize();
            player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::RUN);
            justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
            player_.lock()->GetMovePlayer()->SetIsInputMove(true);
            player_.lock()->GetAttackPlayer()->SetIsNormalAttack(true);
            player_.lock()->GetMovePlayer()->SetIsDash(true);

            //�A�^�b�N�����Ɉ����p��
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

        //�͂ɉ�����
        move->AddForce(dir);

    }
}

//������
void JustAvoidPlayer::JustAvoidanceTriangle(float elapsedTime)
{
    //���G����
    player_.lock()->GetGameObject()->GetComponent<CharacterStatusCom>()
        ->SetInvincibleNonDamage(0.5f);

    //�w��̃J�����|�X��Ԃ�
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

    //�J�����|�X����v���C���[�|�X���t�Z
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

    static float pushSeconds = 0;   //�A�j���[�V�����C�x���g�܂ł̎���
    static float lockEnemySeconds = 0;  //���b�N�I���ύX���Ɏg��
    static DirectX::XMFLOAT3 focusEnemy{0, 0, 0};
    switch (triangleState_)
    {
        //�A�j���[�V�����Đ�
    case 0:
    {
        std::shared_ptr<AnimatorCom> animator = player_.lock()->GetGameObject()->GetComponent<AnimatorCom>();
        animator->SetTriggerOn("triangleJust");

        lockTriangleEnemy_ = justHitEnemy_;
        pushSeconds = 0.0f;

        //���b�N�I���^�[�Q�b�g�ς���
        player_.lock()->SetLockOn(PlayerCom::LOCK_TARGET::JUST_LOCK);

        dashSE_->Play(false);

        triangleState_++;
    }
    break;
    //�����~�߂�A�J����������
    case 1:
    {
        //�A�j���[�V�����C�x���g�܂ł̎��Ԃ̊���
        float animEventStartTime = 0.4f;
        pushSeconds += elapsedTime;
        if (pushSeconds > animEventStartTime)pushSeconds = animEventStartTime;
        float startRatio = pushSeconds / animEventStartTime;

        focusEnemy = lockTriangleEnemy_.lock()->transform_->GetWorldPosition();

        //�J�����ύX
        {
            //�J�����R���g���[���[�擾
            std::shared_ptr<PlayerCameraCom> playerCameraCom = player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>();
            playerCameraCom->SetIsJust(true);

            //��Ԃ��ăJ�����ړ�
            DirectX::XMVECTOR CameraPos = DirectX::XMLoadFloat3(&GameObjectManager::Instance().Find("Camera")->transform_->GetWorldPosition());
            DirectX::XMFLOAT3 cameraPos;
            DirectX::XMStoreFloat3(&cameraPos, DirectX::XMVectorLerp(CameraPos, DirectX::XMLoadFloat3(&GetCameraPos()), startRatio));
            playerCameraCom->SetJustPos(cameraPos);

            //��Ԃ��ăJ�����t�H�[�J�X�|�X�ړ�
            DirectX::XMVECTOR FocusPos = DirectX::XMLoadFloat3(&playerCameraCom->GetForcusPos());
            DirectX::XMFLOAT3 focus;
            DirectX::XMStoreFloat3(&focus, DirectX::XMVectorLerp(FocusPos, DirectX::XMLoadFloat3(&focusEnemy), startRatio));
            playerCameraCom->SetForcusPos(focus);

            //��Ԃ��ăv���C���[�|�X�ړ�
            DirectX::XMFLOAT3 pPos = player_.lock()->GetGameObject()->transform_->GetWorldPosition();
            DirectX::XMVECTOR PPos = DirectX::XMLoadFloat3(&pPos);
            pPos.y = 5;
            DirectX::XMStoreFloat3(&pPos, DirectX::XMVectorLerp(PPos, DirectX::XMLoadFloat3(&pPos), startRatio));
            player_.lock()->GetGameObject()->transform_->SetWorldPosition(pPos);

            //�J�����̌����Ƀv���C���[��������
            QuaternionStruct cameraQuaternion = GameObjectManager::Instance().Find("Camera")->transform_->GetRotation();

            //�J�����̌����Ƀv���C���[�����킹��
            static DirectX::XMFLOAT4 rota = cameraQuaternion.dxFloat4;
            DirectX::XMVECTOR CA = DirectX::XMLoadFloat4(&cameraQuaternion.dxFloat4);
            DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&rota), CA, elapsedTime * 5));

            player_.lock()->GetGameObject()->transform_->SetRotation(rota);

        }

        //�A�j���[�V�����C�x���g�����Ď��̃X�e�[�g��
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
    //�Ə������߂�
    case 2:
    {
        //�J�����̌����Ƀv���C���[��������
        QuaternionStruct cameraQuaternion = GameObjectManager::Instance().Find("Camera")->transform_->GetRotation();

        //�J�����R���g���[���[�擾
        std::shared_ptr<PlayerCameraCom> playerCameraCom = player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>();


        //�w��G�l�~�[�擾
        std::shared_ptr<GameObject> lockEnemy = lockTriangleEnemy_.lock();
        DirectX::XMFLOAT3 enemyPos = lockEnemy->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 playerPos = player_.lock()->GetGameObject()->transform_->GetWorldPosition();

        //�J����
        DirectX::XMStoreFloat3(&focusEnemy, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&focusEnemy), DirectX::XMLoadFloat3(&enemyPos), elapsedTime * 2));
        playerCameraCom->SetForcusPos(focusEnemy);

        //�J�����̌����Ƀv���C���[�����킹��
        static DirectX::XMFLOAT4 rota = cameraQuaternion.dxFloat4;
        DirectX::XMVECTOR CA = DirectX::XMLoadFloat4(&cameraQuaternion.dxFloat4);
        DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&rota), CA, elapsedTime * 5));

        player_.lock()->GetGameObject()->transform_->SetRotation(rota);
        player_.lock()->GetGameObject()->transform_->SetWorldPosition(GetCameraFromPlayerPos());

        //���͏����擾
        GamePad& gamePad = Input::Instance().GetGamePad();
        float ax = gamePad.GetAxisRX();
        float ay = gamePad.GetAxisRY();

        //���͂���Ă���ꍇ
        //���b�N�I���ύX
        lockEnemySeconds += elapsedTime;
        if (ax * ax + ay * ay > 0.01f && lockEnemySeconds > 0.5f)
        {
            //���b�N�I�����̓G�̃|�X
            DirectX::XMVECTOR lockEnemyPos = DirectX::XMLoadFloat3(&lockTriangleEnemy_.lock()->transform_->GetWorldPosition());

            //���ς��Ċp�x���ċ߂��G���^�[�Q�b�g��
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

                //���ς��Ċp�x���ċ߂��G���^�[�Q�b�g��
                DirectX::XMFLOAT3 nearEnemyPos = enemy.enemy.lock()->transform_->GetWorldPosition();
                DirectX::XMVECTOR NextEnemyVec = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&nearEnemyPos), lockEnemyPos);
                DirectX::XMVECTOR NextEnemyVecNorm = DirectX::XMVector3Normalize(NextEnemyVec);

                //���̓x�N�g�����G�l�~�[�̈ʒu����ɕϊ�
                DirectX::XMVECTOR InputEnemyVec = DirectX::XMVectorAdd(DirectX::XMVectorScale(PEForward, ay), DirectX::XMVectorScale(PERight, ax));
                InputEnemyVec = DirectX::XMVector3Normalize(DirectX::XMVectorSetY(InputEnemyVec, 0));


                //���ςŊp�x����
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

                //���ς��Ċp�x���ċ߂��G���^�[�Q�b�g��
                DirectX::XMFLOAT3 farEnemyPos = enemy.enemy.lock()->transform_->GetWorldPosition();
                DirectX::XMVECTOR NextEnemyVec = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&farEnemyPos), lockEnemyPos);
                DirectX::XMVECTOR NextEnemyVecNorm = DirectX::XMVector3Normalize(NextEnemyVec);

                //���̓x�N�g�����G�l�~�[�̈ʒu����ɕϊ�
                DirectX::XMVECTOR InputEnemyVec = DirectX::XMVectorAdd(DirectX::XMVectorScale(PEForward, ay), DirectX::XMVectorScale(PERight, ax));
                InputEnemyVec = DirectX::XMVector3Normalize(DirectX::XMVectorSetY(InputEnemyVec, 0));


                //���ςŊp�x����
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

            //near��far�Ŕ���
            {
                if (enemyNear || enemyFar)
                {
                    bool endJudge = false;
                    //�ǂ��炩�������Ă��Ȃ��Ƃ�
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
                        //�߂���������
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

        //���Ŏ��̃X�e�[�g
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
    //������΂�
    case 3:
    {
        //�A�j���[�V���������ĕ���𓊂���
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

            //�u���[
            player_.lock()->BlurStartPlayer(2.5f, 1, "", {}, lockTriangleEnemy_.lock());

            triangleState_++;
        }
    }
    break;
    //����̈ʒu�Ƀv���C���[�s���A�J��������ɂ��Ă���
    case 4:
    {
        player_.lock()->GetGameObject()->transform_->SetWorldPosition(GetCameraFromPlayerPos());

        std::shared_ptr<GameObject> pushObj = GameObjectManager::Instance().Find("Push");
        //���킪�G�ɓ��B������
        if (!pushObj->GetComponent<PushWeaponCom>()->IsMove())
        {
            JustInisialize();

            //�w��G�l�~�[�擾
            std::shared_ptr<GameObject> lockEnemy = lockTriangleEnemy_.lock();
            DirectX::XMFLOAT3 enemyPos = lockEnemy->transform_->GetWorldPosition();
            DirectX::XMFLOAT3 playerPos = player_.lock()->GetGameObject()->transform_->GetWorldPosition();

            //�G�l�~�[����v���C���[�̐��K���x�N�g��
            DirectX::XMVECTOR EPNorm = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&enemyPos)));


            player_.lock()->GetMovePlayer()->SetIsInputMove(true);
            player_.lock()->GetAttackPlayer()->SetIsNormalAttack(true);
            player_.lock()->GetMovePlayer()->SetIsDash(true);
            player_.lock()->GetMovePlayer()->SetJumpFlag(true);

            //�A�^�b�N�����Ɉ����p��
            player_.lock()->GetAttackPlayer()->SetAnimFlagName("squareIdle");
            player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);

            //�v���C����G�̑O�Ɉړ�
            DirectX::XMFLOAT3 movePos;
            DirectX::XMStoreFloat3(&movePos, EPNorm);

            movePos.x = enemyPos.x + movePos.x * 2;
            movePos.y = enemyPos.y + movePos.y * 2;
            movePos.z = enemyPos.z + movePos.z * 2;

            player_.lock()->GetGameObject()->transform_->SetWorldPosition(movePos);

            //�v���C���[����G�l�~�[
            DirectX::XMFLOAT3 pe;
            
            DirectX::XMStoreFloat3(&pe, 
                DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSetY(EPNorm, 0)), -1));

            player_.lock()->GetGameObject()->transform_->SetRotation(QuaternionStruct::LookRotation(pe));

            //�J�����R���g���[���[�擾
            std::shared_ptr<PlayerCameraCom> playerCameraCom = player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>();

            playerCameraCom->SetIsJust(false);

            std::shared_ptr<GameObject> candyObj = GameObjectManager::Instance().Find("CandyPush");
            std::shared_ptr<WeaponCom> weapon = candyObj->GetComponent<WeaponCom>();
            weapon->SetIsForeverUse(false);

            //���b�N�I���^�[�Q�b�g�ς���
            player_.lock()->SetLockOn(PlayerCom::LOCK_TARGET::NORMAL_LOCK);

            justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
        }
    }
    break;
    }
}

void JustAvoidPlayer::JustDirectionUpdate(float elapsedTime)
{
    //���E�̐F�ς���
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
        //�ҋ@�p
    }
    break;
    case 10:
    {
        //���W���X�g�U��
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
        //���Ԗ߂�܂őҋ@
        if (!EnemyManager::Instance().GetIsSlow())
        {
            //�p�[�e�B�N�����o�I��
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
        //�߂�
        Sprite::DissolveConstans& dissolveData = justSprite_->GetEffectSpriteData().dissolveConstant;
        dissolveData.dissolveThreshold -= 1.5f * elapsedTime;
        if (dissolveData.dissolveThreshold < 0)justSpriteState_++;
    }
    break;
    case 21:
    {
        //�I������
        //�p�[�e�B�N��
        std::shared_ptr<ParticleSystemCom> particle = GameObjectManager::Instance().Find("justUnderParticle")->GetComponent<ParticleSystemCom>();
        particle->GetGameObject()->SetEnabled(false);

        //�X�v���C�g
        isJustSprite_ = false;
        justSpriteState_ = -1;

        //�J������߂�
        player_.lock()->GetGameObject()->GetComponent<PlayerCameraCom>()->SetChangeRange(0.1f, DefaultRange);
    }
    break;
    }

    if (justSpriteState_ > 1 && justSpriteState_ <= 20)
    {
        //�J����������
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

//�W���X�g����o����������
void JustAvoidPlayer::JustAvoidJudge()
{
    //��ԋ߂��G��T�������_��
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

    //�W���X�g����̓����蔻��
    std::vector<HitObj> hitGameObj = player_.lock()->GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
    //�W���X�g��������G�l�~�[��ۑ�
    std::weak_ptr<GameObject> enemy;
    for (auto& hitObj : hitGameObj)
    {
        if (COLLIDER_TAG::JustAvoid != hitObj.gameObject.lock()->GetComponent<Collider>()->GetMyTag())continue;

        //���̂Ƃ���e����EnemyCom���Ȃ��̂Œe�p
        if (!hitObj.gameObject.lock()->GetParent()->GetComponent<EnemyCom>())
        {
            if (!enemy.lock())
                enemy = SearchMostNearEnemy();
            continue;
        }

        //�ŏ��������̂܂ܓ����
        if (!enemy.lock()) {
            enemy = hitObj.gameObject.lock()->GetParent();
            continue;
        }
        //��ԋ߂��G��ۑ�
        DirectX::XMFLOAT3 pPos = player_.lock()->GetGameObject()->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 ePos = hitObj.gameObject.lock()->GetParent()->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 eOldPos = enemy.lock()->transform_->GetWorldPosition();

        DirectX::XMVECTOR PPos = DirectX::XMLoadFloat3(&pPos);
        DirectX::XMVECTOR EPos = DirectX::XMLoadFloat3(&ePos);
        DirectX::XMVECTOR EOldPos = DirectX::XMLoadFloat3(&eOldPos);

        float currentLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(PPos, EPos)));
        float oldLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(PPos, EOldPos)));

        //��r
        if (currentLength < oldLength)
            enemy = hitObj.gameObject.lock()->GetParent();
    }

    justHitEnemy_ = enemy;
}

//�W���X�g������J�n����
void JustAvoidPlayer::StartJustAvoid()
{
    Score::Instance().AddScore(654321);

    //���G����
    player_.lock()->GetGameObject()->GetComponent<CharacterStatusCom>()
        ->SetInvincibleNonDamage(2);
    isJustJudge_ = true;
    justAvoidState_ = 0;

    //�G�t�F�N�g
    std::shared_ptr<GameObject> particleSpark = ParticleComManager::Instance().SetEffect(ParticleComManager::JUST_SPARK
        , player_.lock()->GetGameObject()->transform_->GetWorldPosition());
    particleSpark->SetName("particleSpark");
    particleSpark->GetComponent<ParticleSystemCom>()->SetIsWorldSpeed(false);
    //�G�t�F�N�g
    particleSpark = ParticleComManager::Instance().SetEffect(ParticleComManager::JUST_SPARK
        , player_.lock()->GetGameObject()->transform_->GetWorldPosition());
    particleSpark->SetName("particleSpark1");
    particleSpark->GetComponent<ParticleSystemCom>()->SetIsWorldSpeed(false);

    //�u���[
    player_.lock()->BlurStartPlayer(2.5f, 1, "Head");

    //SE
    justSE_->Stop();
    justSE_->Play(false);
}

void JustAvoidPlayer::JustAvoidDirectionEnd(float elapsedTime)
{
    if (!playerDirection_)return;

    //�v���C���[�F�߂�
    DirectX::XMFLOAT4 playerColor = player_.lock()->GetGameObject()->GetComponent<RendererCom>()->
        GetModel()->GetMaterialColor();
    playerColor.y -= 0.8f * elapsedTime;
    playerColor.w -= 0.5f * elapsedTime;
    //�����߂��Ă���Ή��o�I��
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
