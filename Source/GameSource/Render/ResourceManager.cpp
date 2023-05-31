#include "Graphics\Graphics.h"
#include "ResourceManager.h"
#include <map>


//モデルリソース読み込み
std::shared_ptr<ModelResource> ResourceManager::LoadModelResource(const char* filename)
{
    std::shared_ptr<ModelResource> model;

    //モデル検索
    for (auto& modelmap : models){
        if (modelmap.first == filename){
            if (modelmap.second.expired() == false) {
                model = static_cast<std::shared_ptr<ModelResource>>(modelmap.second);
            }
        }
    }

    //モデルがない場合
    if (!model) {
        //新規モデルリソース作成＆読み込み
        std::shared_ptr<ModelResource> resource = std::make_shared<ModelResource>();
        resource->Load(Graphics::Instance().GetDevice(), filename);

        //マップに登録
        models[filename] = resource;
        model = static_cast<std::shared_ptr<ModelResource>>(models[filename]);
    }

    return model;
}