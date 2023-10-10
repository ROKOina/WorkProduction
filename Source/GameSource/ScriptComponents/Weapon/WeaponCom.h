#pragma once

#include "Components\System\Component.h"

#include "SystemStruct\QuaternionStruct.h"
#include <map>

//攻撃の種類タグ
enum ATTACK_SPECIAL_TYPE : uint64_t
{
    NORMAL = 1 << 0,

    UNSTOP = 1 << 1,
    JUMP_START = 1 << 2,
    JUMP_NOW = 1 << 3,
};
static ATTACK_SPECIAL_TYPE operator| (ATTACK_SPECIAL_TYPE L, ATTACK_SPECIAL_TYPE R)
{
    return static_cast<ATTACK_SPECIAL_TYPE>(static_cast<uint64_t>(L) | static_cast<uint64_t>(R));
}
static ATTACK_SPECIAL_TYPE operator& (ATTACK_SPECIAL_TYPE L, ATTACK_SPECIAL_TYPE R)
{
    return static_cast<ATTACK_SPECIAL_TYPE>(static_cast<uint64_t>(L) & static_cast<uint64_t>(R));
}
static bool operator== (ATTACK_SPECIAL_TYPE L, ATTACK_SPECIAL_TYPE R)
{
    if (static_cast<uint64_t>((static_cast<ATTACK_SPECIAL_TYPE>(L) & static_cast<ATTACK_SPECIAL_TYPE>(R))) == 0)
        return false;
    return true;
}
static bool operator!= (ATTACK_SPECIAL_TYPE L, ATTACK_SPECIAL_TYPE R)
{
    if (static_cast<uint64_t>((static_cast<ATTACK_SPECIAL_TYPE>(L) & static_cast<ATTACK_SPECIAL_TYPE>(R))) == 0)
        return true;
    return false;
}


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

        //攻撃時のアニメーションスピード
        float animSpeed = 1;

        //特殊な攻撃か（ジャンプなど）
        ATTACK_SPECIAL_TYPE specialType = ATTACK_SPECIAL_TYPE::NORMAL;
    };
    //アニメーションとステータスを紐づける
    void SetAttackStatus(int animIndex, int damage, float impactPower, float front = 1, float up = 0, float animSpeed = 1, ATTACK_SPECIAL_TYPE specialAttack = ATTACK_SPECIAL_TYPE::NORMAL);

    //ヒット確認
    bool GetOnHit() { return onHit_; }

    //攻撃アニメーション中
    bool GetIsAttackAnim() { return isAttackAnim_; }

    //当たり判定調整セット
    void SetColliderUpDown(DirectX::XMFLOAT2 upDown) {
        colliderUpDown_.x = upDown.x;
        colliderUpDown_.y = upDown.y;
    }

private:
    //アニメイベント名から当たり判定を付けるか判断("AutoCollision"から始まるイベントを自動で取得)
    bool CollsionFromEventJudge();

private:
    //攻撃のヒット確認
    bool onHit_ = false;

    //アニメーション速度を変更したフラグ
    bool isAnimSetting = false;

    //攻撃アニメーション中trueに
    bool isAttackAnim_ = false;
    int attackAnimIndex_ = -1;
    //攻撃終了タイミング用
    bool oldIsAnim_ = false;
    int oldAnimIndex = -1;

    //武器の当たり判定調整({up,down})
    DirectX::XMFLOAT2 colliderUpDown_ = { 0,0 };

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