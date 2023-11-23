#include "ShadowMapCasterFunctions.hlsli"

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 world_position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
    float3 shadowTexcoord : TEXCOORD1; //	シャドウマップから情報を取得するためのUV座標＋深度値
};

cbuffer CbScene : register(b0)
{
    float4 viewPosition;
    row_major float4x4 viewProjection;
    float4 ambientLightColor;
    float4 lightDirection;
    float4 lightColor;
};

#define MAX_BONES 128
cbuffer CbMesh : register(b1)
{
    row_major float4x4 boneTransforms[MAX_BONES];
};

cbuffer CbSubset : register(b2)
{
    float4 materialColor;

};

cbuffer CbShadowmap : register(b3)
{
    row_major float4x4 lightViewProjection; // ライトビュープロジェクション行列
    float3 shadowColor; // 影の色
    float shadowBias; // 深度値比較時のオフセット
    
    //影を描画するか
    float shadowFall;
    
    float3 dummyShadow;
};

cbuffer UnityChanToon : register(b4)
{
    //ライトの色を反映するか（ ベースと影両方に使う）
    float _Is_LightColor_Base;
    //影のレベル調整
    float _Tweak_SystemShadowsLevel;
    //影の表示閾値
    float _BaseColor_Step;
    //影のぼかし
    float _BaseShade_Feather;

    float4 _BaseColor;

    float4 _1st_ShadeColor;
    
    //ハイカラーの色
    float4 _Set_HighColorMask;

    //ハイカラーの強さ（ 範囲）
    float _HighColor_Power;
    
    float dummy;
    
    //リムライト強さ
    float _RimLight_Power;
    //リムライトON,OFF
    float _RimLight;
    
    //リムライトカラー
    float4 _RimLightColor;
    
    //エミッシブカラー
    float4 _Emissive_Color;
}

cbuffer CbShape : register(b5)
{
    float shapeLerp0;
    float shapeLerp1;
    float shapeLerp2;
    float shapeLerp3;
    float shapeLerp4;
    float shapeLerp5;
    float shapeLerp6;
    float shapeLerp7;
    float shapeLerp8;
    float shapeLerp9;
    float shapeLerp10;
    float shapeLerp11;
    float shapeLerp12;
    float shapeLerp13;
    float shapeLerp14;
    float shapeLerp15;
}