#pragma once

#include <memory>
#include <DirectXMath.h>

//�O���錾
class PlayerCom;
class GameObject;
class Sprite;

//�v���C���[�̃W���X�g������Ǘ�
class JustAvoidPlayer
{
public:
    JustAvoidPlayer(std::shared_ptr<PlayerCom> player);
    ~JustAvoidPlayer() {}

    void Update(float elapsedTime);

    void OnGui();

private:
    //�W���X�g���������
    void JustInisialize();
    //�W���X�g��𒆈ړ�
    void JustAvoidanceMove(float elapsedTime);
    //�W���X�g��𔽌��̓��͂�����
    void JustAvoidanceAttackInput();

    //������
    void JustAvoidanceSquare(float elapsedTime);

    //������
    void JustAvoidanceTriangle(float elapsedTime);

    //�W���X�g��𐢊E�F���o
    void JustDirectionUpdate(float elapsedTime);

    //�W���X�g����p�[�e�B�N���Z�b�g
    void SetJustUnderParticle(bool flag);

public:
    //�W���X�g����o����������
    void JustAvoidJudge();

    //�W���X�g������J�n����
    void StartJustAvoid();

    //�Q�b�^�[���Z�b�^�[
    const std::weak_ptr<GameObject> GetJustHitEnemy() const { return justHitEnemy_; }

    const bool& GetIsJustJudge() const { return isJustJudge_; }

    //�W���X�g��𒆉��o�p
    void JustAvoidDirectionEnd(float elapsedTime);

    //�W���X�g������o�`��
    void justDirectionRender2D();

private:
    //�W���X�g��𔻒�
    bool isJustJudge_ = false;  //�W���X�g��𔻒�
    int justAvoidState_ = -1;   //�W���X�g����̑J��
    float justAvoidTime_ = 1;   //�W���X�g�������
    float justAvoidTimer_ = 0;
    bool onGroundAvoid_ = true; //�n��̃W���X�g�����
    std::weak_ptr<GameObject> justHitEnemy_;   //�W���X�g������̓G�ۑ�

    bool hitStopEnd_ = false;  //�q�b�g�X�g�b�v���o����
    bool playerDirection_ = false;  //�v���C���[���o

    //�W���X�g��𔽌�
    enum class JUST_AVOID_KEY   //���͂𔻒�
    {
        SQUARE,     //��
        TRIANGLE,   //��

        NULL_KEY,
    };
    JUST_AVOID_KEY justAvoidKey_ = JUST_AVOID_KEY::NULL_KEY;
    JUST_AVOID_KEY justAvoidLeadKey_ = JUST_AVOID_KEY::NULL_KEY;    //��s���͗p
    int triangleState_ = 0;
    std::weak_ptr<GameObject> lockTriangleEnemy_;

    //�W���X�g������o�p
    bool isJustSprite_ = false;
    int justSpriteState_ = -1;
    std::unique_ptr<Sprite> justSprite_;

private:
    std::weak_ptr<PlayerCom> player_;
};