#include "Graphics\Graphics.h"
#include "ResourceManager.h"
#include <map>


//���f�����\�[�X�ǂݍ���
std::shared_ptr<FbxModelResource> ResourceManager::LoadModelResource(const char* filename)
{
    std::shared_ptr<FbxModelResource> model;

    //���f������
    for (auto& modelmap : models_){
        if (modelmap.first == filename){
            if (modelmap.second.expired() == false) {
                model = static_cast<std::shared_ptr<FbxModelResource>>(modelmap.second);
            }
        }
    }

    //���f�����Ȃ��ꍇ
    if (!model) {
        //�V�K���f�����\�[�X�쐬���ǂݍ���
        std::shared_ptr<FbxModelResource> resource = std::make_shared<FbxModelResource>();
        resource->Load(Graphics::Instance().GetDevice(), filename);

        //�}�b�v�ɓo�^
        models_[filename] = resource;
        model = static_cast<std::shared_ptr<FbxModelResource>>(models_[filename]);
    }

    return model;
}