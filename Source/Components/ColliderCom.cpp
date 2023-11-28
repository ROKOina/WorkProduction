#include "ColliderCom.h"
#include "Graphics\Graphics.h"
#include "Components\TransformCom.h"
#include <imgui.h>

//当たり判定
#pragma region Collider

//判定前のクリア
void Collider::ColliderStartClear()
{
    hitObj_.clear();
}

//相手を指定して判定
void Collider::ColliderVSOther(std::shared_ptr<Collider> otherSide)
{
    //有効か
    if (!isEnabled_)return;
    if (!otherSide->isEnabled_)return;

    //タグで判定するか決める
    if ((myTag_ == NONE_COL) || (otherSide->myTag_ == NONE_COL))return;
    if ((judgeTag_ != otherSide->myTag_) && (myTag_ != otherSide->judgeTag_))return;

    //形状毎に当たり判定を変える
    int myType = colliderType_;
    int otherType = otherSide->colliderType_;

    bool isJudge = false;

    //球
    if (myType == COLLIDER_TYPE::SphereCollider) {
        if (otherType == COLLIDER_TYPE::SphereCollider)//vs球
            isJudge = SphereVsSphere(otherSide);
        else if (otherType == COLLIDER_TYPE::BoxCollider)//vsボックス
            isJudge = SphereVsBox(otherSide);
        else if (otherType == COLLIDER_TYPE::CapsuleCollider)//vsカプセル
            isJudge = SphereVsCapsule(otherSide);
    }
    //ボックス
    else if(myType == COLLIDER_TYPE::BoxCollider)
    {
        if (otherType == COLLIDER_TYPE::BoxCollider)//vsボックス
            isJudge = BoxVsBox(otherSide);
        else if (otherType == COLLIDER_TYPE::SphereCollider)//vs球
            isJudge = SphereVsBox(otherSide);        
        else if (otherType == COLLIDER_TYPE::CapsuleCollider)//vsカプセル
            isJudge = BoxVsCapsule(otherSide);

    }
    //カプセル
    else if (myType == COLLIDER_TYPE::CapsuleCollider)
    {
        if (otherType == COLLIDER_TYPE::CapsuleCollider)//vsカプセル
            isJudge = CapsuleVsCapsule(otherSide);
        else if (otherType == COLLIDER_TYPE::SphereCollider)//vs球
            isJudge = SphereVsCapsule(otherSide);
        else if (otherType == COLLIDER_TYPE::BoxCollider)//vsボックス
            isJudge = BoxVsCapsule(otherSide);
    }


    //当たった時はゲームオブジェクトで保存
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

//球v球
bool Collider::SphereVsSphere(std::shared_ptr<Collider> otherSide)
{
    std::shared_ptr<SphereColliderCom> mySphere = std::static_pointer_cast<SphereColliderCom>(shared_from_this());
    std::shared_ptr<SphereColliderCom> otherSphere = std::static_pointer_cast<SphereColliderCom>(otherSide);

    //必要なパラメーター取得
    DirectX::XMFLOAT3 myPos = mySphere->GetGameObject()->transform_->GetWorldPosition();
    myPos = { myPos.x + offsetButtonPos_.x,myPos.y + offsetButtonPos_.y,myPos.z + offsetButtonPos_.z };
    DirectX::XMFLOAT3 otherPos = otherSphere->GetGameObject()->transform_->GetWorldPosition();
    otherPos = { otherPos.x + otherSphere->offsetButtonPos_.x,otherPos.y + otherSphere->offsetButtonPos_.y,otherPos.z + otherSphere->offsetButtonPos_.z };
    DirectX::XMVECTOR MyPos = DirectX::XMLoadFloat3(&myPos);
    DirectX::XMVECTOR OtherPos = DirectX::XMLoadFloat3(&otherPos);
    float myRadius = mySphere->GetRadius();
    float otherRadius = otherSphere->GetRadius();

    //距離求める
    float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(MyPos, OtherPos)));
    float radius = myRadius + otherRadius;

    //判定
    if (dist < radius)
    {
        //押し返し判定があるなら押し返す
        if (mySphere->GetPushBack() && otherSphere->GetPushBack())
        {
            assert(mySphere->GetPushBackObj().lock());
            assert(otherSphere->GetPushBackObj().lock());

            //座標取得
            std::shared_ptr<TransformCom> myTransform = mySphere->GetPushBackObj().lock()->transform_;
            std::shared_ptr<TransformCom> otherTransform = otherSphere->GetPushBackObj().lock()->transform_;

            //重さ取得
            float myWeight = mySphere->GetWeight();
            float otherWeight = otherSphere->GetWeight();

            //押し返し処理
            float inSphere = radius - dist; //めり込み量
            //比率算出
            float allRatio = myWeight + otherWeight;
            float myRatio = otherWeight / allRatio;
            float otherRatio = myWeight / allRatio;

            //押し出し
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

//箱v箱
bool Collider::BoxVsBox(std::shared_ptr<Collider> otherSide)
{
    std::shared_ptr<BoxColliderCom> myBox = std::static_pointer_cast<BoxColliderCom>(shared_from_this());
    std::shared_ptr<BoxColliderCom> otherBox = std::static_pointer_cast<BoxColliderCom>(otherSide);

    //必要なパラメーター取得
    DirectX::XMFLOAT3 myPos = myBox->GetGameObject()->transform_->GetWorldPosition();
    myPos = { myPos.x + offsetButtonPos_.x,myPos.y + offsetButtonPos_.y,myPos.z + offsetButtonPos_.z };
    DirectX::XMFLOAT3 otherPos = otherBox->GetGameObject()->transform_->GetWorldPosition();
    otherPos = { otherPos.x + otherBox->offsetButtonPos_.x,otherPos.y + otherBox->offsetButtonPos_.y,otherPos.z + otherBox->offsetButtonPos_.z };
    DirectX::XMFLOAT3 mySize = myBox->GetSize();
    DirectX::XMFLOAT3 otherSize = otherBox->GetSize();

    //Y軸
    float distY = myPos.y - otherPos.y;
    if (distY * distY > (mySize.y + otherSize.y) * (mySize.y + otherSize.y))
        return false;
    
    //X軸
    float distX = myPos.x - otherPos.x;
    if (distX * distX > (mySize.x + otherSize.x) * (mySize.x + otherSize.x))
        return false;

    //Z軸
    float distZ = myPos.z - otherPos.z;
    if (distZ * distZ > (mySize.z + otherSize.z) * (mySize.z + otherSize.z))
        return false;

    return true;
}

//カプセルvカプセル
bool Collider::CapsuleVsCapsule(std::shared_ptr<Collider> otherSide)
{
    std::shared_ptr<CapsuleColliderCom> myCapsuleCom = std::static_pointer_cast<CapsuleColliderCom>(shared_from_this());
    std::shared_ptr<CapsuleColliderCom> otherCapsuleCom = std::static_pointer_cast<CapsuleColliderCom>(otherSide);

    //必要なパラメーター取得
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

    //判定開始
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

    if (a <= FLT_EPSILON && e <= FLT_EPSILON)	// 両線分が点に縮退している場合
    {
        t0 = t1 = 0.0f;
    }
    else if (a <= FLT_EPSILON)					// 片方（My）が点に縮退している場合
    {
        t0 = 0.0f;
        t1 = Clamp(f / e, 0.0f, 1.0f);
    }
    else
    {
        float c = DirectX::XMVectorGetX(DirectX::XMVector3Dot(dMy, r));
        if (e <= FLT_EPSILON)					// 片方（Other）が点に縮退している場合
        {
            t1 = 0.0f;
            t0 = Clamp(-c / a, 0.0f, 1.0f);
        }
        else									// 両方が線分
        {
            float b = DirectX::XMVectorGetX(DirectX::XMVector3Dot(dMy, dOther));
            float denom = a * e - b * b;

            if (denom != 0.0f)					// 平行確認（平行時は t0 = 0.0f（線分の始端）を仮の初期値として計算をすすめる）
            {
                t0 = Clamp((b * f - c * e) / denom, 0.0f, 1.0f);
            }

            t1 = b * t0 + f;

            if (t1 < 0.0f)						// t1が始端より外側にある場合
            {
                t1 = 0.0f;
                t0 = Clamp(-c / a, 0.0f, 1.0f);
            }
            else if (t1 > e)					// t1が終端より外側にある場合
            {
                t1 = 1.0f;
                t0 = Clamp((b - c) / a, 0.0f, 1.0f);
            }
            else								// t1が線分上にある場合
            {
                t1 /= e;
            }
        }
    }

    // 最近点算出
    DirectX::XMFLOAT3 p0;
    DirectX::XMFLOAT3 p1;
    DirectX::XMStoreFloat3(&p0, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&myP0), DirectX::XMVectorScale(dMy, t0)));
    DirectX::XMStoreFloat3(&p1, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&otherP0), DirectX::XMVectorScale(dOther, t1)));

    // 交差判定
    DirectX::XMVECTOR q = { p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };

    return DirectX::XMVectorGetX(DirectX::XMVector3Dot(q, q)) < (myCapsule.radius + otherCapsule.radius) * (myCapsule.radius + otherCapsule.radius);
}

