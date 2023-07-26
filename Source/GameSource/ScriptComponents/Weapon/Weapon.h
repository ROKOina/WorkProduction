#pragma once

#include "Components\System\Component.h"
#include "SystemStruct\QuaternionStruct.h"

class WeaponCom : public Component
{
    //コンポーネントオーバーライド
public:
    WeaponCom() {}
    ~WeaponCom() {}

    // 名前取得
    const char* GetName() const override { return "Weapon"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

    //WeaponComクラス
public:
    //オブジェクトをセット
    void SetObject(std::shared_ptr<GameObject> obj) { parentObject_ = obj; }
    //名前をセット
    void SetNodeName(const char* name) { nodeName_ = name; }


    //アニメイベント名から当たり判定を付ける
    void SetCollFromEvent(std::string name);

private:
    //武器の親になるオブジェクト
    std::shared_ptr<GameObject> parentObject_;
    //武器の子にさせたいノードの名前
    std::string nodeName_;
};