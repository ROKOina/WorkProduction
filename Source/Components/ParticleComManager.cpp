#include "ParticleComManager.h"

#include "ParticleSystemCom.h"
#include "System/GameObject.h"
#include "ColliderCom.h"
#include "TransformCom.h"
#include <imgui.h>

void ParticleComManager::Update(float elapsedTime)
{
    int index = 0;
    for (; index < particles_.size(); )
    {
        if (particles_[index].expired())    //データがないなら削除
        {
            particles_.erase(particles_.begin() + index);
            continue;
        }

        //削除判定
        std::shared_ptr<ParticleSystemCom> par = particles_[index].lock()->GetComponent<ParticleSystemCom>();
        if (par->DeleteFlag())
            GameObjectManager::Instance().Remove(particles_[index].lock());

        index++;
    }
}

std::shared_ptr<GameObject> ParticleComManager::SetEffect(EFFECT_ID id, DirectX::XMFLOAT3 pos, std::shared_ptr<GameObject> parent)
{
    std::shared_ptr<GameObject> obj;
    if (parent)
    {
        obj = parent->AddChildObject();
        obj->transform_->SetLocalPosition(pos);
    }
    else
    {
        obj = GameObjectManager::Instance().Create();
        obj->transform_->SetWorldPosition(pos);
    }

    obj->SetName("Particle");

    std::shared_ptr<ParticleSystemCom> p = obj->AddComponent<ParticleSystemCom>(iniParticle_[id].maxParticle, iniParticle_[id].isAutoDeleteRoopFlag);
    p->SetSweetsParticle(iniParticle_[id].setSweets);	//お菓子用

    p->Load(iniParticle_[id].particleName.c_str());

    particles_.emplace_back(obj);

    return obj;
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