//球v箱
bool Collider::SphereVsBox(std::shared_ptr<Collider> otherSide)
{
    //形状を判定
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

    //ボックスと点の最短距離を取得
    auto PointToBoxLengthSq = [](float boxSize, float point)
    {
        float SqLen = 0;   // 長さのべき乗の値を格納
        if (point < -boxSize)
            SqLen += (point + boxSize) * (point + boxSize);
        if (point > boxSize)
            SqLen += (point - boxSize) * (point - boxSize);
        return SqLen;
    };

    //座標取得
    DirectX::XMFLOAT3 spherePos = sphere->GetGameObject()->transform_->GetWorldPosition();
    spherePos = { spherePos.x + sphere->offsetButtonPos_.x,spherePos.y + sphere->offsetButtonPos_.y,spherePos.z + sphere->offsetButtonPos_.z };
    DirectX::XMFLOAT3 boxPos = box->GetGameObject()->transform_->GetWorldPosition();
    boxPos = { boxPos.x + box->offsetButtonPos_.x,boxPos.y + box->offsetButtonPos_.y,boxPos.z + box->offsetButtonPos_.z };
    //ボックスを基準とした座標にする
    DirectX::XMFLOAT3 spherePosFromBox = { spherePos.x - boxPos.x,spherePos.y - boxPos.y,spherePos.z - boxPos.z };

    //球の中心点とボックスの最短距離
    float nearLengthSq = 0;
    nearLengthSq += PointToBoxLengthSq(box->GetSize().x, spherePosFromBox.x);
    nearLengthSq += PointToBoxLengthSq(box->GetSize().y, spherePosFromBox.y);
    nearLengthSq += PointToBoxLengthSq(box->GetSize().z, spherePosFromBox.z);

    //最短点が球の半径以下なら当たっている
    if (nearLengthSq < sphere->GetRadius() * sphere->GetRadius())
        return true;

    return false;
}

