#pragma once

#include "Components\System\Component.h"
//�����蔻�������R���|�[�l���g�܂Ƃ߂Ă����ɏ���

//�����蔻��̃^�O
enum COLLIDER_TAG : uint64_t
{
    NONE            = 1 << 0,

    Player          = 1 << 1,
    PlayerAttack    = 1 << 2,
    JustAvoid       = 1 << 3,

    Enemy           = 1 << 10,
    EnemyAttack     = 1 << 11,

    Wall            = 1 << 50,
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
    if (static_cast<uint64_t>((static_cast<COLLIDER_TAG>(L) & static_cast<COLLIDER_TAG>(R))) == 0)
        return false;
    return true;
}
static bool operator!= (COLLIDER_TAG L, COLLIDER_TAG R)
{
    if (static_cast<uint64_t>((static_cast<COLLIDER_TAG>(L) & static_cast<COLLIDER_TAG>(R))) == 0)
        return true;
    return false;
}

//�����蔻��̌`
enum class COLLIDER_TYPE {
    SphereCollider,
    BoxCollider,
    CapsuleCollider,
};
static bool operator== (int L, COLLIDER_TYPE R)
{
    return static_cast<int>(L) == static_cast<int>(R);
       
}

//�����������p�̍\����
struct HitObj {
    std::shared_ptr<GameObject> gameObject;
    float colliderDist; //�����蔻��̃T�C�Y�œ������ĂȂ�����
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
    std::vector<HitObj> OnHitGameObject() { return hitObj_; }

    const int GetColliderType() const { return colliderType_; }

    void SetWeight(float weight) { weight_ = weight; }
    const float GetWeight() const { return weight_; }

    //�����̃^�O�����߂�
    void SetMyTag(COLLIDER_TAG tag) { myTag_ = tag; }
    //�����̃^�O������
    const uint64_t GetMyTag() const { return myTag_; }

    //�����蔻�������^�O�����߂�
    void SetJudgeTag(COLLIDER_TAG tag) { judgeTag_ = tag; }

    //����O�̃N���A
    void ColliderStartClear();

    //������w�肵�Ĕ���
    void ColliderVSOther(std::shared_ptr<Collider> otherSide);

    void SetOffsetPosition(DirectX::XMFLOAT3& offsetPos) { offsetPos_ = offsetPos; }
    const DirectX::XMFLOAT3 const GetOffsetPosition() { return offsetPos_; }

private:
    //�����蔻�������(�������Ă�����true)
    //��v��
    bool SphereVsSphere(std::shared_ptr<Collider> otherSide);
    //��v��
    bool BoxVsBox(std::shared_ptr<Collider> otherSide);
    //�J�v�Z��v�J�v�Z��
    bool CapsuleVsCapsule(std::shared_ptr<Collider> otherSide);

    //��v��
    bool SphereVsBox(std::shared_ptr<Collider> otherSide);
    //��v�J�v�Z��
    bool SphereVsCapsule(std::shared_ptr<Collider> otherSide);
    //��v�J�v�Z��
    bool BoxVsCapsule(std::shared_ptr<Collider> otherSide);

private:
    //�����蔻��^�O
    COLLIDER_TAG myTag_= NONE;    //�����̃^�O
    COLLIDER_TAG judgeTag_ = NONE; //�����蔻�������^�O

    //���̃t���[���œ������Ă�����̂�ۑ�
    std::vector<HitObj> hitObj_;

    float colliderSizeDist_; //�����蔻��̃T�C�Y�œ������ĂȂ�������ۑ�

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
    SphereColliderCom() { colliderType_ = static_cast<int>(COLLIDER_TYPE::SphereCollider); }
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
    BoxColliderCom() { colliderType_ = static_cast<int>(COLLIDER_TYPE::BoxCollider); }
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
    void SetSize(DirectX::XMFLOAT3 size) { size_ = size; }
    const DirectX::XMFLOAT3 GetSize() const { return size_; }

private:
    DirectX::XMFLOAT3 size_ = { 0.5f,0.5f,0.5f };
};

class CapsuleColliderCom : public Collider
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    CapsuleColliderCom() { colliderType_ = static_cast<int>(COLLIDER_TYPE::CapsuleCollider); }
    ~CapsuleColliderCom() {}

    // ���O�擾
    const char* GetName() const override { return "CapsuleCollider"; }

    // �J�n����
    void Start() override {}

    // �X�V����
    void Update(float elapsedTime) override {}

    // GUI�`��
    void OnGUI() override;

    // debug�`��
    void DebugRender() override;

    //CapsuleCollider�N���X
public:
    // �J�v�Z���̒�`
    struct Capsule
    {
        DirectX::XMFLOAT3	p0 = { 0,0,0 };	// �~���̒��S���̎n�[
        DirectX::XMFLOAT3	p1 = { 0,0,0 };	// �~���̒��S���̏I�[
        float				radius = 0.5f;	// ���a
    };

    void SetPosition1(DirectX::XMFLOAT3 pos) { capsule_.p0 = pos; }
    void SetPosition2(DirectX::XMFLOAT3 pos) { capsule_.p1 = pos; }
    void SetRadius(float radius) { capsule_.radius = radius; }

    const Capsule& GetCupsule()const { return capsule_; }

private:
    Capsule capsule_;
};
