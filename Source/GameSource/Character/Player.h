#pragma once

#include "Graphics/Shader.h"
#include "Graphics/Model.h"
#include "GameSource\Character\Character.h"
#include "GameSource\Render\Effect\Effect.h"
#include "GameSource\Camera\CameraController.h"
#include "GameSource\Weapon\Weapon.h"

//�v���C���[
class Player : public Character
{
public: //�֐�
    Player(CameraController* camera);
    ~Player() override;

    //�C���X�^���X�擾
    static Player& Instance();

    //�X�V����
    void Update(float elapsedTime);

    //�`�揈��
    void Render(ID3D11DeviceContext* dc, Shader* shader);

    //�f�o�b�O�pGUI�`��
    void DrawDebugGUI();

    //�f�o�b�O�v���~�e�B�u�`��
    void DrawDebugPrimitive();

    //�W�����v���͏���
    bool InputJump();

private:    //�֐�
    //�X�e�B�b�N���͒l����ړ��x�N�g�����擾
    DirectX::XMFLOAT3 GetMoveVec() const;

    //�ړ����͏���
    bool InputMove(float elapsedTime);

    //�U�����͏���
    bool InputAttack();

protected:
    //���n�������ɌĂ΂��
    void OnLanding()override;

    //�_���[�W���󂯂����ɌĂ΂��
    void OnDamage()override;

    //���S�������ɌĂ΂��
    void OnDead()override;

private:    //�ϐ�
    Model* model = nullptr;
    float moveSpeed = 5.0f;
    float turnSpeed = DirectX::XMConvertToRadians(720);

    //�W�����v
    float jumpSpeed = 20.0f;

    int JumpCount = 0;
    int JumpLimit = 2;

    //�{�[��
    float leftHandRadius = 0.4f;
    bool attackCollisionFlag = false;

    //ImGui
    bool VisibleDebugPrimitive = true;

    //�G�t�F�N�g
    Effect* hitEffect = nullptr;

    std::unique_ptr<CameraController> cameraController;

    std::unique_ptr<Weapon> w;

private:   //�A�j���[�V����
    //�J��
    //�ҋ@�X�e�[�g�֑J��
    void TransitionIdleState();
    //�ړ��X�e�[�g�֑J��
    void TransitionMoveState();
    //�W�����v�X�e�[�g�֑J��
    void TransitionJumpState();
    //���n�X�e�[�g�֑J��
    void TransitionLandState();
    //�U���X�e�[�g�֑J��
    void TransitionAttackState();
    //�_���[�W�X�e�[�g�֑J��
    void TransitionDamageState();
    //���S�X�e�[�g�֑J��
    void TransitionDeathState();
    //�����X�e�[�g�֑J��
    void TransitionReviveState();
    
    


    //�X�V����
    //�ҋ@�X�e�[�g�X�V����
    void UpdateIdleState(float elapsedTime);
    //�ړ��X�e�[�g�X�V����
    void UpdateMoveState(float elapsedTime);
    //�W�����v�X�e�[�g�X�V����
    void UpdateJumpState(float elapsedTime);
    //���n�X�e�[�g�X�V����
    void UpdateLandState(float elapsedTime);
    //�U���X�e�[�g�X�V����
    void UpdateAttackState(float elapsedTime);
    //�_���[�W�X�e�[�g�X�V����
    void UpdateDamageState(float elapsedTime);
    //���S�X�e�[�g�X�V����
    void UpdateDeathState(float elapsedTime);
    //�����X�e�[�g�X�V����
    void UpdateReviveState(float elapsedTime);

    enum class State {
        Idle,
        Move,
        Jump,
        Land,
        Attack,
        Damage,
        Death,
        Revive
    };

    State state = State::Idle;

    //�A�j���[�V����
    enum Animation
    {
        Anim_spin,
        Anim_happyWalk,
        Anim_ninjaWalk,
    };
};