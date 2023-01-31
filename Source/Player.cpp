#include <imgui.h>
#include "Player.h"
#include "Input/Input.h"
#include "Camera.h"
#include "Graphics/Graphics.h"
#include "EnemyManager.h"
#include "Collision.h"
#include "ProjectileStraite.h"
#include "ProjectileHoming.h"

static Player* instance = nullptr;

//�C���X�^���X�擾
Player& Player::Instance()
{
    return *instance;
}

//�R���X�g���N�^
Player::Player()
{
    //�C���X�^���X�|�C���^�ݒ�
    instance = this;

    model = new Model("Data/Model/Jammo/Jammo.mdl");
    //model->PlayAnimation(0,true);

    //model = new Model("Data/Model/Mr.Incredible/Mr.Incredible.mdl");

    //���f�����傫���̂ŃX�P�[�����O
    scale.x = scale.y = scale.z = 0.01f;

    //�q�b�g�G�t�F�N�g�ǂݍ���
    hitEffect = new Effect("Data/Effect/Hit.efk");

    //�ҋ@�X�e�[�g�֑J��
    TransitionIdleState();
}

//�f�X�g���N�^
Player::~Player()
{
    delete hitEffect;
    delete model;
}


//�X�e�B�b�N���͒l
DirectX::XMFLOAT3 Player::GetMoveVec() const
{
    //���͏����擾
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();

    //�J���������ƃX�e�B�b�N�̓��͒l�ɂ���Đi�s�������v�Z����
    Camera& camera = Camera::Instance();
    const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
    const DirectX::XMFLOAT3& cameraFront = camera.GetFront();

    //�ړ��x�N�g����XZ���ʂɐ����ȃx�N�g���ɂȂ�悤�ɂ���

    //�J�����E�����x�N�g����XZ�P�ʃx�N�g���ɕϊ�
    float cameraRightX = cameraRight.x;
    float cameraRightZ = cameraRight.z;
    float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
    if (cameraRightLength > 0.0f)
    {
        //�P�ʃx�N�g����
        cameraRightX /= cameraRightLength;
        cameraRightZ /= cameraRightLength;
    }

    //�J�����O�����x�N�g����XZ�P�ʃx�N�g���ɕϊ�
    float cameraFrontX = cameraFront.x;
    float cameraFrontZ = cameraFront.z;
    float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
    if (cameraFrontLength > 0.0f)
    {
        //�P�ʃx�N�g����
        cameraFrontX /= cameraFrontLength;
        cameraFrontZ /= cameraFrontLength;
    }

    //�X�e�B�b�N�̐������͒l���J�����E�����ɔ��f���A
    //�X�e�B�b�N�̐������͒l���J�����O�����ɔ��f���A
    //�i�s�x�N�g�����v�Z����
    DirectX::XMFLOAT3 vec;
    vec.x = cameraFrontX * ay + cameraRightX * ax;
    vec.z = cameraFrontZ * ay + cameraRightZ * ax;
    //Y�������ɂ͈ړ����Ȃ�
    vec.y = 0.0f;

    return vec;
}

//�X�V����
void Player::Update(float elapsedTime)
{
    ////�ړ����͏���
    //InputMove(elapsedTime);

    ////�W�����v���͏���
    //InputJump();
    
    ////�e�ۓ��͏���
    //InputProjectile();

    //�X�e�[�g���̏���
    switch (state)
    {
    case State::Idle:
        UpdateIdleState(elapsedTime);
        break;

    case State::Move:
        UpdateMoveState(elapsedTime);
        break;

    case State::Jump:
        UpdateJumpState(elapsedTime);
        break;

    case State::Land:
        UpdateLandState(elapsedTime);
        break;

    case State::Attack:
        UpdateAttackState(elapsedTime);
        break;

    case State::Damage:
        UpdateDamageState(elapsedTime);
        break;

    case State::Death:
        UpdateDeathState(elapsedTime);
        break;

    case State::Revive:
        UpdateReviveState(elapsedTime);
        break;

    }

    //���͏����X�V
    UpdateVelocity(elapsedTime);

    //���G���ԍX�V
    UpdateInvincibleTimer(elapsedTime);

    //�e�ۍX�V����
    projectileManager.Update(elapsedTime);

    //�v���C���[�ƓG�Ƃ̏Փˏ���
    CollisionPlayerVsEnemies();

    //�e�ۂƓG�̏Փ˔���
    CollisionProjectileVsEnemies();

    //�I�u�W�F�N�g�̍s����X�V
    UpdateTransform();

    //���f���A�j���[�V�����X�V����
    model->UpdateAnimetion(elapsedTime);

    //���f���s��X�V
    model->UpdateTransform(transform);  //�ʒu�s���n��
}


