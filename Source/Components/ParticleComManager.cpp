#include "ParticleComManager.h"

#include "ParticleSystemCom.h"
#include "ColliderCom.h"
#include "TransformCom.h"
#include <imgui.h>

void ParticleComManager::OnGui()
{
    ImGui::SetNextWindowPos(ImVec2(950, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("ParticleManager", nullptr, ImGuiWindowFlags_None))
    {
        //‰¼’Ç‰Á
        if (ImGui::Button("Add"))
        {
            std::shared_ptr<GameObject> p = GameObjectManager::Instance().Create();
            p->SetName("Particle");
            p->transform_->SetWorldPosition(DirectX::XMFLOAT3(1.0f, 1, -10));

            std::shared_ptr<ParticleSystemCom> c = p->AddComponent<ParticleSystemCom>(10000);
            c->Load("Data/Effect/para/pp.ipff");

            p->AddComponent<SphereColliderCom>();
        }
    }
    ImGui::End();
}