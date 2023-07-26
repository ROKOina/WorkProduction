#include "Weapon.h"

#include <imgui.h>

#include "Components\TransformCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"

// 開始処理
void WeaponCom::Start()
{
    //回転
    GetGameObject()->transform_->SetEulerRotation(DirectX::XMFLOAT3(-90, 0, 0));
    //当たり大きさ
    GetGameObject()->GetComponent<CapsuleColliderCom>()->SetRadius(0.19f);
}

// 更新処理
void WeaponCom::Update(float elapsedTime)
{
    assert(parentObject_);
    assert(nodeName_.size() > 0);

    std::shared_ptr<RendererCom> rendererCom = parentObject_->GetComponent<RendererCom>();
    Model::Node* node = rendererCom->GetModel()->FindNode(nodeName_.c_str());

    //親にする
    GetGameObject()->transform_->SetParentTransform(node->worldTransform);



    //カプセル当たり判定設定
    {
        std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
        //剣先から剣元を設定
        capsule->SetPosition1({ 0,0,0 });
        DirectX::XMFLOAT3 up = GetGameObject()->transform_->GetWorldUp();
        capsule->SetPosition2({ up.x,up.y,up.z });
    }
}

// GUI描画
void WeaponCom::OnGUI()
{

}