bool Player::InputMove(float elapsedTime)
{
    //�i�s�x�N�g���擾
    DirectX::XMFLOAT3 moveVec = GetMoveVec();

    //�ړ�����
    Move(moveVec.x, moveVec.z, moveSpeed);

    //���񏈗�
    Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);

    //�i�s�x�N�g�����[���x�N�g���łȂ��ꍇ�͓��͂��ꂽ
    return moveVec.x * moveVec.x + moveVec.y * moveVec.y + moveVec.z * moveVec.z;
}

//�W�����v���͏���
bool Player::InputJump()
{
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (gamePad.GetButtonDown() & GamePad::BTN_A) 
    {
        if (JumpCount < JumpLimit)  //�W�����v�񐔐���
        {
            Jump(jumpSpeed);
            ++JumpCount;

            //�W�����v���͂���
            return true;
        }
    }
    return false;
}

//�U�����͏���
bool Player::InputAttack()
{
    GamePad& gamePad = Input::Instance().GetGamePad();

    if (gamePad.GetButtonDown() & GamePad::BTN_B)
    {
        return true;
    }
    return false;
}

//���n�������ɌĂ΂��
void Player::OnLanding()
{
    JumpCount = 0;

    //�_���[�W�A���S�X�e�[�g���͒��n�������ɃX�e�[�g�J�ڂ��Ȃ��悤�ɂ���
    if (state != State::Damage && state != State::Death)
    {
        //���n�X�e�[�g�֑J��
        TransitionLandState();
    }
}

//�_���[�W���󂯂����ɌĂ΂��
void Player::OnDamage()
{
    //�_���[�W�X�e�[�g�֑J��
    TransitionDamageState();
}

//���S�������ɌĂ΂��
void Player::OnDead()
{
    //���S�������ɌĂ΂��
    TransitionDeathState();
}

////�W�����v����
//void Player::Jump(float speed)
//{
//    //������̗͂�ݒ�
//    velocity.y = speed;
//}
//
////���͏����X�V
//void Player::UpdateVelocity(float elapsedTime)
//{
//    //�o�߃t���[��
//    float elapsedFrame = 60.0f * elapsedTime;
//
//    //�d�͏���
//    velocity.y += gravity * elapsedFrame;
//
//    //�ړ�����
//    position.y += velocity.y * elapsedTime;
//
//    //�n�ʔ���
//    if (position.y < 0.0f)
//    {
//        position.y = 0.0f;
//        velocity.y = 0.0f;
//    }
//}
//
////���񏈗�
//void Player::Turn(float elapsedTime, float vx, float vz, float speed)
//{
//    speed *= elapsedTime;
//
//
//    //�i�s�x�N�g����P�ʃx�N�g����
//    float moveVecLength = sqrtf(vx * vx + vz * vz);
//    if (moveVecLength <= 0)return;  //�i�s�x�N�g�����[���x�N�g���̏ꍇ�͏�������K�v�Ȃ�
//    if (moveVecLength > 0.0f)
//    {
//        //�P�ʃx�N�g����
//        vx /= moveVecLength;
//        vz /= moveVecLength;
//    }
//
//    //���g�̉�]�l����O���������߂�
//    float frontX = sinf(angle.y);
//    float frontZ = cosf(angle.y);
//
//    //��]�p�����߂邽�߁A�Q�̒P�ʃx�N�g���̓��ς��v�Z����
//    float dot = (vx * frontX) + (vz * frontZ);
//
//    //���ϒl��-1.0~1.0�ŕ\������Ă���A�Q�̒P�ʃx�N�g���̊p�x��
//    //�������قǂ�1.0�ɋ߂Â��Ƃ��������𗘗p���ĉ�]���x�𒲐�����
//    float rot = 1.0f - dot; //�␳��
//    if (rot > speed)rot = speed;
//
//    //���E������s�����߂�2�̒P�ʃx�N�g���̊O�ς��v�Z����
//    float cross = (vx * frontZ) - (vz * frontX);
//
//    //2D�̊O�ϒl�����̏ꍇ�����̏ꍇ�ɂ���č��E���肪�s����
//    //���E������s�����Ƃɂ���č��E��]��I������
//    if (cross < 0.0f)   //�E
//    {
//        angle.y -= rot;
//    }
//    else
//    {
//        angle.y += rot;
//    }
//
//}


