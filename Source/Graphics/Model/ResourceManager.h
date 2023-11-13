#pragma once

#include <memory>
#include <string>
#include <map>
#include "Graphics\Model\FbxModelResource.h"

#include "Components/ParticleSystemCom.h"

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
    using ModelMap = std::map<std::string, std::shared_ptr<FbxModelResource>>;

    ModelMap models_;
    std::mutex mutex_;
};

//�p�[�e�B�N�����\�[�X�}�l�W���[
class ResourceManagerParticle
{
private:
    ResourceManagerParticle(){}
    ~ResourceManagerParticle(){}

public:
    //�B��̃C���X�^���X�擾
    static ResourceManagerParticle& Instance()
    {
        static ResourceManagerParticle instance;
        return instance;
    }

    //�p�[�e�B�N�����\�[�X�ǂݍ���
    std::shared_ptr<ParticleSystemCom::SaveParticleData> LoadParticleResource(const char* filename);

    ////�p�[�e�B�N�����\�[�X�o�^
    //void RegisterParticle(const char* filename, std::shared_ptr<ParticleSystemCom::SaveParticleData> resource);

    ////�t�@�C�����œo�^�m�F
    //bool JudgeParticleFilename(const char* filename);

private:
    using ParticleMap = std::map<std::string, std::shared_ptr<ParticleSystemCom::SaveParticleData>>;

    ParticleMap particleMap_;
    std::mutex mutex_;
};