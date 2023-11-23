#include "UnityChanToon.hlsli"

StructuredBuffer<float3> shapePos[10] : register(t10);

VertexOutput main(
	float4 position : POSITION,
	float3 normal : NORMAL,
	float3 tangent : TANGENT,
	float2 texcoord : TEXCOORD,
	float4 color : COLOR,
	float4 boneWeights : WEIGHTS,
	uint4 boneIndices : BONES,

    uint id : SV_VertexID
)
{
    float3 pos = float3(0, 0, 0);
    pos += lerp(float3(0, 0, 0), shapePos[0][id], shapeLerp0);
    pos += lerp(float3(0, 0, 0), shapePos[1][id], shapeLerp1);
    pos += lerp(float3(0, 0, 0), shapePos[2][id], shapeLerp2);
    pos += lerp(float3(0, 0, 0), shapePos[3][id], shapeLerp3);
    pos += lerp(float3(0, 0, 0), shapePos[4][id], shapeLerp4);
    pos += lerp(float3(0, 0, 0), shapePos[5][id], shapeLerp5);
    pos += lerp(float3(0, 0, 0), shapePos[6][id], shapeLerp6);
    pos += lerp(float3(0, 0, 0), shapePos[7][id], shapeLerp7);
    pos += lerp(float3(0, 0, 0), shapePos[8][id], shapeLerp8);
    pos += lerp(float3(0, 0, 0), shapePos[9][id], shapeLerp9);
    
    float3 p = { 0, 0, 0 };
    float3 n = { 0, 0, 0 };
    float3 t = { 0, 0, 0 };
    for (int i = 0; i < 4; i++)
    {
        p += (boneWeights[i] * mul(float4(position.xyz + pos.xyz, position.w), boneTransforms[boneIndices[i]])).xyz;
        n += (boneWeights[i] * mul(float4(normal.xyz, 0), boneTransforms[boneIndices[i]])).xyz;
        t += (boneWeights[i] * mul(float4(tangent.xyz, 0), boneTransforms[boneIndices[i]])).xyz;
    }

    VertexOutput o = (VertexOutput) 0;
    o.texcoord = texcoord;
    o.normal = normalize(n);
    o.tangent = normalize(t);
    o.binormal = cross(o.normal, o.tangent);
    o.world_position = p;
    o.position = mul(float4(p, 1.0f), viewProjection);
    o.color = color * materialColor;
    
    if (shadowFall > 0.1f)
    {
        // シャドウマップで使用する情報を算出
        o.shadowTexcoord = CalcShadowTexcoord(p, lightViewProjection);
    }
    
    return o;
}
 