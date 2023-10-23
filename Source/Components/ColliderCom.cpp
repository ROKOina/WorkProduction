#include "ColliderCom.h"
#include "Graphics\Graphics.h"
#include "Components\TransformCom.h"
#include <imgui.h>

//�����蔻��
#pragma region Collider

//����O�̃N���A
void Collider::ColliderStartClear()
{
    hitObj_.clear();
}

//������w�肵�Ĕ���
void Collider::ColliderVSOther(std::shared_ptr<Collider> otherSide)
{
    //�L����
    if (!isEnabled_)return;
    if (!otherSide->isEnabled_)return;

    //�^�O�Ŕ��肷�邩���߂�
    if ((myTag_ == NONE_COL) || (otherSide->myTag_ == NONE_COL))return;
    if ((judgeTag_ != otherSide->myTag_) && (myTag_ != otherSide->judgeTag_))return;

    //�`�󖈂ɓ����蔻���ς���
    int myType = colliderType_;
    int otherType = otherSide->colliderType_;

    bool isJudge = false;

    //��
    if (myType == COLLIDER_TYPE::SphereCollider) {
        if (otherType == COLLIDER_TYPE::SphereCollider)//vs��
            isJudge = SphereVsSphere(otherSide);
        else if (otherType == COLLIDER_TYPE::BoxCollider)//vs�{�b�N�X
            isJudge = SphereVsBox(otherSide);
        else if (otherType == COLLIDER_TYPE::CapsuleCollider)//vs�J�v�Z��
            isJudge = SphereVsCapsule(otherSide);
    }
    //�{�b�N�X
    else if(myType == COLLIDER_TYPE::BoxCollider)
    {
        if (otherType == COLLIDER_TYPE::BoxCollider)//vs�{�b�N�X
            isJudge = BoxVsBox(otherSide);
        else if (otherType == COLLIDER_TYPE::SphereCollider)//vs��
            isJudge = SphereVsBox(otherSide);        
        else if (otherType == COLLIDER_TYPE::CapsuleCollider)//vs�J�v�Z��
            isJudge = BoxVsCapsule(otherSide);

    }
    //�J�v�Z��
    else if (myType == COLLIDER_TYPE::CapsuleCollider)
    {
        if (otherType == COLLIDER_TYPE::CapsuleCollider)//vs�J�v�Z��
            isJudge = CapsuleVsCapsule(otherSide);
        else if (otherType == COLLIDER_TYPE::SphereCollider)//vs��
            isJudge = SphereVsCapsule(otherSide);
        else if (otherType == COLLIDER_TYPE::BoxCollider)//vs�{�b�N�X
            isJudge = BoxVsCapsule(otherSide);
    }


    //�����������̓Q�[���I�u�W�F�N�g�ŕۑ�
    if (isJudge)
    {
        if (judgeTag_ == otherSide->myTag_) {
            HitObj h;
            h.gameObject = otherSide->GetGameObject();
            hitObj_.emplace_back(h);
        }

        if (otherSide->judgeTag_ == myTag_) {
            HitObj h;
            h.gameObject = GetGameObject();
            otherSide->hitObj_.emplace_back(h);
        }
    }

}