//�e�ۓ��͏���
void Player::InputProjectile()
{
    GamePad& gamePad = Input::Instance().GetGamePad();

    //���i�e�۔���
    if (gamePad.GetButtonDown() & GamePad::BTN_X|| gamePad.GetButtonDown() & GamePad::BTN_Y)    //C:�܂������AV:�ǔ�
    {
        //�O����
        DirectX::XMFLOAT3 dir;  //Z������transform������
        dir.x = sinf(angle.y);
        dir.y = 0;
        dir.z = cosf(angle.y);
        //���ˈʒu�i�v���C���[�̍�������j
        DirectX::XMFLOAT3 pos;
        pos.x = position.x;
        pos.y = position.y + GetHeight() / 2;
        pos.z = position.z;
        if (gamePad.GetButtonDown() & GamePad::BTN_Y) {
            //�^�[�Q�b�g(�f�t�H���g�ł̓v���C���[�̑O��
            DirectX::XMFLOAT3 target;
            target.x = position.x * 2;
            target.y = position.y + GetHeight() / 2 * 2;
            target.z = position.z * 2;

            //��ԋ߂��̓G���^�[�Q�b�g�ɂ���
            EnemyManager& enemyManager = EnemyManager::Instance();
            int enemyCount = enemyManager.GetEnemyCount();  //�G�̐��������Ă���
            if (enemyCount > 0) //�G�������ꍇ
            {
                for (int e = 0; e < enemyCount; ++e)    //�G�̐��������肩����
                {
                    if (e == 0) //�P�̖ڂ����^�[�Q�b�g�ɓ���Ė߂�
                    {
                        target = enemyManager.GetEnemy(e)->GetPosition();    //target�ɓ����
                        continue;
                    }

                    //�G�̃x�N�g���擾
                    DirectX::XMVECTOR enemy0 = DirectX::XMLoadFloat3(&target);
                    //���̓G�̃x�N�g���擾
                    DirectX::XMVECTOR enemy = DirectX::XMLoadFloat3(&enemyManager.GetEnemy(e)->GetPosition());
                    //�L�����̃x�N�g���擾
                    DirectX::XMVECTOR ziki = DirectX::XMLoadFloat3(&pos);
                    //�傫���𑪂�
                    DirectX::XMVECTOR E0Z = DirectX::XMVectorSubtract(enemy0, ziki);
                    E0Z = DirectX::XMVector3Length(E0Z);
                    float length0;
                    DirectX::XMStoreFloat(&length0, E0Z);
                    DirectX::XMVECTOR EZ = DirectX::XMVectorSubtract(enemy, ziki);
                    EZ = DirectX::XMVector3Length(EZ);
                    float length;
                    DirectX::XMStoreFloat(&length, EZ);

                    //�߂������^�[�Q�b�g�ɂ���
                    if (length < length0)target = enemyManager.GetEnemy(e)->GetPosition();

                }
            }
        //����
        ProjectileHoming* projectile = new ProjectileHoming(&projectileManager);
        projectile->Launch(dir, pos,target);
        }
        else
        {
            //����
            ProjectileStraite* projectile = new ProjectileStraite(&projectileManager);
            projectile->Launch(dir, pos);
        }
        ////projectileManager.Register(projectile);
    }
}

//�v���C���[�Ƃ̓����蔻��
void Player::CollisionPlayerVsEnemies()
{
    EnemyManager& enemyManager = EnemyManager::Instance();

    //�S�Ă̐��Ƒ�������ŏՓˏ���
    int enemyCount = enemyManager.GetEnemyCount();
    for (int i = 0; i < enemyCount; ++i)
    {
        Enemy* enemy = enemyManager.GetEnemy(i);

        //�Փˏ���
        DirectX::XMFLOAT3 outPosition;
        //if (Collision::IntersectSphereVsSphere(   //�~
        //    this->GetPosition(), this->GetRadius(),
        //    enemy->GetPosition(), enemy->GetRadius(),
        //    outPosition))
        //{
        //    //�����o����̈ʒu�ݒ�
        //    enemy->SetPosition(outPosition);
        //}

        if (Collision::IntersectCylinder(
            this->GetPosition(), this->GetRadius(), this->GetHeight(),
            enemy->GetPosition(), enemy->GetRadius(), enemy->GetHeight(),
            outPosition
        ))
        {
            DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&this->GetPosition());
            DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->GetPosition());
            DirectX::XMVECTOR EP = DirectX::XMVectorSubtract(P, E);
            DirectX::XMVECTOR N = DirectX::XMVector3Normalize(EP);

            //�����o����̈ʒu�ݒ�
            if(DirectX::XMVectorGetY(N)>0.8f) Jump(10);
            /*if(IsGround())*/
            else enemy->SetPosition(outPosition);
        }
    }
}

