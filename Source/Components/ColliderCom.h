#pragma once

#include "Components\System\Component.h"
//�����蔻�������R���|�[�l���g�܂Ƃ߂Ă����ɏ���

//�����蔻��̃^�O
enum COLLIDER_TAG : uint64_t
{
    NONE    = 1 << 0,
    Player  = 1 << 1,
    Enemy   = 1 << 2,
    Wall    = 1 << 3,
};
static COLLIDER_TAG operator| (COLLIDER_TAG L, COLLIDER_TAG R)
{
    return static_cast<COLLIDER_TAG>(static_cast<uint64_t>(L) | static_cast<uint64_t>(R));
}
static COLLIDER_TAG operator& (COLLIDER_TAG L, COLLIDER_TAG R)
{
    return static_cast<COLLIDER_TAG>(static_cast<uint64_t>(L) & static_cast<uint64_t>(R));
}
static bool operator== (COLLIDER_TAG L, COLLIDER_TAG R)
{
    return static_cast<uint64_t>((static_cast<COLLIDER_TAG>(L) & static_cast<COLLIDER_TAG>(R)))
                == static_cast<uint64_t>(R);
}
static bool operator!= (COLLIDER_TAG L, COLLIDER_TAG R)
{
    return static_cast<uint64_t>((static_cast<COLLIDER_TAG>(L) & static_cast<COLLIDER_TAG>(R)))
                != static_cast<uint64_t>(R);
}

//�����蔻��̌`
enum COLLIDER_TYPE {
    SphereCollider,
    BoxCollider,
};

//�p�����Ĉ�̔z��ɗ��Ƃ�����
class Collider : public Component, public std::enable_shared_from_this<Collider>
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    // ���O�擾
    virtual const char* GetName() const = 0;

    // �J�n����
    virtual void Start() {}

    // �X�V����
    virtual void Update(float elapsedTime) {}

    // GUI�`��
    virtual void OnGUI() {}

    // debug�`��
    virtual void DebugRender() {}

    //Collider�N���X
public:
    //���̊֐��œ����������̏���������
    std::vector<std::shared_ptr<GameObject>> OnHitGameObject() { return hitGameObject_; }

    const int GetColliderType() const { return colliderType_; }

    void SetWeight(float weight) { weight_ = weight; }
    const float GetWeight() const { return weight_; }

    //�����̃^�O�����߂�
    void SetMyTag(COLLIDER_TAG tag) { myTag_ = tag; }

    //�����蔻�������^�O�����߂�
    void SetJudgeTag(COLLIDER_TAG tag) { judgeTag_ = tag; }

    //����O�̃N���A
    void ColliderStartClear();

    //������w�肵�Ĕ���
    void ColliderVSOther(std::shared_ptr<Collider> otherSide);

private:
    //�����蔻�������
    //��v��(�������Ă�����true)
    bool SphereVsSphere(std::shared_ptr<Collider> otherSide);

private:
    //�����蔻��^�O
    COLLIDER_TAG myTag_= NONE;    //�����̃^�O
    COLLIDER_TAG judgeTag_ = NONE; //�����蔻�������^�O

    //���̃t���[���œ������Ă���GameObject��ۑ�
    std::vector<std::shared_ptr<GameObject>> hitGameObject_;

protected:
    //�`��ۑ�
    int colliderType_;
    //�d���i����ŏd�����𓮂��Ȃ��悤�ɂ���
    float weight_ = 1;
    //�I�t�Z�b�g�ʒu
    DirectX::XMFLOAT3 offsetPos_ = { 0,0,0 };
};

class SphereColliderCom : public Collider
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    SphereColliderCom() { colliderType_ = COLLIDER_TYPE::SphereCollider; }
    ~SphereColliderCom() {}

    // ���O�擾
    const char* GetName() const override { return "SphereCollider"; }

    // �J�n����
    void Start() override {}

    // �X�V����
    void Update(float elapsedTime) override {}

    // GUI�`��
    void OnGUI() override;

    // debug�`��
    void DebugRender() override;

    //SphereCollider�N���X
public:
    void SetRadius(float r) { radius_ = r; }
    float GetRadius() { return radius_; }

private:
    float radius_ = 0.5f;
};

class BoxColliderCom : public Collider
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    BoxColliderCom() { colliderType_ = COLLIDER_TYPE::BoxCollider; }
    ~BoxColliderCom() {}

    // ���O�擾
    const char* GetName() const override { return "BoxCollider"; }

    // �J�n����
    void Start() override {}

    // �X�V����
    void Update(float elapsedTime) override {}

    // GUI�`��
    void OnGUI() override;

    // debug�`��
    void DebugRender() override;

    //BoxCollider�N���X
public:
    void SetSize(DirectX::XMFLOAT2 size) { size_ = size; }
    DirectX::XMFLOAT2 GetSize() { return size_; }

private:
    DirectX::XMFLOAT2 size_ = { 0.5f,0.5f };
};