//��v��
bool Collider::SphereVsSphere(std::shared_ptr<Collider> otherSide)
{
    std::shared_ptr<SphereColliderCom> mySphere = std::static_pointer_cast<SphereColliderCom>(shared_from_this());
    std::shared_ptr<SphereColliderCom> otherSphere = std::static_pointer_cast<SphereColliderCom>(otherSide);

    //�K�v�ȃp�����[�^�[�擾
    DirectX::XMFLOAT3 myPos = mySphere->GetGameObject()->transform_->GetWorldPosition();
    myPos = { myPos.x + offsetPos_.x,myPos.y + offsetPos_.y,myPos.z + offsetPos_.z };
    DirectX::XMFLOAT3 otherPos = otherSphere->GetGameObject()->transform_->GetWorldPosition();
    otherPos = { otherPos.x + otherSphere->offsetPos_.x,otherPos.y + otherSphere->offsetPos_.y,otherPos.z + otherSphere->offsetPos_.z };
    DirectX::XMVECTOR MyPos = DirectX::XMLoadFloat3(&myPos);
    DirectX::XMVECTOR OtherPos = DirectX::XMLoadFloat3(&otherPos);
    float myRadius = mySphere->GetRadius();
    float otherRadius = otherSphere->GetRadius();

    //�������߂�
    float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(MyPos, OtherPos)));
    float radius = myRadius + otherRadius;

    //����
    if (dist < radius)
    {
        //�����Ԃ����肪����Ȃ牟���Ԃ�
        if (mySphere->GetPushBack() && otherSphere->GetPushBack())
        {
            assert(mySphere->GetPushBackObj().lock());
            assert(otherSphere->GetPushBackObj().lock());

            //���W�擾
            std::shared_ptr<TransformCom> myTransform = mySphere->GetPushBackObj().lock()->transform_;
            std::shared_ptr<TransformCom> otherTransform = otherSphere->GetPushBackObj().lock()->transform_;

            //�d���擾
            float myWeight = mySphere->GetWeight();
            float otherWeight = otherSphere->GetWeight();

            //�����Ԃ�����
            float inSphere = radius - dist; //�߂荞�ݗ�
            //�䗦�Z�o
            float allRatio = myWeight + otherWeight;
            float myRatio = otherWeight / allRatio;
            float otherRatio = myWeight / allRatio;

            //�����o��
            DirectX::XMVECTOR OtherFromMyNormVec = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(MyPos, OtherPos));
            DirectX::XMFLOAT3 myPlus;
            DirectX::XMStoreFloat3(&myPlus, DirectX::XMVectorScale(OtherFromMyNormVec, inSphere * myRatio));
            DirectX::XMFLOAT3 otherPlus;
            DirectX::XMStoreFloat3(&otherPlus, DirectX::XMVectorScale(OtherFromMyNormVec, -inSphere * otherRatio));

            DirectX::XMFLOAT3 myPos = myTransform->GetWorldPosition();
            myPos = { myPos.x + myPlus.x,myPos.y + myPlus.y,myPos.z + myPlus.z };
            myTransform->SetWorldPosition(myPos);
            DirectX::XMFLOAT3 otherPos = otherTransform->GetWorldPosition();
            otherPos = { otherPos.x + otherPlus.x,otherPos.y + otherPlus.y,otherPos.z + otherPlus.z };
            otherTransform->SetWorldPosition(otherPos);
        }

        return true;
    }

    return false;
}

//��v��
bool Collider::BoxVsBox(std::shared_ptr<Collider> otherSide)
{
    std::shared_ptr<BoxColliderCom> myBox = std::static_pointer_cast<BoxColliderCom>(shared_from_this());
    std::shared_ptr<BoxColliderCom> otherBox = std::static_pointer_cast<BoxColliderCom>(otherSide);

    //�K�v�ȃp�����[�^�[�擾
    DirectX::XMFLOAT3 myPos = myBox->GetGameObject()->transform_->GetWorldPosition();
    myPos = { myPos.x + offsetPos_.x,myPos.y + offsetPos_.y,myPos.z + offsetPos_.z };
    DirectX::XMFLOAT3 otherPos = otherBox->GetGameObject()->transform_->GetWorldPosition();
    otherPos = { otherPos.x + otherBox->offsetPos_.x,otherPos.y + otherBox->offsetPos_.y,otherPos.z + otherBox->offsetPos_.z };
    DirectX::XMFLOAT3 mySize = myBox->GetSize();
    DirectX::XMFLOAT3 otherSize = otherBox->GetSize();

    //Y��
    float distY = myPos.y - otherPos.y;
    if (distY * distY > (mySize.y + otherSize.y) * (mySize.y + otherSize.y))
        return false;
    
    //X��
    float distX = myPos.x - otherPos.x;
    if (distX * distX > (mySize.x + otherSize.x) * (mySize.x + otherSize.x))
        return false;

    //Z��
    float distZ = myPos.z - otherPos.z;
    if (distZ * distZ > (mySize.z + otherSize.z) * (mySize.z + otherSize.z))
        return false;

    return true;
}

