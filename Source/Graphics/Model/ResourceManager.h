#pragma once

#include <memory>
#include <string>
#include <map>
#include "Graphics\Model\ModelResource.h"

//リソースマネジャー
class ResourceManager
{
private:
    ResourceManager(){}
    ~ResourceManager(){}

public:
    //唯一のインスタンス取得
    static ResourceManager& Instance()
    {
        static ResourceManager instance;
        return instance;
    }

    //モデルリソース読み込み
    std::shared_ptr<FbxModelResource> LoadModelResource(const char* filename);

    //モデルリソース登録
    void RegisterModel(const char* filename, std::shared_ptr<FbxModelResource> resource);

    //ファイル名で登録確認
    bool JudgeModelFilename(const char* filename);

private:
    using ModelMap = std::map<std::string, std::weak_ptr<FbxModelResource>>;

    ModelMap models_;
    std::mutex mutex_;
};