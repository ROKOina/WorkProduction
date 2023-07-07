#pragma once

#include "Components\System\Component.h"
//当たり判定をするコンポーネントまとめてここに書く

//当たり判定のタグ
enum COLLIDER_TAG : uint64_t
{
    NONE    = 1 << 0,
    Player  = 1 << 1,
    Enemy   = 1 << 2,
    Wall    = 1 << 3,
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
    return static_cast<uint64_t>((static_cast<COLLIDER_TAG>(L) & static_cast<COLLIDER_TAG>(R)))
                == static_cast<uint64_t>(R);
}
static bool operator!= (COLLIDER_TAG L, COLLIDER_TAG R)
{
    return static_cast<uint64_t>((static_cast<COLLIDER_TAG>(L) & static_cast<COLLIDER_TAG>(R)))
                != static_cast<uint64_t>(R);
}

//当たり判定の形
enum COLLIDER_TYPE {
    SphereCollider,
    BoxCollider,
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
    std::vector<std::shared_ptr<GameObject>> OnHitGameObject() { return hitGameObject_; }

    const int GetColliderType() const { return colliderType_; }

    void SetWeight(float weight) { weight_ = weight; }
    const float GetWeight() const { return weight_; }

    //自分のタグを決める
    void SetMyTag(COLLIDER_TAG tag) { myTag_ = tag; }

    //当たり判定をするタグを決める
    void SetJudgeTag(COLLIDER_TAG tag) { judgeTag_ = tag; }

    //判定前のクリア
    void ColliderStartClear();

    //相手を指定して判定
    void ColliderVSOther(std::shared_ptr<Collider> otherSide);

private:
    //当たり判定をする
    //球v球(当たっていたらtrue)
    bool SphereVsSphere(std::shared_ptr<Collider> otherSide);

private:
    //当たり判定タグ
    COLLIDER_TAG myTag_= NONE;    //自分のタグ
    COLLIDER_TAG judgeTag_ = NONE; //当たり判定をするタグ

    //今のフレームで当たっているGameObjectを保存
    std::vector<std::shared_ptr<GameObject>> hitGameObject_;

protected:
    //形を保存
    int colliderType_;
    //重さ（判定で重い方を動かないようにする
    float weight_ = 1;
    //オフセット位置
    DirectX::XMFLOAT3 offsetPos_ = { 0,0,0 };
};

class SphereColliderCom : public Collider
{
    //コンポーネントオーバーライド
public:
    SphereColliderCom() { colliderType_ = COLLIDER_TYPE::SphereCollider; }
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

private:
    float radius_ = 0.5f;
};

class BoxColliderCom : public Collider
{
    //コンポーネントオーバーライド
public:
    BoxColliderCom() { colliderType_ = COLLIDER_TYPE::BoxCollider; }
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
    void SetSize(DirectX::XMFLOAT2 size) { size_ = size; }
    DirectX::XMFLOAT2 GetSize() { return size_; }

private:
    DirectX::XMFLOAT2 size_ = { 0.5f,0.5f };
};