//�J�v�Z��v�J�v�Z��
bool Collider::CapsuleVsCapsule(std::shared_ptr<Collider> otherSide)
{
    std::shared_ptr<CapsuleColliderCom> myCapsuleCom = std::static_pointer_cast<CapsuleColliderCom>(shared_from_this());
    std::shared_ptr<CapsuleColliderCom> otherCapsuleCom = std::static_pointer_cast<CapsuleColliderCom>(otherSide);

    //�K�v�ȃp�����[�^�[�擾
    CapsuleColliderCom::Capsule myCapsule = myCapsuleCom->GetCupsule();
    CapsuleColliderCom::Capsule otherCapsule = otherCapsuleCom->GetCupsule();
    DirectX::XMFLOAT3 myCenterPos = GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 myP0 = { myCenterPos.x + myCapsule.p0.x,myCenterPos.y + myCapsule.p0.y,myCenterPos.z + myCapsule.p0.z };
    DirectX::XMFLOAT3 myP1 = { myCenterPos.x + myCapsule.p1.x,myCenterPos.y + myCapsule.p1.y,myCenterPos.z + myCapsule.p1.z };
    DirectX::XMFLOAT3 otherCenterPos = otherCapsuleCom->GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 otherP0 = { otherCenterPos.x + otherCapsule.p0.x,otherCenterPos.y + otherCapsule.p0.y,otherCenterPos.z + otherCapsule.p0.z };
    DirectX::XMFLOAT3 otherP1 = { otherCenterPos.x + otherCapsule.p1.x,otherCenterPos.y + otherCapsule.p1.y,otherCenterPos.z + otherCapsule.p1.z };

    DirectX::XMVECTOR dMy = { myP1.x - myP0.x,myP1.y - myP0.y,myP1.z - myP0.z };
    DirectX::XMVECTOR dOther = { otherP1.x - otherP0.x,otherP1.y - otherP0.y,otherP1.z - otherP0.z };
    DirectX::XMVECTOR r = { myP0.x - otherP0.x, myP0.y - otherP0.y, myP0.z - otherP0.z };

    //����J�n
    float a = DirectX::XMVectorGetX(DirectX::XMVector3Dot(dMy, dMy));
    float e = DirectX::XMVectorGetX(DirectX::XMVector3Dot(dOther, dOther));
    float f = DirectX::XMVectorGetX(DirectX::XMVector3Dot(dOther, r));

    float t0 = 0.0f, t1 = 0.0f;

    auto Clamp = [](float val, float min, float max)
    {
        if (val < min) return min;
        if (val > max) return max;
        return val;
    };

    if (a <= FLT_EPSILON && e <= FLT_EPSILON)	// ���������_�ɏk�ނ��Ă���ꍇ
    {
        t0 = t1 = 0.0f;
    }
    else if (a <= FLT_EPSILON)					// �Е��iMy�j���_�ɏk�ނ��Ă���ꍇ
    {
        t0 = 0.0f;
        t1 = Clamp(f / e, 0.0f, 1.0f);
    }
    else
    {
        float c = DirectX::XMVectorGetX(DirectX::XMVector3Dot(dMy, r));
        if (e <= FLT_EPSILON)					// �Е��iOther�j���_�ɏk�ނ��Ă���ꍇ
        {
            t1 = 0.0f;
            t0 = Clamp(-c / a, 0.0f, 1.0f);
        }
        else									// ����������
        {
            float b = DirectX::XMVectorGetX(DirectX::XMVector3Dot(dMy, dOther));
            float denom = a * e - b * b;

            if (denom != 0.0f)					// ���s�m�F�i���s���� t0 = 0.0f�i�����̎n�[�j�����̏����l�Ƃ��Čv�Z�������߂�j
            {
                t0 = Clamp((b * f - c * e) / denom, 0.0f, 1.0f);
            }

            t1 = b * t0 + f;

            if (t1 < 0.0f)						// t1���n�[���O���ɂ���ꍇ
            {
                t1 = 0.0f;
                t0 = Clamp(-c / a, 0.0f, 1.0f);
            }
            else if (t1 > e)					// t1���I�[���O���ɂ���ꍇ
            {
                t1 = 1.0f;
                t0 = Clamp((b - c) / a, 0.0f, 1.0f);
            }
            else								// t1��������ɂ���ꍇ
            {
                t1 /= e;
            }
        }
    }

    // �ŋߓ_�Z�o
    DirectX::XMFLOAT3 p0;
    DirectX::XMFLOAT3 p1;
    DirectX::XMStoreFloat3(&p0, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&myP0), DirectX::XMVectorScale(dMy, t0)));
    DirectX::XMStoreFloat3(&p1, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&otherP0), DirectX::XMVectorScale(dOther, t1)));

    // ��������
    DirectX::XMVECTOR q = { p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };

    return DirectX::XMVectorGetX(DirectX::XMVector3Dot(q, q)) < (myCapsule.radius + otherCapsule.radius) * (myCapsule.radius + otherCapsule.radius);
}