//球vカプセル
bool Collider::SphereVsCapsule(std::shared_ptr<Collider> otherSide)
{
    //形状を判定
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

    //必要なパラメーター取得
    //カプセル
    CapsuleColliderCom::Capsule Capsule = capsule->GetCupsule();
    DirectX::XMFLOAT3 centerPos = capsule->GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 cP0 = { centerPos.x + Capsule.p0.x,centerPos.y + Capsule.p0.y,centerPos.z + Capsule.p0.z };
    DirectX::XMFLOAT3 cP1 = { centerPos.x + Capsule.p1.x,centerPos.y + Capsule.p1.y,centerPos.z + Capsule.p1.z };

    DirectX::XMVECTOR DCapsule = { cP1.x - cP0.x,cP1.y - cP0.y,cP1.z - cP0.z };

    //球
    DirectX::XMFLOAT3 sPos = sphere->GetGameObject()->transform_->GetWorldPosition();
    sPos = { sPos.x + sphere->offsetButtonPos_.x,sPos.y + sphere->offsetButtonPos_.y,sPos.z + sphere->offsetButtonPos_.z };
    float sRadius = sphere->GetRadius();

    //判定開始
    float l = sqrtf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(DCapsule, DCapsule)));
    DCapsule = DirectX::XMVector3Normalize(DCapsule);	// 正規化

    FLOAT t = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DCapsule, { sPos.x - cP0.x, sPos.y - cP0.y, sPos.z - cP0.z }));	// 射影長の算出
    DirectX::XMVECTOR Q = {};	// 最近点
    if (t < 0)
        Q = DirectX::XMLoadFloat3(&cP0);
    else if (t > l)
        Q = DirectX::XMLoadFloat3(&cP1);
    else
        Q = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&cP0), DirectX::XMVectorScale(DCapsule, t));

    // 交差判定
    DirectX::XMVECTOR Len = DirectX::XMVectorSubtract(Q, DirectX::XMLoadFloat3(&sPos));

    return DirectX::XMVectorGetX(DirectX::XMVector3Dot(Len, Len)) < (sRadius + Capsule.radius) * (sRadius + Capsule.radius);	// ※２乗同士で高速比較
}

