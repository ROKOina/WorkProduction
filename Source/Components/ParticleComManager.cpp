#include "ParticleComManager.h"

#include "ParticleSystemCom.h"
#include "System/GameObject.h"
#include "ColliderCom.h"
#include "TransformCom.h"
#include "RendererCom.h"
#include <imgui.h>

void ParticleComManager::Update(float elapsedTime)
{
    int index = 0;
    for (; index < particles_.size(); )
    {
        if (particles_[index].particle.expired())    //データがないなら削除
        {
            particles_.erase(particles_.begin() + index);
            continue;
        }

        //削除判定
        std::shared_ptr<ParticleSystemCom> par = particles_[index].particle.lock()->GetComponent<ParticleSystemCom>();
        if (par->DeleteFlag())
            GameObjectManager::Instance().Remove(particles_[index].particle.lock());

        //追従処理
        if (particles_[index].posObj.lock())
        {
            if (particles_[index].boneName.empty()) //ボーンない場合
            {
                DirectX::XMFLOAT3 pos = particles_[index].posObj.lock()->transform_->GetWorldPosition();
                pos.x += particles_[index].offsetPos.x;
                pos.y += particles_[index].offsetPos.y;
                pos.z += particles_[index].offsetPos.z;
                particles_[index].particle.lock()->transform_->SetWorldPosition(pos);
            }
            else
            {
                DirectX::XMFLOAT4X4 boneTransform = particles_[index].posObj.lock()->GetComponent<RendererCom>()->GetModel()->FindNode(particles_[index].boneName.c_str())->worldTransform;
                DirectX::XMFLOAT3 pos = { boneTransform._41,boneTransform._42,boneTransform._43 };
                pos.x += particles_[index].offsetPos.x;
                pos.y += particles_[index].offsetPos.y;
                pos.z += particles_[index].offsetPos.z;
                particles_[index].particle.lock()->transform_->SetWorldPosition(pos);
            }
        }

        index++;
    }
}

std::shared_ptr<GameObject> ParticleComManager::SetEffect(EFFECT_ID id, DirectX::XMFLOAT3 pos
    , std::shared_ptr<GameObject> parent, std::shared_ptr<GameObject> posObj, std::string boneName, DirectX::XMFLOAT3 offsetPos)
{
    ParticleDataMove obj;
    if (parent)
    {
        obj.particle = parent->AddChildObject();
        obj.particle.lock()->transform_->SetLocalPosition(pos);
    }
    else
    {
        obj.particle = GameObjectManager::Instance().Create();
        obj.particle.lock()->transform_->SetWorldPosition(pos);
    }

    obj.particle.lock()->SetName("Particle");

    std::shared_ptr<ParticleSystemCom> p = obj.particle.lock()->AddComponent<ParticleSystemCom>(iniParticle_[id].maxParticle, iniParticle_[id].isAutoDeleteRoopFlag);
    p->SetSweetsParticle(iniParticle_[id].setSweets);	//お菓子用

    p->Load(iniParticle_[id].particleName.c_str());

    obj.posObj = posObj;
    if (posObj)
    {
        if (!boneName.empty())  //ボーン指定されていたら
        {
            obj.boneName = boneName;
        }
    }
    obj.offsetPos = offsetPos;

    particles_.emplace_back(obj);

    return obj.particle.lock();
}

void ParticleComManager::OnGui()
{
    ImGui::SetNextWindowPos(ImVec2(950, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("ParticleManager", nullptr, ImGuiWindowFlags_None))
    {
        //仮追加
        if (ImGui::Button("Add"))
        {
            SetEffect(EFFECT_ID::COMBO_3
                , GameObjectManager::Instance().Instance().Find("pico")->transform_->GetWorldPosition());
            //SetEffect(EFFECT_ID::DOOM_SWEETS, {0,0,0}
            //    ,GameObjectManager::Instance().Instance().Find("pico"));
        }
    }
    ImGui::End();
}