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
    //形状毎に当たり判定を変える
    int myType = colliderType_;
    int otherType = otherSide->colliderType_;

    //球
    if (myType == COLLIDER_TYPE::SphereCollider) {
        if (otherType == COLLIDER_TYPE::SphereCollider)
            SphereVsSphere(otherSide);
        if (otherType == COLLIDER_TYPE::BoxCollider)
            int i = 0;
    }
    //ボックス
    else if(myType == COLLIDER_TYPE::BoxCollider)
    {

    }
}


//球v球(当たっていたらtrue)
bool Collider::SphereVsSphere(std::shared_ptr<Collider> otherSide)
{
    //タグで判定するか決める
    if (myTag_ == NONE || judgeTag_ == NONE)return false;
    if (judgeTag_ != otherSide->myTag_)return false;

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
    {
        ////軽い方を押し返す処理
        //std::shared_ptr<GameObject> heavyGameObject;    //重い
        //std::shared_ptr<GameObject> lightGameObject;    //軽い

        //if (mySphere->GetWeight() > otherSphere->GetWeight())
        //{
        //    heavyGameObject = mySphere->GetGameObject();
        //    lightGameObject = otherSphere->GetGameObject();
        //}
        //else
        //{
        //    heavyGameObject = otherSphere->GetGameObject();
        //    lightGameObject = mySphere->GetGameObject();
        //}

        ////ポジション
        //DirectX::XMVECTOR 

        ////押し返す距離
        //float returnPush = dist - radius + 0.01f;   //オフセットで少し離す

        ////軽い方へのベクトル
        //DirectX::XMVectorSubtract(MyPos, OtherPos)


        //当たった時はゲームオブジェクトで保存
        hitGameObject_.emplace_back(otherSide->GetGameObject());
        otherSide->hitGameObject_.emplace_back(GetGameObject());

        return true;
    }

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

}

// debug描画
void BoxColliderCom::DebugRender()
{
    //DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetPosition();
    //pos = { pos.x + offsetPos_.x,pos.y + offsetPos_.y,pos.z + offsetPos_.z };
    //Graphics::Instance().GetDebugRenderer()->DrawSphere
}

#pragma endregion