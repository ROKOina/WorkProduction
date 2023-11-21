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
#include "../CharacterStatusCom.h"

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
    JustAvoidDirection(elapsedTime);

    //�W���X�g��𐢊E�F���o
    JustSpriteUpdate(elapsedTime);

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
    
    ImGui::DragFloat("dissolveThreshold", &dissolveData.dissolveThreshold, 0.01f, 0, 1);
    ImGui::DragFloat("edgeThreshold", &dissolveData.edgeThreshold, 0.01f, 0, 1);
    ImGui::DragFloat4("edgeColor", &dissolveData.edgeColor.x, 0.01f, 0, 1);
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

        //�G�̕�������
        if (!justHitEnemy_.expired())
            player_.lock()->GetGameObject()->transform_->LookAtTransform(justHitEnemy_.lock()->transform_->GetWorldPosition());

        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUST);


        //�J���[�O���[�f�B���O
        Graphics::Instance().shaderParameter3D_.colorGradingData.saturation = 0.4f;

        ////�J�����V�F�C�N
        //GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()->CameraShake(0.1f, 0.25f);

        //�q�b�g�X�g�b�v
        GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()
            ->HitStop(0.2f);
        hitStopEnd_ = false;

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
        player_.lock()->GetGameObject()->transform_->LookAtTransform(justHitEnemy_.lock()->transform_->GetWorldPosition());

        //�G���X���[�ɂ���
        EnemyManager::Instance().SetEnemySpeed(0.1f, 5.0f);

        //���E�F���o
        justSpriteState_ = 10;
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

        DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&player_.lock()->GetGameObject()->transform_->GetWorldPosition());
        DirectX::XMVECTOR EnemyPos = Pos;
        if (!justHitEnemy_.expired())
            EnemyPos = DirectX::XMLoadFloat3(&justHitEnemy_.lock()->transform_->GetWorldPosition());
        DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(EnemyPos, Pos);
        float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
        //�G�̋߂��܂ňړ�����
        if (length < 1.5f)
        {
            JustInisialize();
            player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::RUN);
            justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
            player_.lock()->GetMovePlayer()->SetIsInputMove(true);
            player_.lock()->GetAttackPlayer()->SetIsNormalAttack(true);
            player_.lock()->GetMovePlayer()->SetIsDash(true);

            //�A�^�b�N�����Ɉ����p��
            player_.lock()->GetAttackPlayer()->SetAnimFlagName("squareJust");
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
            playerCameraCom->isJust = true;

            //��Ԃ��Ĉړ�
            DirectX::XMVECTOR CameraPos = DirectX::XMLoadFloat3(&GameObjectManager::Instance().Find("Camera")->transform_->GetWorldPosition());
            DirectX::XMStoreFloat3(&playerCameraCom->pos, DirectX::XMVectorLerp(CameraPos, DirectX::XMLoadFloat3(&GetCameraPos()), startRatio));

            DirectX::XMVECTOR FocusPos = DirectX::XMLoadFloat3(&playerCameraCom->GetForcusPos());
            DirectX::XMFLOAT3 focus;
            DirectX::XMStoreFloat3(&focus, DirectX::XMVectorLerp(FocusPos, DirectX::XMLoadFloat3(&focusEnemy), startRatio));

            playerCameraCom->SetForcusPos(focus);

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

        //�G�l�~�[����v���C���[�̐��K���x�N�g��
        DirectX::XMVECTOR EPNorm = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&enemyPos)));


        float len = DirectX::XMVector3Length(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&focusEnemy), DirectX::XMLoadFloat3(&enemyPos))).m128_f32[0];


        //�J����
        DirectX::XMFLOAT3 cameraPos = GetCameraPos();
        {
            float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&player_.lock()->GetGameObject()->transform_->GetWorldFront())), { 0, 1, 0 }));
            if (len > 1.0f)
            {
                DirectX::XMStoreFloat3(&focusEnemy, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&focusEnemy), DirectX::XMLoadFloat3(&enemyPos), elapsedTime * 2));


                //�����|�W�V���������Ȃ��悤�ɕ␳
                if (abs(cameraPos.x - focusEnemy.x) < 0.01f)
                {
                    focusEnemy.x += 0.01f;
                }

                if (abs(cameraPos.z - focusEnemy.z) < 0.01f)
                {
                    focusEnemy.z += 0.01f;
                }

                playerCameraCom->SetForcusPos(focusEnemy);
            }

            //�J�����̌����Ƀv���C���[�����킹��
            static DirectX::XMFLOAT4 rota = cameraQuaternion.dxFloat4;
            DirectX::XMVECTOR CA = DirectX::XMLoadFloat4(&cameraQuaternion.dxFloat4);
            DirectX::XMStoreFloat4(&rota, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&rota), CA, elapsedTime * 5));

            player_.lock()->GetGameObject()->transform_->SetRotation(rota);

            playerCameraCom->pos = cameraPos;
        }

        //���͏����擾
        GamePad& gamePad = Input::Instance().GetGamePad();
        float ax = gamePad.GetAxisRX();
        float ay = gamePad.GetAxisRY();

        //���ŐF�ς���
        lockTriangleEnemy_.lock()->GetComponent<RendererCom>()->GetModel()->SetMaterialColor({ 1,15,1,1 });

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
            for (auto& enemy : EnemyManager::Instance().GetNearEnemies())
            {
                //���ŐF�ς���
                enemy.enemy.lock()->GetComponent<RendererCom>()->GetModel()->SetMaterialColor({ 1,1,1,1 });

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
                        lockTriangleEnemy_ = enemy.enemy;
                    }
                }
            }
            lockEnemySeconds = 0;
        }

        //���Ŏ��̃X�e�[�g
        if (gamePad.GetButtonDown() & GamePad::BTN_Y)
        {
            Graphics::Instance().SetWorldSpeed(1);
            EnemyManager::Instance().SetIsUpdateFlag(true);       
            //���ŐF�߂�
            lockTriangleEnemy_.lock()->GetComponent<RendererCom>()->GetModel()->SetMaterialColor({ 1,1,1,1 });


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

            triangleState_++;
        }
    }
    break;
    //����̈ʒu�Ƀv���C���[�s���A�J��������ɂ��Ă���
    case 4:
    {
        DirectX::XMFLOAT3 pos = player_.lock()->GetGameObject()->transform_->GetWorldPosition();
        pos.y = 5;
        player_.lock()->GetGameObject()->transform_->SetWorldPosition(pos);
        std::shared_ptr<GameObject> pushObj = GameObjectManager::Instance().Find("Push");
        //���킪�G�ɓ��B������
        if (!pushObj->GetComponent<PushWeaponCom>()->IsMove())
        {
            JustInisialize();

            //�w��G�l�~�[�擾
            std::shared_ptr<GameObject> lockEnemy = lockTriangleEnemy_.lock();
            DirectX::XMFLOAT3 enemyPos = lockEnemy->transform_->GetWorldPosition();
            //DirectX::XMFLOAT3 enemyPos = justHitEnemy_.lock()->transform_->GetWorldPosition();
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

            playerCameraCom->isJust = false;

            std::shared_ptr<GameObject> candyObj = GameObjectManager::Instance().Find("CandyPush");
            std::shared_ptr<WeaponCom> weapon = candyObj->GetComponent<WeaponCom>();
            weapon->SetIsForeverUse(false);

            justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
        }
    }
    break;
    }
}

