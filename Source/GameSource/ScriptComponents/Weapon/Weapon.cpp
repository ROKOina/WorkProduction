#include "Weapon.h"

#include <imgui.h>

#include "Components\TransformCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"

// �J�n����
void WeaponCom::Start()
{
    //��]
    GetGameObject()->transform_->SetEulerRotation(DirectX::XMFLOAT3(-90, 0, 0));
    //������傫��
    GetGameObject()->GetComponent<CapsuleColliderCom>()->SetRadius(0.19f);
}

// �X�V����
void WeaponCom::Update(float elapsedTime)
{
    assert(parentObject_);
    assert(nodeName_.size() > 0);

    std::shared_ptr<RendererCom> rendererCom = parentObject_->GetComponent<RendererCom>();
    Model::Node* node = rendererCom->GetModel()->FindNode(nodeName_.c_str());

    //�e�ɂ���
    GetGameObject()->transform_->SetParentTransform(node->worldTransform);



    //�J�v�Z�������蔻��ݒ�
    {
        std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
        //���悩�猕����ݒ�
        capsule->SetPosition1({ 0,0,0 });
        DirectX::XMFLOAT3 up = GetGameObject()->transform_->GetWorldUp();
        capsule->SetPosition2({ up.x,up.y,up.z });
    }
}

// GUI�`��
void WeaponCom::OnGUI()
{

}
