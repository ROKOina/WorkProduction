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
    //�W���X�g��𔻒莞
    if (isJustJudge_)
    {
        //�W���X�g����ړ�����
        JustAvoidanceMove(elapsedTime);

        //�W���X�g����㔽�����͊m�F
        JustAvoidanceAttackInput();
    }

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


//�W���X�g���������
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
        player_.lock()->GetMovePlayer()->moveParamType_ = MovePlayer::MOVE_PARAM::JUSTDASH;
        move->SetMoveMaxSpeed(player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::JUSTDASH].moveMaxSpeed);
        move->SetMoveAcceleration(player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::JUSTDASH].moveAcceleration);
        justAvoidTimer_ = justAvoidTime_;

        player_.lock()->GetMovePlayer()->isInputMove_ = false;
        player_.lock()->GetAttackPlayer()->isNormalAttack_ = false;
        player_.lock()->GetMovePlayer()->isDash_ = false;

        //���͕������݂ăA�j���[�V�����Đ�
        DirectX::XMVECTOR Input = DirectX::XMLoadFloat3(&player_.lock()->GetMovePlayer()->inputMoveVec_);

        //�A�j���[�V�����Đ�
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

        //���g�o��
        for (int i = 0; i < 4; ++i)
        {
            std::shared_ptr<AnimationCom> justAnim = justPico[i]->GetComponent<AnimationCom>();
            //������
            justPico[i]->transform_->SetLocalPosition({ 0,0,0 });
            justAnim->SetAnimationSpeed(0.2f);  //�A�j�����x�X���[��
            Model* justModel = justPico[i]->GetComponent<RendererCom>()->GetModel();
            justModel->SetMaterialColor({ 1.2f,1.2f,0.1f,0.65f });  //�F������

            //�o��
            justPico[i]->SetEnabled(true);
            if (inputFlag)
                justAnim->PlayAnimation(25, false);
            else
                justAnim->PlayAnimation(24, false);
        }

        //�G�̕�������
        player_.lock()->GetGameObject()->transform_->LookAtTransform(justHitEnemy_->transform_->GetWorldPosition());

        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::JUST);



        ////�J�����V�F�C�N
        //GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()->CameraShake(0.1f, 0.25f);

        //�q�b�g�X�g�b�v
        GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>()->HitStop(0.2f);

        justAvoidState_++;
        break;
    }
    //���g�ړ��������A�j���X�s�[�h�߂�
    case 1:
    {
        bool endFlag = false;
        DirectX::XMFLOAT3 justPicoPos[4];
        for (int i = 0; i < 4; ++i)
        {
            justPicoPos[i] = justPico[i]->transform_->GetLocalPosition();
        }
        //�c
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
        //��
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

        //�X�e�[�g�I���ɃA�j���X�s�[�h��߂�
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
    //���g����
    //�v���C���[�o��
    case 2:
    {
        bool endFlag = false;
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
        }
        if (endFlag)
            justAvoidState_++;

        break;
    }
    //�����󂯓���
    case 3:
    {
        //�A�j���I���Ŕ����I��
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

        //�W���X�g����I���^�C�}�[
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

    //�ړ��o���邩
    int animIndex = player_.lock()->GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();
    if (animIndex != 25 && animIndex != 24)
    {
        //�ړ����������߂�i�W���X�g��𒆂͏���Ɉړ�����j
        DirectX::XMFLOAT3 Direction;

        DirectX::XMFLOAT3 inputVec = player_.lock()->GetMovePlayer()->inputMoveVec_;
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
        Direction.x *= player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::JUSTDASH].moveSpeed;
        Direction.z *= player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::JUSTDASH].moveSpeed;
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
    if (gamePad.GetButtonDown() & GamePad::BTN_Y)
    {
        JustInisialize();
        player_.lock()->GetMovePlayer()->moveParamType_ = MovePlayer::MOVE_PARAM::DASH;
        move->SetMoveMaxSpeed(player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::DASH].moveMaxSpeed);
        move->SetMoveAcceleration(player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::DASH].moveAcceleration);

        justAvoidKey_ = JUST_AVOID_KEY::SQUARE;

        player_.lock()->GetMovePlayer()->isInputTrun_ = false;

        player_.lock()->SetPlayerStatus(PlayerCom::PLAYER_STATUS::ATTACK);

        //�G�̕�������
        player_.lock()->GetGameObject()->transform_->LookAtTransform(justHitEnemy_->transform_->GetWorldPosition());
    }
}

//������
void JustAvoidPlayer::JustAvoidanceSquare(float elapsedTime)
{
    //�G�ɐڋ߂���
    {
        std::shared_ptr<MovementCom> move = player_.lock()->GetGameObject()->GetComponent<MovementCom>();

        DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&player_.lock()->GetGameObject()->transform_->GetWorldPosition());
        DirectX::XMVECTOR EnemyPos = DirectX::XMLoadFloat3(&justHitEnemy_->transform_->GetWorldPosition());
        DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(EnemyPos, Pos);
        float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(PE));
        //�G�̋߂��܂ňړ�����
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

            //�A�^�b�N�����Ɉ����p��
            player_.lock()->GetAttackPlayer()->animFlagName_ = "squareJust";
        }

        DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(PE);
        Dir = DirectX::XMVectorScale(Dir, player_.lock()->GetMovePlayer()->moveParam_[MovePlayer::MOVE_PARAM::DASH].moveSpeed);
        DirectX::XMFLOAT3 dir;
        DirectX::XMStoreFloat3(&dir, Dir);

        //�͂ɉ�����
        move->AddForce(dir);

    }
}
