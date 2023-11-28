#pragma once

#include "Components\System\Component.h"
//当たり判定をするコンポーネントまとめてここに書く

//当たり判定のタグ
enum COLLIDER_TAG : uint64_t
{
    NONE_COL            = 1 << 0,

    Player          = 1 << 1,
    PlayerAttack    = 1 << 2,
    PlayerAttackAssist      = 1 << 3,
    PlayerPushBack      = 1 << 4,
    JustAvoid       = 1 << 5,

    Enemy           = 1 << 10,
    EnemyAttack     = 1 << 11,
    EnemyPushBack = 1 << 12,

    Wall            = 1 << 30,
};
static COLLIDER_TAG operator| (COLLIDER_TAG L, COLLIDER_TAG R)
{
    return static_cast<COLLIDER_TAG>(static_cast<uint64_t>(L) | static_cast<uint64_t>(R));
}
static COLLIDER_TAG operator& (COLLIDER_TAG L, COLLIDER_TAG R)
{
    return static_cast<COLLIDER_TAG>(static_cast<uint64_t>(L) & static_cast<uint64_t>(R));
}
static bool operator== (COLLIDER_TAG L, COLLIDER_TAG R)
{
    if (static_cast<uint64_t>((static_cast<COLLIDER_TAG>(L) & static_cast<COLLIDER_TAG>(R))) == 0)
        return false;
    return true;
}
static bool operator!= (COLLIDER_TAG L, COLLIDER_TAG R)
{
    if (static_cast<uint64_t>((static_cast<COLLIDER_TAG>(L) & static_cast<COLLIDER_TAG>(R))) == 0)
        return true;
    return false;
}

//当たり判定の形
enum class COLLIDER_TYPE {
    SphereCollider,
    BoxCollider,
    CapsuleCollider,
};
static bool operator== (int L, COLLIDER_TYPE R)
{
    return static_cast<int>(L) == static_cast<int>(R);
       
}

//当たった時用の構造体
struct HitObj {
    std::weak_ptr<GameObject> gameObject;
};

//継承して一つの配列に落とし込む
class Collider : public Component, public std::enable_shared_from_this<Collider>
{
    //コンポーネントオーバーライド
public:
    // 名前取得
    virtual const char* GetName() const = 0;

    // 開始処理
    virtual void Start() {}

    // 更新処理
    virtual void Update(float elapsedTime) {}

    // GUI描画
    virtual void OnGUI() {}

    // debug描画
    virtual void DebugRender() {}

    //Colliderクラス
public:
    //この関数で当たった時の処理を書く
    std::vector<HitObj> OnHitGameObject() { return hitObj_; }

    int GetColliderType() const { return colliderType_; }

    //自分のタグを決める
    void SetMyTag(COLLIDER_TAG tag) { myTag_ = tag; }
    //自分のタグを見る
    uint64_t GetMyTag() const { return myTag_; }

    //当たり判定をするタグを決める
    void SetJudgeTag(COLLIDER_TAG tag) { judgeTag_ = tag; }

    //判定前のクリア
    void ColliderStartClear();

    //相手を指定して判定
    void ColliderVSOther(std::shared_ptr<Collider> otherSide);

    void SetOffsetPosition(DirectX::XMFLOAT3& offsetPos) { offsetButtonPos_ = offsetPos; }
    DirectX::XMFLOAT3 const GetOffsetPosition() { return offsetButtonPos_; }

private:
    //当たり判定をする(当たっていたらtrue)
    //球v球
    bool SphereVsSphere(std::shared_ptr<Collider> otherSide);
    //箱v箱
    bool BoxVsBox(std::shared_ptr<Collider> otherSide);
    //カプセルvカプセル
    bool CapsuleVsCapsule(std::shared_ptr<Collider> otherSide);

