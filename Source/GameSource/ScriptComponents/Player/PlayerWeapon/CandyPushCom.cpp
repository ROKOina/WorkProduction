#include "CandyPushCom.h"

#include "Components/TransformCom.h"
#include "SystemStruct/QuaternionStruct.h"
#include <imgui.h>

void PushWeaponCom::Update(float elapsedTime)
{
    if (isMove_)
    {
        //“®‚©‚·
        DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&GetGameObject()->transform_->GetWorldPosition());
        DirectX::XMVECTOR MovePos = DirectX::XMLoadFloat3(&movePos_);

        DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(MovePos, Pos);
        DirectX::XMVECTOR VecNorm = DirectX::XMVector3Normalize(Vec);

        DirectX::XMFLOAT3 pos;
        DirectX::XMStoreFloat3(&pos, DirectX::XMVectorAdd(Pos, DirectX::XMVectorScale(VecNorm, speed_ * elapsedTime)));
        GetGameObject()->transform_->SetWorldPosition(pos);

        //–Ú•W’n“_‚É‚Â‚¢‚½
        if (DirectX::XMVector3Length(Vec).m128_f32[0] < 1.5f)
        {
            isMove_ = false;
            GetGameObject()->GetChildFind("CandyPush")->transform_->SetEulerRotation(saveEuler_);
        }
    }
}

void PushWeaponCom::OnGUI()
{
    ImGui::DragFloat("speed_", &speed_, 0.1f);
}

void PushWeaponCom::MoveStart(DirectX::XMFLOAT3 movePos, DirectX::XMFLOAT3 startPos)
{
    isMove_ = true;
    movePos_ = movePos;
    startPos_ = startPos;

    DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&GetGameObject()->transform_->GetWorldPosition());
    DirectX::XMVECTOR MovePos = DirectX::XMLoadFloat3(&movePos_);

    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(MovePos, Pos);
    DirectX::XMVECTOR VecNorm = DirectX::XMVector3Normalize(Vec);

    DirectX::XMFLOAT3 vec;
    DirectX::XMStoreFloat3(&vec, VecNorm);
    QuaternionStruct rota;
    rota = rota.LookRotation(vec);

    GetGameObject()->transform_->SetWorldPosition(startPos);
    GetGameObject()->transform_->SetRotation(rota);        


    std::shared_ptr<GameObject> candy = GetGameObject()->GetChildFind("CandyPush");
    saveEuler_= candy->transform_->GetEulerRotation();
    candy->transform_->SetEulerRotation(DirectX::XMFLOAT3(90, 0, 0));
}
