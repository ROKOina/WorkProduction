#pragma once

#include "Components\System\Component.h"

#include <wrl.h>
#include <d3d11.h>

class SwordTrailCom : public Component
{
    //コンポーネントオーバーライド
public:
    SwordTrailCom() {}
    ~SwordTrailCom() {}

    // 名前取得
    const char* GetName() const override { return "SwordTrail"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

    //SwordTrailComクラス
public:
    //描画
    void Render();

    //末端と先端ノード登録(tail：末端　、　head：先端)
    void SetHeadTailNodeName(std::string tail, std::string head)
    {
        nodeName_[0] = tail;
        nodeName_[1] = head;
    }

    //トレイルの位置をリセットする
    void ResetNodePos();

    //トレイルを終わらせる
    void EndTrail();

private:
    //剣の位置を保存する構造体
    struct PosBuffer
    {
        DirectX::XMFLOAT3 head = { 0,0,0 }; //剣の先端の位置
        DirectX::XMFLOAT3 tail = { 0,0,0 }; //剣の末端の位置
    };

    //頂点バッファ
    struct SwordTrailVertex
    {
        DirectX::XMFLOAT3 pos = { 0, 0, 0 };
        DirectX::XMFLOAT2 uv = { 0, 0 };
    };

    //曲線の頂点を作る
    void CreateCurveVertex(std::vector<PosBuffer>& usedPosArray, int split);



private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;   
    Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;

    Microsoft::WRL::ComPtr<ID3D11Buffer> vBuffer_;

    struct TrailBuffer
    {
        DirectX::XMFLOAT4X4	viewProjection;
        DirectX::XMFLOAT4 trailColor;
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> VPBuffer_;

    std::vector<PosBuffer> posArray_;             //剣の位置を保存するバッファ
    std::vector<SwordTrailVertex> vertex_;        //頂点バッファ
    //曲線の滑らかさ
    int split_ = 10;
    //線の長さ
    int index_ = 20;

    //トレイルの先端と末端ノードの名前
    std::string nodeName_[2];   //0:末端　、1:先端

    //トレイルを終了する
    bool endTrail_ = false;
};