//�e�ۂƓG�̏Փˏ���
void Player::CollisionProjectileVsEnemies()
{
    EnemyManager& enemyManager = EnemyManager::Instance();

    //�S�Ă̒e�ۂƑS�Ă̓G�𑍓�����ŏՓˏ���
    int projectileCount = projectileManager.GetProjectileCount();
    int enemyCount = enemyManager.GetEnemyCount();
    for (int i = 0; i < projectileCount; ++i)
    {
        Projectile* projectile = projectileManager.GetProjectile(i);
        for (int j = 0; j < enemyCount; ++j)
        {
            Enemy* enemy = enemyManager.GetEnemy(j);
            //�Փˏ���
            DirectX::XMFLOAT3 outPosition;
            if (Collision::IntersectSphereVsCylider(
                projectile->GetPosition(),
                projectile->GetRadius(),
                enemy->GetPosition(),
                enemy->GetRadius(),
                enemy->GetHeight(),
                outPosition))
            {
                //�_���[�W��^����
                if (enemy->ApplyDamage(1, 0.5f))
                {
                    //������΂�
                    {
                        DirectX::XMFLOAT3 impulse;

                        DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&projectile->GetPosition());
                        DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&enemy->GetPosition());
                        DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);  //�����Z�ŕ��������߂�a->b
                        Vec = DirectX::XMVector3Normalize(Vec);
                        Vec = DirectX::XMVectorScale(Vec, 10.0f);   //���ƓG�̋����𐁂���΂�
                        DirectX::XMStoreFloat3(&impulse, Vec);

                        impulse.y = 3.0f;//�����W�����v
                        enemy->AddImpulse(impulse);
                    }

                        //�q�b�g�G�t�F�N�g�Đ�
                    {
                        DirectX::XMFLOAT3 e = enemy->GetPosition();
                        e.y += enemy->GetHeight() * 0.5f;
                        hitEffect->Play(e);
                    }

                    //�e�۔j��
                    projectile->Destroy();
                }
            }
        }
    }
}

//�`��X�V
void Player::Render(ID3D11DeviceContext* dc, Shader* shader)
{
    shader->Draw(dc, model);

    //�e�ە`�揈��
    projectileManager.Render(dc, shader);
}

//�f�o�b�O�v���~�e�B�u�`��
void Player::DrawDebugPrimitive()
{
    if (!VisibleDebugPrimitive)return;
    DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();

    //�Փ˔���p�̃f�o�b�O����`��
    //debugRenderer->DrawSphere(position, radius, DirectX::XMFLOAT4(0, 0, 0, 1));  
    //�Փ˔���p�̃f�o�b�O�~����`��
    debugRenderer->DrawCylinder(position, radius, height, DirectX::XMFLOAT4(0, 0, 0, 1));  
    
    //�e�ۃf�o�b�O�v���~�e�B�u�`��
    projectileManager.DrawDebugPrimitive();

    //�U���Փ˗p�̍���m�[�h�̃f�o�b�O����`��
    if (attackCollisionFlag)
    {
        Model::Node* leftHandBone = model->FindNode("mixamorig:LeftHand");
        debugRenderer->DrawSphere(DirectX::XMFLOAT3(
            leftHandBone->worldTransform._41,
            leftHandBone->worldTransform._42,
            leftHandBone->worldTransform._43),
            leftHandRadius,
            DirectX::XMFLOAT4(1, 0, 0, 1));
    }
}