//��v��
bool Collider::SphereVsBox(std::shared_ptr<Collider> otherSide)
{
    //�`��𔻒�
    std::shared_ptr<SphereColliderCom> sphere;
    std::shared_ptr<BoxColliderCom> box;

    if (colliderType_ == COLLIDER_TYPE::SphereCollider)
    {
        sphere = std::static_pointer_cast<SphereColliderCom>(shared_from_this());
        box = std::static_pointer_cast<BoxColliderCom>(otherSide);
    }
    else
    {
        sphere = std::static_pointer_cast<SphereColliderCom>(otherSide);
        box = std::static_pointer_cast<BoxColliderCom>(shared_from_this());
    }

    //�{�b�N�X�Ɠ_�̍ŒZ�������擾
    auto PointToBoxLengthSq = [](float boxSize, float point)
    {
        float SqLen = 0;   // �����ׂ̂���̒l���i�[
        if (point < -boxSize)
            SqLen += (point + boxSize) * (point + boxSize);
        if (point > boxSize)
            SqLen += (point - boxSize) * (point - boxSize);
        return SqLen;
    };

    //���W�擾
    DirectX::XMFLOAT3 spherePos = sphere->GetGameObject()->transform_->GetWorldPosition();
    spherePos = { spherePos.x + sphere->offsetPos_.x,spherePos.y + sphere->offsetPos_.y,spherePos.z + sphere->offsetPos_.z };
    DirectX::XMFLOAT3 boxPos = box->GetGameObject()->transform_->GetWorldPosition();
    boxPos = { boxPos.x + box->offsetPos_.x,boxPos.y + box->offsetPos_.y,boxPos.z + box->offsetPos_.z };
    //�{�b�N�X����Ƃ������W�ɂ���
    DirectX::XMFLOAT3 spherePosFromBox = { spherePos.x - boxPos.x,spherePos.y - boxPos.y,spherePos.z - boxPos.z };

    //���̒��S�_�ƃ{�b�N�X�̍ŒZ����
    float nearLengthSq = 0;
    nearLengthSq += PointToBoxLengthSq(box->GetSize().x, spherePosFromBox.x);
    nearLengthSq += PointToBoxLengthSq(box->GetSize().y, spherePosFromBox.y);
    nearLengthSq += PointToBoxLengthSq(box->GetSize().z, spherePosFromBox.z);

    //�ŒZ�_�����̔��a�ȉ��Ȃ瓖�����Ă���
    if (nearLengthSq < sphere->GetRadius() * sphere->GetRadius())
        return true;

    return false;
}

