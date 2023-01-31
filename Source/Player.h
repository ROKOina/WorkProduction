#pragma once

#include "Graphics/Shader.h"
#include "Graphics/Model.h"
#include "Character.h"
#include "ProjectileManager.h"
#include "Effect.h"

//�v���C���[
class Player : public Character
{
public: //�֐�
    Player();
    ~Player() override;

    //�C���X�^���X�擾
    static Player& Instance();

    //�X�V����
    void Update(float elapsedTime);

    //�`�揈��
    void Render(ID3D11DeviceContext* dc, Shader* shader);

    //�v���C���[�ƃG�l�~�[�Ƃ̏Փˏ���
    void CollisionPlayerVsEnemies();

    //�e�ۂƓG�̏Փˏ���
    void CollisionProjectileVsEnemies();

    //�f�o�b�O�pGUI�`��
    void DrawDebugGUI();

    //�f�o�b�O�v���~�e�B�u�`��
    void DrawDebugPrimitive();

    ////�W�����v����
    //void Jump(float speed);

    ////���͏����X�V
    //void UpdateVelocity(float elapsedTime);

    //�W�����v���͏���
    bool InputJump();

private:    //�֐�
    //�X�e�B�b�N���͒l����ړ��x�N�g�����擾
    DirectX::XMFLOAT3 GetMoveVec() const;

    //�ړ����͏���
    bool InputMove(float elapsedTime);

    //�e�ۓ��͏���
    void InputProjectile();

    //�U�����͏���
    bool InputAttack();

    ////���񏈗�
    //void Turn(float elapsedTime, float vx, float vz, float speed);

    //�m�[�h�ƃG�l�~�[�̏Փ˔���
    void CollisionNodeVsEnemies(const char* nodeName, float nodeRadius);

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
    //float gravity = -1.0f;
    //DirectX::XMFLOAT3 velocity = { 0,0,0 }; //����

    int JumpCount = 0;
    int JumpLimit = 2;

    ProjectileManager projectileManager;

    //�{�[��
    float leftHandRadius = 0.4f;
    bool attackCollisionFlag = false;

    //ImGui
    bool VisibleDebugPrimitive = true;

    //�G�t�F�N�g
    Effect* hitEffect = nullptr;

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
        Anim_Attack,
        Anim_Death,
        Anim_Falling,
        Anim_GetHit1,
        Anim_GetHit2,
        Anim_Idle,
        Anim_Jump,
        Anim_Jump_Flip,
        Anim_Landing,
        Anim_Revive,
        Anim_Running,
        Anim_Walking 
    };
};