//�f�o�b�O�pGUI�`��
void Player::DrawDebugGUI()
{
    //ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    //ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

    //if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
    if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("VisibleDebugPrimitive", &VisibleDebugPrimitive);
            //�ʒu
            ImGui::InputFloat3("Position", &position.x);
            //��]
            DirectX::XMFLOAT3 a;
            a.x = DirectX::XMConvertToDegrees(angle.x);
            a.y = DirectX::XMConvertToDegrees(angle.y);
            a.z = DirectX::XMConvertToDegrees(angle.z);
            ImGui::InputFloat3("Angle", &a.x);
            angle.x = DirectX::XMConvertToRadians(a.x);
            angle.y = DirectX::XMConvertToRadians(a.y);
            angle.z = DirectX::XMConvertToRadians(a.z);
            //�X�P�[��
            ImGui::InputFloat3("Scale", &scale.x);
            //speed
            ImGui::InputFloat3("velocity", &velocity.x);
            ImGui::SliderFloat("MoveSpeed", &moveSpeed, 0, 100);
            ImGui::SliderFloat("TurnSpeed", &turnSpeed, 0, 100);
            ImGui::InputFloat("Radius", &radius);
            ImGui::InputFloat("Height", &height);
            ImGui::InputFloat("Friction", &friction);
            ImGui::InputFloat("Acceleration", &acceleration);
            ImGui::InputFloat("AirControl", &airControl);
    }
    //ImGui::End();
}

//�m�[�h�ƃG�l�~�[�̏Փ˔���
void Player::CollisionNodeVsEnemies(const char* nodeName, float nodeRadius)
{
    //�m�[�h�擾
    DirectX::XMFLOAT3 nodePosition;
    Model::Node* n = model->FindNode(nodeName);
    nodePosition.x = n->worldTransform._41;
    nodePosition.y = n->worldTransform._42;
    nodePosition.z = n->worldTransform._43;


    //�w��̃m�[�h�ƑS�Ă̓G�𑍓�����ŏՓ˔���
    EnemyManager& enemyManager = EnemyManager::Instance();

    //�S�Ă̐��Ƒ�������ŏՓˏ���
    int enemyCount = enemyManager.GetEnemyCount();
    for (int i = 0; i < enemyCount; ++i)
    {
        Enemy* enemy = enemyManager.GetEnemy(i);

        //�Փˏ���
        DirectX::XMFLOAT3 outPosition;
        if (Collision::IntersectSphereVsCylider(   //�~
            nodePosition, nodeRadius,
            enemy->GetPosition(), enemy->GetRadius(),
            enemy->GetHeight(),
            outPosition))
        {
            //�_���[�W��^����
            if (enemy->ApplyDamage(1, 0.5f))
            {
                //������΂�
                {
                    DirectX::XMFLOAT3 impulse;

                    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&nodePosition);
                    DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&enemy->GetPosition());
                    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);  //�����Z�ŕ��������߂�a->b
                    Vec = DirectX::XMVector3Normalize(Vec);
                    Vec = DirectX::XMVectorScale(Vec, 10.0f);   //���ƓG�̋����𐁂���΂�
                    DirectX::XMStoreFloat3(&impulse, Vec);

                    impulse.y = 3.0f;//�����W�����v
                    enemy->AddImpulse(impulse);
                }

                //�q�b�g�G�t�F�N�g�Đ�
                {
                    DirectX::XMFLOAT3 e = enemy->GetPosition();
                    e.y += enemy->GetHeight() * 0.5f;
                    hitEffect->Play(e);
                }
            }
        }

        //if (Collision::IntersectCylinder(
        //    this->GetPosition(), this->GetRadius(), this->GetHeight(),
        //    enemy->GetPosition(), enemy->GetRadius(), enemy->GetHeight(),
        //    outPosition
        //))
        //{
        //    DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&this->GetPosition());
        //    DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->GetPosition());
        //    DirectX::XMVECTOR EP = DirectX::XMVectorSubtract(P, E);
        //    DirectX::XMVECTOR N = DirectX::XMVector3Normalize(EP);

        //    //�����o����̈ʒu�ݒ�
        //    if (DirectX::XMVectorGetY(N) > 0.8f) Jump(10);
        //    /*if(IsGround())*/
        //    else enemy->SetPosition(outPosition);
        //}
    }

}

//�A�j���[�V����
//�ҋ@�X�e�[�g�֑J��
void Player::TransitionIdleState()
{
    state = State::Idle;
    
    //�ҋ@�A�j���[�V�����Đ�
    model->PlayAnimation(Anim_Idle, true);
}

//�ҋ@�X�e�[�g�X�V����
void Player::UpdateIdleState(float elapsedTime)
{
    //�ړ����͏���
    if (InputMove(elapsedTime))TransitionMoveState();

    //�W�����v���͏���
    if (InputJump())TransitionJumpState();

    //�e�ۓ��͏���
    InputProjectile();

    //�U�����͏���
    if (InputAttack())TransitionAttackState();
}

