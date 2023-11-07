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

    //���������X�V
    switch (justAvoidKey_)
    {
        //������
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


//�W���X�g���������
void JustAvoidPlayer::JustInisialize()
{
    justAvoidState_ = -1;
    isJustJudge_ = false;
    Graphics::Instance().SetWorldSpeed(1);
    //�J���[�O���[�f�B���O�߂�
    Graphics::Instance().shaderParameter3D_.colorGradingData.saturation = 1.5f;

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

        //�v���C���[������
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
    if (justAvoidState_ < 3)return;

    std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

    //�{�^���Ŕ����ς���
    GamePad& gamePad = Input::Instance().GetGamePad();

    //���̏ꍇ
    if (gamePad.GetButtonDown() & GamePad::BTN_X)
    {
        JustInisialize();
        player_.lock()->GetMovePlayer()->SetMoveParamType(MovePlayer::MOVE_PARAM::DASH);

        justAvoidKey_ = JUST_AVOID_KEY::SQUARE;

        player_.lock()->GetMovePlayer()->SetIsInputMove(false);

        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);

        //�G�̕�������
        player_.lock()->GetGameObject()->transform_->LookAtTransform(justHitEnemy_.lock()->transform_->GetWorldPosition());
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
