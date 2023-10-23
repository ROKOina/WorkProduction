#pragma once

#include <memory>
#include <string>
#include <map>
#include "Graphics\Model\ModelResource.h"

//���\�[�X�}�l�W���[
class ResourceManager
{
private:
    ResourceManager(){}
    ~ResourceManager(){}

public:
    //�B��̃C���X�^���X�擾
    static ResourceManager& Instance()
    {
        static ResourceManager instance;
        return instance;
    }

    //���f�����\�[�X�ǂݍ���
    std::shared_ptr<FbxModelResource> LoadModelResource(const char* filename);

    //���f�����\�[�X�o�^
    void RegisterModel(const char* filename, std::shared_ptr<FbxModelResource> resource);

    //�t�@�C�����œo�^�m�F
    bool JudgeModelFilename(const char* filename);

private:
    using ModelMap = std::map<std::string, std::weak_ptr<FbxModelResource>>;

    ModelMap models_;
    std::mutex mutex_;
};