//�ړ��X�e�[�g�֑J��
void Player::TransitionMoveState()
{
    state = State::Move;

    //����A�j���[�V�����Đ�
    model->PlayAnimation(Anim_Running, true);
}

//�ړ��X�e�[�g�X�V����
void Player::UpdateMoveState(float elapsedTime)
{
    //�ړ����͏���
    if (!InputMove(elapsedTime))TransitionIdleState();

    //�W�����v���͏���
    if (InputJump())TransitionJumpState();

    //�e�ۓ��͏���
    InputProjectile();

    //�U�����͏���
    if (InputAttack())TransitionAttackState();
}

//�W�����v�X�e�[�g�֑J��
void Player::TransitionJumpState()
{
    state = State::Jump;

    //�W�����v�A�j���[�V�����Đ�
    model->PlayAnimation(Anim_Jump, false);
}

//�W�����v�X�e�[�g�X�V����
void Player::UpdateJumpState(float elapsedTime)
{
    //�ړ����͏���
    InputMove(elapsedTime);

    //�W�����v���͏���
    InputJump();

    //�e�ۓ��͏���
    InputProjectile();

    //�A�j���[�V�����Đ��I��������Falling�ɂ���
    if (!model->IsPlayAnimation())model->PlayAnimation(Anim_Falling, true);
}

//���n�X�e�[�g�֑J��
void Player::TransitionLandState()
{
    state = State::Land;

    //�W�����v�A�j���[�V�����Đ�
    model->PlayAnimation(Anim_Landing, false);
}

//���n�X�e�[�g�X�V����
void Player::UpdateLandState(float elapsedTime)
{
    //�W�����v���͏���
    if (InputJump())TransitionJumpState();

    //�e�ۓ��͏���
    InputProjectile();

    //�A�j���[�V�����Đ��I��������Falling�ɂ���
    if (!model->IsPlayAnimation())TransitionIdleState();
}

//�U���X�e�[�g�֑J��
void Player::TransitionAttackState()
{
    state = State::Attack;

    //�W�����v�A�j���[�V�����Đ�
    model->PlayAnimation(Anim_Attack, false);
}

//�U���X�e�[�g�X�V����
void Player::UpdateAttackState(float elapsedTime)
{
    //�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݏՓ˔��菈��������
    float animationTime = model->GetCurrentAnimationSecoonds();
    attackCollisionFlag = animationTime > 0.31f;
    if (attackCollisionFlag)
    {
        //����m�[�h�ƃG�l�~�[�̏Փˏ���
        CollisionNodeVsEnemies("mixamorig:LeftHand", leftHandRadius);
    }

    //�A�j���[�V�����Đ��I��������ҋ@�ɂ���
    if (!model->IsPlayAnimation())
    {
        TransitionIdleState();
        attackCollisionFlag = false;
    }
}

//�_���[�W�X�e�[�g�֑J��
void Player::TransitionDamageState()
{
    state = State::Damage;

    //�_���[�W�A�j���[�V�����Đ�
    model->PlayAnimation(Anim_GetHit1, false);
}

//�_���[�W�X�e�[�g�X�V����
void Player::UpdateDamageState(float elapsedTime)
{
    //�A�j���[�V�����Đ��I��������ҋ@�ɂ���
    if (!model->IsPlayAnimation())
    {
        TransitionIdleState();
    }
}

//���S�X�e�[�g�֑J��
void Player::TransitionDeathState()
{
    state = State::Death;

    //���S�A�j���[�V�����Đ�
    model->PlayAnimation(Anim_Death, false);
}

//���S�X�e�[�g�X�V����
void Player::UpdateDeathState(float elapsedTime)
{
    if (!model->IsPlayAnimation())
    {
        //�{�^�����������畜��
        GamePad& gamePad = Input::Instance().GetGamePad();
        if (gamePad.GetButtonDown() & GamePad::BTN_A)
        {
            TransitionReviveState();
        }
    }
}

//�����X�e�[�g�֑J��
void Player::TransitionReviveState()
{
    state = State::Revive;

    //�̗͉�
    health = maxHealth;

    //�����A�j���[�V�����Đ�
    model->PlayAnimation(Anim_Revive, false);
}

//�����X�e�[�g�X�V����
void Player::UpdateReviveState(float elapsedTime)
{
    if (!model->IsPlayAnimation())
    {
        TransitionIdleState();
    }
}