void JustAvoidPlayer::JustSpriteUpdate(float elapsedTime)
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
            justSpriteState_ = 20;
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
        isJustSprite_ = false;
        justSpriteState_ = -1;
    }
    break;
    }
}

//�W���X�g����o����������
void JustAvoidPlayer::JustAvoidJudge()
{
    //�W���X�g����̓����蔻��
    std::vector<HitObj> hitGameObj = player_.lock()->GetGameObject()->GetComponent<Collider>()->OnHitGameObject();
    //�W���X�g��������G�l�~�[��ۑ�
    std::weak_ptr<GameObject> enemy;
    for (auto& hitObj : hitGameObj)
    {
        if (COLLIDER_TAG::JustAvoid != hitObj.gameObject.lock()->GetComponent<Collider>()->GetMyTag())continue;

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
    //���G����
    player_.lock()->GetGameObject()->GetComponent<CharacterStatusCom>()
        ->SetInvincibleNonDamage(2);
    isJustJudge_ = true;
    justAvoidState_ = 0;
}

void JustAvoidPlayer::JustAvoidDirection(float elapsedTime)
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

    justSprite_->Render(dc, 0, 0, graphics.GetScreenWidth(), graphics.GetScreenHeight()
        , 0, 0, justSprite_->GetTextureWidth(), justSprite_->GetTextureHeight()
        , 0, 1, 0.89f, 0, 1
    );
}