//箱vカプセル
bool Collider::BoxVsCapsule(std::shared_ptr<Collider> otherSide)
{
    //形状を判定
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

    //必要なパラメーター取得
    //カプセル
    CapsuleColliderCom::Capsule Capsule = capsule->GetCupsule();
    DirectX::XMFLOAT3 centerPos = capsule->GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 cP0 = { centerPos.x + Capsule.p0.x,centerPos.y + Capsule.p0.y,centerPos.z + Capsule.p0.z };
    DirectX::XMFLOAT3 cP1 = { centerPos.x + Capsule.p1.x,centerPos.y + Capsule.p1.y,centerPos.z + Capsule.p1.z };

    DirectX::XMVECTOR DCapsule = { cP1.x - cP0.x,cP1.y - cP0.y,cP1.z - cP0.z };

    //ボックス
    DirectX::XMFLOAT3 bPos = box->GetGameObject()->transform_->GetWorldPosition();
    bPos = { bPos.x + box->offsetButtonPos_.x,bPos.y + box->offsetButtonPos_.y,bPos.z + box->offsetButtonPos_.z };
    DirectX::XMFLOAT3 bSize = box->GetSize();

    //判定開始
    float l = sqrtf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(DCapsule, DCapsule)));
    DCapsule = DirectX::XMVector3Normalize(DCapsule);	// 正規化

    FLOAT t = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DCapsule, { bPos.x - cP0.x, bPos.y - cP0.y, bPos.z - cP0.z }));	// 射影長の算出
    DirectX::XMVECTOR Q = {};	// 最近点
    if (t < 0)
        Q = DirectX::XMLoadFloat3(&cP0);
    else if (t > l)
        Q = DirectX::XMLoadFloat3(&cP1);
    else
        Q = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&cP0), DirectX::XMVectorScale(DCapsule, t));

    // 交差判定
    DirectX::XMVECTOR Len = DirectX::XMVectorSubtract(Q, DirectX::XMLoadFloat3(&bPos));
    DirectX::XMFLOAT3 len;
    DirectX::XMStoreFloat3(&len, Len);

    //ボックスと点の最短距離を取得
    auto PointToBoxLengthSq = [](float boxSize, float point)
    {
        float SqLen = 0;   // 長さのべき乗の値を格納
        if (point < -boxSize)
            SqLen += (point + boxSize) * (point + boxSize);
        if (point > boxSize)
            SqLen += (point - boxSize) * (point - boxSize);
        return SqLen;
    };

    //球の中心点とボックスの最短距離
    float nearLengthSq = 0;
    nearLengthSq += PointToBoxLengthSq(bSize.x, len.x);
    nearLengthSq += PointToBoxLengthSq(bSize.y, len.y);
    nearLengthSq += PointToBoxLengthSq(bSize.z, len.z);

    //最短点が球の半径以下なら当たっている
    if (nearLengthSq < Capsule.radius * Capsule.radius)
        return true;

    return false;
}

#pragma endregion


/// 当たり形状関数 ///

//球
#pragma region SphereCollider

// GUI描画
void SphereColliderCom::OnGUI()
{
    ImGui::DragFloat("radius", &radius_,0.1f);
    ImGui::DragFloat3("offsetPos", &offsetButtonPos_.x,0.1f);
    if (isPushBack_)
    {
        ImGui::DragFloat("weight", &weight_, 0.1f, 0, 10);
    }
}

// debug描画
void SphereColliderCom::DebugRender()
{
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    pos = { pos.x + offsetButtonPos_.x,pos.y + offsetButtonPos_.y,pos.z + offsetButtonPos_.z };
    Graphics::Instance().GetDebugRenderer()->DrawSphere(
       pos, radius_, { 1,0,0,1 });
}

#pragma endregion

//四角
#pragma region BoxCollider

// GUI描画
void BoxColliderCom::OnGUI()
{
    ImGui::DragFloat3("size", &size_.x, 0.1f);
    ImGui::DragFloat3("offsetPos", &offsetButtonPos_.x, 0.1f);
}

// debug描画
void BoxColliderCom::DebugRender()
{
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    pos = { pos.x + offsetButtonPos_.x,pos.y + offsetButtonPos_.y,pos.z + offsetButtonPos_.z };
    Graphics::Instance().GetDebugRenderer()->DrawBox(pos, size_, { 0,1,0,1 });
}

#pragma endregion

//カプセル
#pragma region CapsuleCollider

// GUI描画
void CapsuleColliderCom::OnGUI()
{
    ImGui::DragFloat3("p0", &capsule_.p0.x, 0.1f);
    ImGui::DragFloat3("p1", &capsule_.p1.x, 0.1f);
    ImGui::DragFloat("radius", &capsule_.radius, 0.01f);
}

// debug描画
void CapsuleColliderCom::DebugRender()
{
    //p0
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 pos0 = { capsule_.p0.x + pos.x,capsule_.p0.y + pos.y,capsule_.p0.z + pos.z };
    Graphics::Instance().GetDebugRenderer()->DrawSphere(pos0, capsule_.radius, { 0,0,1,1 });
    //p1
    DirectX::XMFLOAT3 pos1 = { capsule_.p1.x + pos.x,capsule_.p1.y + pos.y,capsule_.p1.z + pos.z };
    Graphics::Instance().GetDebugRenderer()->DrawSphere(pos1, capsule_.radius, { 0,0,1,1 });
    //円柱部分
    DirectX::XMVECTOR P0 = DirectX::XMLoadFloat3(&pos0);
    DirectX::XMVECTOR P1 = DirectX::XMLoadFloat3(&pos1);
    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(P0, P1)));
    Graphics::Instance().GetDebugRenderer()->DrawCylinder(pos0, pos1, capsule_.radius, length, { 0,0,1,1 });
}

#pragma endregion