//��v�J�v�Z��
bool Collider::SphereVsCapsule(std::shared_ptr<Collider> otherSide)
{
    //�`��𔻒�
    std::shared_ptr<SphereColliderCom> sphere;
    std::shared_ptr<CapsuleColliderCom> capsule;

    if (colliderType_ == COLLIDER_TYPE::SphereCollider)
    {
        sphere = std::static_pointer_cast<SphereColliderCom>(shared_from_this());
        capsule = std::static_pointer_cast<CapsuleColliderCom>(otherSide);
    }
    else
    {
        sphere = std::static_pointer_cast<SphereColliderCom>(otherSide);
        capsule = std::static_pointer_cast<CapsuleColliderCom>(shared_from_this());
    }

    //�K�v�ȃp�����[�^�[�擾
    //�J�v�Z��
    CapsuleColliderCom::Capsule Capsule = capsule->GetCupsule();
    DirectX::XMFLOAT3 centerPos = capsule->GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 cP0 = { centerPos.x + Capsule.p0.x,centerPos.y + Capsule.p0.y,centerPos.z + Capsule.p0.z };
    DirectX::XMFLOAT3 cP1 = { centerPos.x + Capsule.p1.x,centerPos.y + Capsule.p1.y,centerPos.z + Capsule.p1.z };

    DirectX::XMVECTOR DCapsule = { cP1.x - cP0.x,cP1.y - cP0.y,cP1.z - cP0.z };

    //��
    DirectX::XMFLOAT3 sPos = sphere->GetGameObject()->transform_->GetWorldPosition();
    sPos = { sPos.x + sphere->offsetPos_.x,sPos.y + sphere->offsetPos_.y,sPos.z + sphere->offsetPos_.z };
    float sRadius = sphere->GetRadius();

    //����J�n
    float l = sqrtf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(DCapsule, DCapsule)));
    DCapsule = DirectX::XMVector3Normalize(DCapsule);	// ���K��

    FLOAT t = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DCapsule, { sPos.x - cP0.x, sPos.y - cP0.y, sPos.z - cP0.z }));	// �ˉe���̎Z�o
    DirectX::XMVECTOR Q = {};	// �ŋߓ_
    if (t < 0)
        Q = DirectX::XMLoadFloat3(&cP0);
    else if (t > l)
        Q = DirectX::XMLoadFloat3(&cP1);
    else
        Q = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&cP0), DirectX::XMVectorScale(DCapsule, t));

    // ��������
    DirectX::XMVECTOR Len = DirectX::XMVectorSubtract(Q, DirectX::XMLoadFloat3(&sPos));

    return DirectX::XMVectorGetX(DirectX::XMVector3Dot(Len, Len)) < (sRadius + Capsule.radius) * (sRadius + Capsule.radius);	// ���Q�擯�m�ō�����r
}

//��v�J�v�Z��
bool Collider::BoxVsCapsule(std::shared_ptr<Collider> otherSide)
{
    //�`��𔻒�
    std::shared_ptr<BoxColliderCom> box;
    std::shared_ptr<CapsuleColliderCom> capsule;

    if (colliderType_ == COLLIDER_TYPE::BoxCollider)
    {
        box = std::static_pointer_cast<BoxColliderCom>(shared_from_this());
        capsule = std::static_pointer_cast<CapsuleColliderCom>(otherSide);
    }
    else
    {
        box = std::static_pointer_cast<BoxColliderCom>(otherSide);
        capsule = std::static_pointer_cast<CapsuleColliderCom>(shared_from_this());
    }

    //�K�v�ȃp�����[�^�[�擾
    //�J�v�Z��
    CapsuleColliderCom::Capsule Capsule = capsule->GetCupsule();
    DirectX::XMFLOAT3 centerPos = capsule->GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 cP0 = { centerPos.x + Capsule.p0.x,centerPos.y + Capsule.p0.y,centerPos.z + Capsule.p0.z };
    DirectX::XMFLOAT3 cP1 = { centerPos.x + Capsule.p1.x,centerPos.y + Capsule.p1.y,centerPos.z + Capsule.p1.z };

    DirectX::XMVECTOR DCapsule = { cP1.x - cP0.x,cP1.y - cP0.y,cP1.z - cP0.z };

    //�{�b�N�X
    DirectX::XMFLOAT3 bPos = box->GetGameObject()->transform_->GetWorldPosition();
    bPos = { bPos.x + box->offsetPos_.x,bPos.y + box->offsetPos_.y,bPos.z + box->offsetPos_.z };
    DirectX::XMFLOAT3 bSize = box->GetSize();

    //����J�n
    float l = sqrtf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(DCapsule, DCapsule)));
    DCapsule = DirectX::XMVector3Normalize(DCapsule);	// ���K��

    FLOAT t = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DCapsule, { bPos.x - cP0.x, bPos.y - cP0.y, bPos.z - cP0.z }));	// �ˉe���̎Z�o
    DirectX::XMVECTOR Q = {};	// �ŋߓ_
    if (t < 0)
        Q = DirectX::XMLoadFloat3(&cP0);
    else if (t > l)
        Q = DirectX::XMLoadFloat3(&cP1);
    else
        Q = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&cP0), DirectX::XMVectorScale(DCapsule, t));

    // ��������
    DirectX::XMVECTOR Len = DirectX::XMVectorSubtract(Q, DirectX::XMLoadFloat3(&bPos));
    DirectX::XMFLOAT3 len;
    DirectX::XMStoreFloat3(&len, Len);

    //�{�b�N�X�Ɠ_�̍ŒZ�������擾
    auto PointToBoxLengthSq = [](float boxSize, float point)
    {
        float SqLen = 0;   // �����ׂ̂���̒l���i�[
        if (point < -boxSize)
            SqLen += (point + boxSize) * (point + boxSize);
        if (point > boxSize)
            SqLen += (point - boxSize) * (point - boxSize);
        return SqLen;
    };

    //���̒��S�_�ƃ{�b�N�X�̍ŒZ����
    float nearLengthSq = 0;
    nearLengthSq += PointToBoxLengthSq(bSize.x, len.x);
    nearLengthSq += PointToBoxLengthSq(bSize.y, len.y);
    nearLengthSq += PointToBoxLengthSq(bSize.z, len.z);

    //�ŒZ�_�����̔��a�ȉ��Ȃ瓖�����Ă���
    if (nearLengthSq < Capsule.radius * Capsule.radius)
        return true;

    return false;
}

