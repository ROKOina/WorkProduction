//#include <imgui.h>
//#include "Player.h"
//#include "Input/Input.h"
//#include "Graphics/Graphics.h"
//#include "GameSource\Math\Collision.h"
//
//static Player* instance = nullptr;
//
////�C���X�^���X�擾
//Player& Player::Instance()
//{
//    return *instance;
//}
//
////�R���X�g���N�^
//Player::Player(CameraController* camera)
//    :cameraController(camera)
//{
//    //�C���X�^���X�|�C���^�ݒ�
//    instance = this;
//
//    model = new Model("Data/Model/pico/picoAnim.mdl");
//    model->PlayAnimation(0,true);
//
//    //���f�����傫���̂ŃX�P�[�����O
//    scale.x = scale.y = scale.z = 0.02f;
//
//    //�q�b�g�G�t�F�N�g�ǂݍ���
//    hitEffect = new Effect("Data/Effect/Hit.efk");
//
//    w = std::make_unique<Weapon>("Data/Model/Swords/BigSword.mdl", DirectX::XMFLOAT3{ 0.01f,0.01f,0.01f });
//    w->SetAngle({ -90,0,0 });
//
//    //�ҋ@�X�e�[�g�֑J��
//    TransitionIdleState();
//}
//
////�f�X�g���N�^
//Player::~Player()
//{
//    delete hitEffect;
//    delete model;
//}
//
//
////�X�e�B�b�N���͒l
//DirectX::XMFLOAT3 Player::GetMoveVec() const
//{
//    //���͏����擾
//    GamePad& gamePad = Input::Instance().GetGamePad();
//    float ax = gamePad.GetAxisLX();
//    float ay = gamePad.GetAxisLY();
//
//    //�J���������ƃX�e�B�b�N�̓��͒l�ɂ���Đi�s�������v�Z����
//    std::shared_ptr<CameraCom> camera = cameraController->GetCamera();
//    const DirectX::XMFLOAT3& cameraRight = camera->GetRight();
//    const DirectX::XMFLOAT3& cameraFront = camera->GetFront();
//
//    //�ړ��x�N�g����XZ���ʂɐ����ȃx�N�g���ɂȂ�悤�ɂ���
//
//    //�J�����E�����x�N�g����XZ�P�ʃx�N�g���ɕϊ�
//    float cameraRightX = cameraRight.x;
//    float cameraRightZ = cameraRight.z;
//    float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
//    if (cameraRightLength > 0.0f)
//    {
//        //�P�ʃx�N�g����
//        cameraRightX /= cameraRightLength;
//        cameraRightZ /= cameraRightLength;
//    }
//
//    //�J�����O�����x�N�g����XZ�P�ʃx�N�g���ɕϊ�
//    float cameraFrontX = cameraFront.x;
//    float cameraFrontZ = cameraFront.z;
//    float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
//    if (cameraFrontLength > 0.0f)
//    {
//        //�P�ʃx�N�g����
//        cameraFrontX /= cameraFrontLength;
//        cameraFrontZ /= cameraFrontLength;
//    }
//
//    //�X�e�B�b�N�̐������͒l���J�����E�����ɔ��f���A
//    //�X�e�B�b�N�̐������͒l���J�����O�����ɔ��f���A
//    //�i�s�x�N�g�����v�Z����
//    DirectX::XMFLOAT3 vec;
//    vec.x = cameraFrontX * ay + cameraRightX * ax;
//    vec.z = cameraFrontZ * ay + cameraRightZ * ax;
//    //Y�������ɂ͈ړ����Ȃ�
//    vec.y = 0.0f;
//
//    return vec;
//}
//
////�X�V����
//void Player::Update(float elapsedTime)
//{
//    //�X�e�[�g���̏���
//    switch (state)
//    {
//    case State::Idle:
//        UpdateIdleState(elapsedTime);
//        break;
//
//    case State::Move:
//        UpdateMoveState(elapsedTime);
//        break;
//
//    case State::Jump:
//        UpdateJumpState(elapsedTime);
//        break;
//
//    case State::Land:
//        UpdateLandState(elapsedTime);
//        break;
//
//    case State::Attack:
//        UpdateAttackState(elapsedTime);
//        break;
//
//    case State::Damage:
//        UpdateDamageState(elapsedTime);
//        break;
//
//    case State::Death:
//        UpdateDeathState(elapsedTime);
//        break;
//
//    case State::Revive:
//        UpdateReviveState(elapsedTime);
//        break;
//
//    }
//
//    //���͏����X�V
//    UpdateVelocity(elapsedTime);
//
//    //���G���ԍX�V
//    UpdateInvincibleTimer(elapsedTime);
//
//    //�I�u�W�F�N�g�̍s����X�V
//    UpdateTransform();
//
//    //���f���A�j���[�V�����X�V����
//    model->UpdateAnimation(elapsedTime);
//
//    //���f���s��X�V
//    model->UpdateTransform(transform);  //�ʒu�s���n��
//
//
//    Model::Node* rightHandBone = model->FindNode("RightHand");
//    w->ParentTransform(rightHandBone->worldTransform);
//    w->Update(elapsedTime);
//}
//
//
//bool Player::InputMove(float elapsedTime)
//{
//    //�i�s�x�N�g���擾
//    DirectX::XMFLOAT3 moveVec = GetMoveVec();
//
//    //�ړ�����
//    Move(moveVec.x, moveVec.z, moveSpeed);
//
//    //���񏈗�
//    Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);
//
//    //�i�s�x�N�g�����[���x�N�g���łȂ��ꍇ�͓��͂��ꂽ
//    return moveVec.x * moveVec.x + moveVec.y * moveVec.y + moveVec.z * moveVec.z;
//}
//
////�W�����v���͏���
//bool Player::InputJump()
//{
//    GamePad& gamePad = Input::Instance().GetGamePad();
//    if (gamePad.GetButtonDown() & GamePad::BTN_A) 
//    {
//        if (JumpCount < JumpLimit)  //�W�����v�񐔐���
//        {
//            Jump(jumpSpeed);
//            ++JumpCount;
//
//            //�W�����v���͂���
//            return true;
//        }
//    }
//    return false;
//}
//
////�U�����͏���
//bool Player::InputAttack()
//{
//    GamePad& gamePad = Input::Instance().GetGamePad();
//
//    if (gamePad.GetButtonDown() & GamePad::BTN_B)
//    {
//        return true;
//    }
//    return false;
//}
//
////���n�������ɌĂ΂��
//void Player::OnLanding()
//{
//    JumpCount = 0;
//
//    //�_���[�W�A���S�X�e�[�g���͒��n�������ɃX�e�[�g�J�ڂ��Ȃ��悤�ɂ���
//    if (state != State::Damage && state != State::Death)
//    {
//        //���n�X�e�[�g�֑J��
//        TransitionLandState();
//    }
//}
//
////�_���[�W���󂯂����ɌĂ΂��
//void Player::OnDamage()
//{
//    //�_���[�W�X�e�[�g�֑J��
//    TransitionDamageState();
//}
//
////���S�������ɌĂ΂��
//void Player::OnDead()
//{
//    //���S�������ɌĂ΂��
//    TransitionDeathState();
//}
//
////�`��X�V
//void Player::Render(ID3D11DeviceContext* dc, Shader* shader)
//{
//    shader->Draw(dc, model);
//    w->Render(dc, shader);
//}
//
////�f�o�b�O�v���~�e�B�u�`��
//void Player::DrawDebugPrimitive()
//{
//    if (!VisibleDebugPrimitive)return;
//    DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
//
//    //�Փ˔���p�̃f�o�b�O�~����`��
//    debugRenderer->DrawCylinder(position, radius, height, DirectX::XMFLOAT4(0, 0, 0, 1));  
//
//    //�U���Փ˗p�̍���m�[�h�̃f�o�b�O����`��
//    if (attackCollisionFlag)
//    {
//        Model::Node* leftHandBone = model->FindNode("mixamorig:LeftHand");
//        debugRenderer->DrawSphere(DirectX::XMFLOAT3(
//            leftHandBone->worldTransform._41,
//            leftHandBone->worldTransform._42,
//            leftHandBone->worldTransform._43),
//            leftHandRadius,
//            DirectX::XMFLOAT4(1, 0, 0, 1));
//    }
//}
//
////�f�o�b�O�pGUI�`��
//void Player::DrawDebugGUI()
//{
//    if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
//    {
//        ImGui::Checkbox("VisibleDebugPrimitive", &VisibleDebugPrimitive);
//            //�ʒu
//            ImGui::InputFloat3("Position", &position.x);
//            //��]
//            DirectX::XMFLOAT3 a;
//            a.x = DirectX::XMConvertToDegrees(angle.x);
//            a.y = DirectX::XMConvertToDegrees(angle.y);
//            a.z = DirectX::XMConvertToDegrees(angle.z);
//            ImGui::InputFloat3("Angle", &a.x);
//            angle.x = DirectX::XMConvertToRadians(a.x);
//            angle.y = DirectX::XMConvertToRadians(a.y);
//            angle.z = DirectX::XMConvertToRadians(a.z);
//            //�X�P�[��
//            ImGui::InputFloat3("Scale", &scale.x);
//            //speed
//            ImGui::InputFloat3("velocity", &velocity.x);
//            ImGui::SliderFloat("MoveSpeed", &moveSpeed, 0, 100);
//            ImGui::SliderFloat("TurnSpeed", &turnSpeed, 0, 100);
//            ImGui::InputFloat("Radius", &radius);
//            ImGui::InputFloat("Height", &height);
//            ImGui::InputFloat("Friction", &friction);
//            ImGui::InputFloat("Acceleration", &acceleration);
//            ImGui::InputFloat("AirControl", &airControl);
//    }
//}
//
////�A�j���[�V����
////�ҋ@�X�e�[�g�֑J��
//void Player::TransitionIdleState()
//{
//    state = State::Idle;
//    
//    //�ҋ@�A�j���[�V�����Đ�
//    model->PlayAnimation(Anim_spin, true);
//}
//
////�ҋ@�X�e�[�g�X�V����
//void Player::UpdateIdleState(float elapsedTime)
//{
//    //�ړ����͏���
//    if (InputMove(elapsedTime))TransitionMoveState();
//
//    //�W�����v���͏���
//    if (InputJump())TransitionJumpState();
//
//    //�U�����͏���
//    if (InputAttack())TransitionAttackState();
//}
//
////�ړ��X�e�[�g�֑J��
//void Player::TransitionMoveState()
//{
//    state = State::Move;
//
//    //����A�j���[�V�����Đ�
//    model->PlayAnimation(Anim_happyWalk, true);
//}
//
////�ړ��X�e�[�g�X�V����
//void Player::UpdateMoveState(float elapsedTime)
//{
//    //�ړ����͏���
//    if (!InputMove(elapsedTime))TransitionIdleState();
//
//    //�W�����v���͏���
//    if (InputJump())TransitionJumpState();
//
//    //�U�����͏���
//    if (InputAttack())TransitionAttackState();
//}
//
////�W�����v�X�e�[�g�֑J��
//void Player::TransitionJumpState()
//{
//    state = State::Jump;
//
//    //�W�����v�A�j���[�V�����Đ�
//    model->PlayAnimation(Anim_ninjaWalk, false);
//}
//
////�W�����v�X�e�[�g�X�V����
//void Player::UpdateJumpState(float elapsedTime)
//{
//    //�ړ����͏���
//    InputMove(elapsedTime);
//
//    //�W�����v���͏���
//    InputJump();
//
//    //�A�j���[�V�����Đ��I��������Falling�ɂ���
//    if (!model->IsPlayAnimation())model->PlayAnimation(Anim_happyWalk, true);
//}
//
////���n�X�e�[�g�֑J��
//void Player::TransitionLandState()
//{
//    state = State::Land;
//
//    //�W�����v�A�j���[�V�����Đ�
//    model->PlayAnimation(Anim_happyWalk, false);
//}
//
////���n�X�e�[�g�X�V����
//void Player::UpdateLandState(float elapsedTime)
//{
//    //�W�����v���͏���
//    if (InputJump())TransitionJumpState();
//
//    //�A�j���[�V�����Đ��I��������Falling�ɂ���
//    if (!model->IsPlayAnimation())TransitionIdleState();
//}
//
////�U���X�e�[�g�֑J��
//void Player::TransitionAttackState()
//{
//    state = State::Attack;
//
//    //�W�����v�A�j���[�V�����Đ�
//    model->PlayAnimation(Anim_happyWalk, false);
//}
//
////�U���X�e�[�g�X�V����
//void Player::UpdateAttackState(float elapsedTime)
//{
//    //�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݏՓ˔��菈��������
//    float animationTime = model->GetCurrentAnimationSecoonds();
//    attackCollisionFlag = animationTime > 0.31f;
//
//    //�A�j���[�V�����Đ��I��������ҋ@�ɂ���
//    if (!model->IsPlayAnimation())
//    {
//        TransitionIdleState();
//        attackCollisionFlag = false;
//    }
//}
//
////�_���[�W�X�e�[�g�֑J��
//void Player::TransitionDamageState()
//{
//    state = State::Damage;
//
//    //�_���[�W�A�j���[�V�����Đ�
//    model->PlayAnimation(Anim_happyWalk, false);
//}
//
////�_���[�W�X�e�[�g�X�V����
//void Player::UpdateDamageState(float elapsedTime)
//{
//    //�A�j���[�V�����Đ��I��������ҋ@�ɂ���
//    if (!model->IsPlayAnimation())
//    {
//        TransitionIdleState();
//    }
//}
//
////���S�X�e�[�g�֑J��
//void Player::TransitionDeathState()
//{
//    state = State::Death;
//
//    //���S�A�j���[�V�����Đ�
//    model->PlayAnimation(Anim_happyWalk, false);
//}
//
////���S�X�e�[�g�X�V����
//void Player::UpdateDeathState(float elapsedTime)
//{
//    if (!model->IsPlayAnimation())
//    {
//        //�{�^�����������畜��
//        GamePad& gamePad = Input::Instance().GetGamePad();
//        if (gamePad.GetButtonDown() & GamePad::BTN_A)
//        {
//            TransitionReviveState();
//        }
//    }
//}
//
////�����X�e�[�g�֑J��
//void Player::TransitionReviveState()
//{
//    state = State::Revive;
//
//    //�̗͉�
//    health = maxHealth;
//
//    //�����A�j���[�V�����Đ�
//    model->PlayAnimation(Anim_happyWalk, false);
//}
//
////�����X�e�[�g�X�V����
//void Player::UpdateReviveState(float elapsedTime)
//{
//    if (!model->IsPlayAnimation())
//    {
//        TransitionIdleState();
//    }
//}