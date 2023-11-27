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
    //武器コンポーネント常時使用フラグ
    void SetIsForeverUse(bool flag) { isForeverUse_ = flag; }
    bool GetIsForeverUse() { return isForeverUse_; }

    //オブジェクトをセット
    void SetObject(std::shared_ptr<GameObject> obj) { parentObject_ = obj; }
    //ノードのオブジェクト設定
    void SetNodeParent(std::shared_ptr<GameObject> obj) { nodeParent_ = obj; }
    //名前をセット
    void SetNodeName(const char* name) { nodeName_ = name; }

    //親オブジェクトのアニメーションを考慮するか
    void SetIsParentAnimUse(bool flag) { isParentAnimUse_ = flag; }
    bool GetIsParentAnimUse() { return isParentAnimUse_; }

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

        //無敵時間（与えた側）
        float weaponInvTime = -1;

        //特殊な攻撃か（ジャンプなど）
        ATTACK_SPECIAL_TYPE specialType = ATTACK_SPECIAL_TYPE::NORMAL;
    };
    //アニメーションとステータスを紐づける
    void SetAttackStatus(int animIndex, int damage, float impactPower, float front = 1, float up = 0, float animSpeed = 1, ATTACK_SPECIAL_TYPE specialAttack = ATTACK_SPECIAL_TYPE::NORMAL);
    //デフォルトステータスを紐づける
    void SetAttackDefaultStatus(int damage, float impactPower, float front = 1, float up = 0, float animSpeed = 1, ATTACK_SPECIAL_TYPE specialAttack = ATTACK_SPECIAL_TYPE::NORMAL, float invincibleTime = -1.0f);

    //ヒット確認
    bool GetOnHit() { return onHit_; }

    //攻撃アニメーション中
    bool GetIsAttackAnim() { return isAttackAnim_; }

    //当たり判定特殊設定（△コンボ２用）
    void SetCircleArc(bool enable) { circleArc_ = enable; }

    //当たり判定調整セット
    void SetColliderUpDown(DirectX::XMFLOAT2 upDown) {
        colliderUpDown_.x = upDown.x;
        colliderUpDown_.y = upDown.y;
    }

private:

    //コリジョンで処理を変える
    void CollisionWeaponAttack();

    //親子関係更新
    void ParentSetting();

    //攻撃処理
    void AttackProcess(std::shared_ptr<GameObject> damageObj, bool useAnim = true);

    //アニメイベント名から当たり判定を付けるか判断("AutoCollision"から始まるイベントを自動で取得)
    bool CollsionFromEventJudge();

    //武器出現演出
    void DirectionStart(float elapsedTime);
    //武器終了演出
    void DirectionEnd(float elapsedTime);

private:
    //武器コンポーネント使用時だけONにするか
    bool isForeverUse_ = false;
    //武器使用時
    bool isWeaponUse_ = false;

    //攻撃のヒット確認
    bool onHit_ = false;

    //親オブジェクトのアニメーションを考慮するか(sphereコライダー限定)(標準は使用しない)
    bool isParentAnimUse_ = false;  

    //アニメーション速度を変更したフラグ
    bool isAnimSetting_ = false;

    //攻撃アニメーション中trueに
    bool isAttackAnim_ = false;
    int attackAnimIndex_ = -1;
    //攻撃終了タイミング用
    bool oldIsAnim_ = false;
    int oldAnimIndex_ = -1;

    //演出フラグ
    bool isDirectionStart_ = true; //武器を出すとき
    bool isDirectionEnd_ = false;   //武器をしまうとき
    int directionState_ = -1;

    //当たり判定特定の設定（△コンボ２用）
    bool circleArc_ = false;

    //武器の当たり判定調整({up,down})
    DirectX::XMFLOAT2 colliderUpDown_ = { 0,0 };

    //武器を使うオブジェクト（アニメーション関係）
    std::weak_ptr<GameObject> parentObject_;
    //武器のノードの親(行列関係)
    std::weak_ptr<GameObject> nodeParent_;
    //武器の子にさせたいノードの名前(ない場合nodeParent_の行列を使う)
    std::string nodeName_;

    //アニメーション使用しないときに使用する
    AttackStatus defaultStatus_;

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