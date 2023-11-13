#include "Graphics\Graphics.h"
#include "ResourceManager.h"
#include <map>


//モデルリソース読み込み
std::shared_ptr<FbxModelResource> ResourceManager::LoadModelResource(const char* filename)
{
    std::shared_ptr<FbxModelResource> model;

    //モデル検索
    for (auto& modelmap : models_){
        if (modelmap.first == filename){
            if (modelmap.second) {
                model = std::make_shared<FbxModelResource>(*modelmap.second.get());
                //model = static_cast<std::shared_ptr<FbxModelResource>>(modelmap.second);
            }
        }
    }

    //モデルがない場合
    if (!model) {
        //新規モデルリソース作成＆読み込み
        std::shared_ptr<FbxModelResource> resource = std::make_shared<FbxModelResource>();
        resource->Load(Graphics::Instance().GetDevice(), filename);

        //マップに登録
        models_[filename] = resource;
        model = static_cast<std::shared_ptr<FbxModelResource>>(models_[filename]);
    }

    return model;
}

void ResourceManager::RegisterModel(const char* filename, std::shared_ptr<FbxModelResource> resource)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (models_.count(filename) == 0)
        models_[filename] = resource;
}

bool ResourceManager::JudgeModelFilename(const char* filename)
{
    if (models_.count(filename) == 0)
        return false;
    else
    {
        if (models_[filename])
            return true;
        else
            return false;
    }
}

std::shared_ptr<ParticleSystemCom::SaveParticleData> ResourceManagerParticle::LoadParticleResource(const char* filename)
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::shared_ptr<ParticleSystemCom::SaveParticleData> particle;

    //モデル検索
    for (auto& particlemap : particleMap_) {
        if (particlemap.first == filename) {
            if (particlemap.second) {
                particle = std::make_shared<ParticleSystemCom::SaveParticleData>(*particlemap.second.get());
            }
        }
    }

    //モデルがない場合
    if (!particle) {
        //新規モデルリソース作成＆読み込み
        ParticleSystemCom* particleCom=new ParticleSystemCom(1000);
        std::shared_ptr<ParticleSystemCom::SaveParticleData> resource = std::make_shared<ParticleSystemCom::SaveParticleData>(particleCom->LoadParticle(filename));
        
        //マップに登録
        particleMap_[filename] = resource;
        particle = static_cast<std::shared_ptr<ParticleSystemCom::SaveParticleData>>(particleMap_[filename]);

        delete particleCom;
    }

    return particle;
}
