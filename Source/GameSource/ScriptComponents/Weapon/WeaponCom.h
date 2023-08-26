#pragma once

#include "Components\System\Component.h"

#include "SystemStruct\QuaternionStruct.h"
#include <map>

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

    //攻撃のステータス構造体
    struct AttackStatus
    {
        int damage = 0;     //アタックダメージ
        float impactPower = 0;  //吹っ飛ばし力
        //吹っ飛ばし方向
        float front = 1;    //1~0で強さを決める
        float up = 0;       //両方1なら斜めになる
    };
    //アニメーションとステータスを紐づける
    void SetAttackStatus(int animIndex, int damage, float impactPower, float front = 1, float up = 0);

    //ヒット確認
    bool GetOnHit() { return onHit_; }

private:
    //アニメイベント名から当たり判定を付けるか判断("AutoCollision"から始まるイベントを自動で取得)
    bool CollsionFromEventJudge();

private:
    //攻撃のヒット確認
    bool onHit_ = false;

    //武器の親になるオブジェクト
    std::shared_ptr<GameObject> parentObject_;
    //武器の子にさせたいノードの名前
    std::string nodeName_;

    //攻撃時のステータス（int:アニメーションIndex）
    std::map<int, AttackStatus> attackStatus_;
};


/*
    ------  アニメーション当たり判定について ---------
当たり判定をアニメーションに合わせて付ける場合は、
アニメーションイベントの名前の頭に"AutoCollision"を付けて、
付けるフレームを指定する。
nodeはどこでもいい（判定の大きさはこっちで定義しているから）
*/