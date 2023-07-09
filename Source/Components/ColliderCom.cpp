#include "ColliderCom.h"
#include "Graphics\Graphics.h"
#include "Components\TransformCom.h"
#include <imgui.h>

//当たり判定
#pragma region Collider

//判定前のクリア
void Collider::ColliderStartClear()
{
    hitGameObject_.clear();
}

//相手を指定して判定
void Collider::ColliderVSOther(std::shared_ptr<Collider> otherSide)
{
    //タグで判定するか決める
    if ((myTag_ == NONE) || (otherSide->myTag_ == NONE))return;
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
    }
    //ボックス
    else if(myType == COLLIDER_TYPE::BoxCollider)
    {
        if (otherType == COLLIDER_TYPE::BoxCollider)//vsボックス
            isJudge = BoxVsBox(otherSide);
        else if (otherType == COLLIDER_TYPE::SphereCollider)//vs球
            isJudge = SphereVsBox(otherSide);
    }

    //当たった時はゲームオブジェクトで保存
    if (isJudge)
    {
        if (judgeTag_ == otherSide->myTag_)
            hitGameObject_.emplace_back(otherSide->GetGameObject());

        if (otherSide->judgeTag_ == myTag_)
            otherSide->hitGameObject_.emplace_back(GetGameObject());
    }
}


//球v球
bool Collider::SphereVsSphere(std::shared_ptr<Collider> otherSide)
{
    std::shared_ptr<SphereColliderCom> mySphere = std::static_pointer_cast<SphereColliderCom>(shared_from_this());
    std::shared_ptr<SphereColliderCom> otherSphere = std::static_pointer_cast<SphereColliderCom>(otherSide);

    //必要なパラメーター取得
    DirectX::XMFLOAT3 myPos = mySphere->GetGameObject()->transform_->GetPosition();
    myPos = { myPos.x + offsetPos_.x,myPos.y + offsetPos_.y,myPos.z + offsetPos_.z };
    DirectX::XMFLOAT3 otherPos = otherSphere->GetGameObject()->transform_->GetPosition();
    otherPos = { otherPos.x + otherSphere->offsetPos_.x,otherPos.y + otherSphere->offsetPos_.y,otherPos.z + otherSphere->offsetPos_.z };
    DirectX::XMVECTOR MyPos = DirectX::XMLoadFloat3(&myPos);
    DirectX::XMVECTOR OtherPos = DirectX::XMLoadFloat3(&otherPos);
    float myRadius = mySphere->GetRadius();
    float otherRadius = otherSphere->GetRadius();

    //距離求める
    float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(MyPos, OtherPos)));
    float radius = myRadius + otherRadius;

    //判定
    if (dist < radius)
        return true;

    return false;
}

//箱v箱
bool Collider::BoxVsBox(std::shared_ptr<Collider> otherSide)
{
    std::shared_ptr<BoxColliderCom> myBox = std::static_pointer_cast<BoxColliderCom>(shared_from_this());
    std::shared_ptr<BoxColliderCom> otherBox = std::static_pointer_cast<BoxColliderCom>(otherSide);

    //必要なパラメーター取得
    DirectX::XMFLOAT3 myPos = myBox->GetGameObject()->transform_->GetPosition();
    myPos = { myPos.x + offsetPos_.x,myPos.y + offsetPos_.y,myPos.z + offsetPos_.z };
    DirectX::XMFLOAT3 otherPos = otherBox->GetGameObject()->transform_->GetPosition();
    otherPos = { otherPos.x + otherBox->offsetPos_.x,otherPos.y + otherBox->offsetPos_.y,otherPos.z + otherBox->offsetPos_.z };
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
    DirectX::XMFLOAT3 spherePos = sphere->GetGameObject()->transform_->GetPosition();
    DirectX::XMFLOAT3 boxPos = box->GetGameObject()->transform_->GetPosition();
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


#pragma endregion


/// 当たり形状関数 ///

//球
#pragma region SphereCollider

// GUI描画
void SphereColliderCom::OnGUI()
{
    ImGui::DragFloat("radius", &radius_,0.1f);
    ImGui::DragFloat3("offsetPos", &offsetPos_.x,0.1f);
}

// debug描画
void SphereColliderCom::DebugRender()
{
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetPosition();
    pos = { pos.x + offsetPos_.x,pos.y + offsetPos_.y,pos.z + offsetPos_.z };
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
    ImGui::DragFloat3("offsetPos", &offsetPos_.x, 0.1f);
}

// debug描画
void BoxColliderCom::DebugRender()
{
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetPosition();
    pos = { pos.x + offsetPos_.x,pos.y + offsetPos_.y,pos.z + offsetPos_.z };
    Graphics::Instance().GetDebugRenderer()->DrawBox(pos, size_, { 1,0,0,1 });
}

#pragma endregion