    //球v箱
    bool SphereVsBox(std::shared_ptr<Collider> otherSide);
    //球vカプセル
    bool SphereVsCapsule(std::shared_ptr<Collider> otherSide);
    //箱vカプセル
    bool BoxVsCapsule(std::shared_ptr<Collider> otherSide);

private:
    //当たり判定タグ
    COLLIDER_TAG myTag_= COLLIDER_TAG::NONE_COL;    //自分のタグ
    COLLIDER_TAG judgeTag_ = COLLIDER_TAG::NONE_COL; //当たり判定をするタグ

    //今のフレームで当たっているものを保存
    std::vector<HitObj> hitObj_;

protected:
    //形を保存
    int colliderType_;
    //オフセット位置
    DirectX::XMFLOAT3 offsetButtonPos_ = { 0,0,0 };


};

class SphereColliderCom : public Collider
{
    //コンポーネントオーバーライド
public:
    SphereColliderCom() { colliderType_ = static_cast<int>(COLLIDER_TYPE::SphereCollider); }
    ~SphereColliderCom() {}

    // 名前取得
    const char* GetName() const override { return "SphereCollider"; }

    // 開始処理
    void Start() override {}

    // 更新処理
    void Update(float elapsedTime) override {}

    // GUI描画
    void OnGUI() override;

    // debug描画
    void DebugRender() override;

    //SphereColliderクラス
public:
    void SetRadius(float r) { radius_ = r; }
    float GetRadius() { return radius_; }

    void SetPushBack(bool flag) { isPushBack_ = flag; }
    bool GetPushBack() const { return isPushBack_; }

    void SetWeight(float weight) { weight_ = weight; }
    float GetWeight() const { return weight_; }

    void SetPushBackObj(std::shared_ptr<GameObject> obj) { pushBackObj_ = obj; }
    std::weak_ptr<GameObject> GetPushBackObj() const { return pushBackObj_; }

private:
    float radius_ = 0.5f;

    //押し返しするか
    bool isPushBack_ = false;
    //重さ
    float weight_ = 1;
    //押し返しするオブジェクトを指定
    std::weak_ptr<GameObject> pushBackObj_;
};

class BoxColliderCom : public Collider
{
    //コンポーネントオーバーライド
public:
    BoxColliderCom() { colliderType_ = static_cast<int>(COLLIDER_TYPE::BoxCollider); }
    ~BoxColliderCom() {}

    // 名前取得
    const char* GetName() const override { return "BoxCollider"; }

    // 開始処理
    void Start() override {}

    // 更新処理
    void Update(float elapsedTime) override {}

    // GUI描画
    void OnGUI() override;

    // debug描画
    void DebugRender() override;

    //BoxColliderクラス
public:
    void SetSize(DirectX::XMFLOAT3 size) { size_ = size; }
    const DirectX::XMFLOAT3 GetSize() const { return size_; }

private:
    DirectX::XMFLOAT3 size_ = { 0.5f,0.5f,0.5f };
};

class CapsuleColliderCom : public Collider
{
    //コンポーネントオーバーライド
public:
    CapsuleColliderCom() { colliderType_ = static_cast<int>(COLLIDER_TYPE::CapsuleCollider); }
    ~CapsuleColliderCom() {}

    // 名前取得
    const char* GetName() const override { return "CapsuleCollider"; }

    // 開始処理
    void Start() override {}

    // 更新処理
    void Update(float elapsedTime) override {}

    // GUI描画
    void OnGUI() override;

    // debug描画
    void DebugRender() override;

    //CapsuleColliderクラス
public:
    // カプセルの定義
    struct Capsule
    {
        DirectX::XMFLOAT3	p0 = { 0,0,0 };	// 円柱の中心線の始端
        DirectX::XMFLOAT3	p1 = { 0,0,0 };	// 円柱の中心線の終端
        float				radius = 0.5f;	// 半径
    };

    void SetPosition1(DirectX::XMFLOAT3 pos) { capsule_.p0 = pos; }
    void SetPosition2(DirectX::XMFLOAT3 pos) { capsule_.p1 = pos; }
    void SetRadius(float radius) { capsule_.radius = radius; }

    const Capsule& GetCupsule()const { return capsule_; }

private:
    Capsule capsule_;
};