#pragma endregion


/// ������`��֐� ///

//��
#pragma region SphereCollider

// GUI�`��
void SphereColliderCom::OnGUI()
{
    ImGui::DragFloat("radius", &radius_,0.1f);
    ImGui::DragFloat3("offsetPos", &offsetPos_.x,0.1f);
    if (isPushBack_)
    {
        ImGui::DragFloat("weight", &weight_, 0.1f, 0, 10);
    }
}

// debug�`��
void SphereColliderCom::DebugRender()
{
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    pos = { pos.x + offsetPos_.x,pos.y + offsetPos_.y,pos.z + offsetPos_.z };
    Graphics::Instance().GetDebugRenderer()->DrawSphere(
       pos, radius_, { 1,0,0,1 });
}

#pragma endregion

//�l�p
#pragma region BoxCollider

// GUI�`��
void BoxColliderCom::OnGUI()
{
    ImGui::DragFloat3("size", &size_.x, 0.1f);
    ImGui::DragFloat3("offsetPos", &offsetPos_.x, 0.1f);
}

// debug�`��
void BoxColliderCom::DebugRender()
{
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    pos = { pos.x + offsetPos_.x,pos.y + offsetPos_.y,pos.z + offsetPos_.z };
    Graphics::Instance().GetDebugRenderer()->DrawBox(pos, size_, { 0,1,0,1 });
}

#pragma endregion

//�J�v�Z��
#pragma region CapsuleCollider

// GUI�`��
void CapsuleColliderCom::OnGUI()
{
    ImGui::DragFloat3("p0", &capsule_.p0.x, 0.1f);
    ImGui::DragFloat3("p1", &capsule_.p1.x, 0.1f);
    ImGui::DragFloat("radius", &capsule_.radius, 0.01f);
}

// debug�`��
void CapsuleColliderCom::DebugRender()
{
    //p0
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 pos0 = { capsule_.p0.x + pos.x,capsule_.p0.y + pos.y,capsule_.p0.z + pos.z };
    Graphics::Instance().GetDebugRenderer()->DrawSphere(pos0, capsule_.radius, { 0,0,1,1 });
    //p1
    DirectX::XMFLOAT3 pos1 = { capsule_.p1.x + pos.x,capsule_.p1.y + pos.y,capsule_.p1.z + pos.z };
    Graphics::Instance().GetDebugRenderer()->DrawSphere(pos1, capsule_.radius, { 0,0,1,1 });
    //�~������
    DirectX::XMVECTOR P0 = DirectX::XMLoadFloat3(&pos0);
    DirectX::XMVECTOR P1 = DirectX::XMLoadFloat3(&pos1);
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(P0, P1)));
    Graphics::Instance().GetDebugRenderer()->DrawCylinder(pos0, pos1, capsule_.radius, length, { 0,0,1,1 });
}

#pragma endregion
