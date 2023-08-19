#include "UnityChanToon.hlsli"


VertexOutput main(
	float4 position : POSITION,
	float3 normal : NORMAL,
	float3 tangent : TANGENT,
	float2 texcoord : TEXCOORD,
	float4 color : COLOR,
	float4 boneWeights : WEIGHTS,
	uint4 boneIndices : BONES
)
{
    float3 p = { 0, 0, 0 };
    float3 n = { 0, 0, 0 };
    float3 t = { 0, 0, 0 };
    for (int i = 0; i < 4; i++)
    {
        p += (boneWeights[i] * mul(position, boneTransforms[boneIndices[i]])).xyz;
        n += (boneWeights[i] * mul(float4(normal.xyz, 0), boneTransforms[boneIndices[i]])).xyz;
        t += (boneWeights[i] * mul(float4(tangent.xyz, 0), boneTransforms[boneIndices[i]])).xyz;
    }

    VertexOutput o = (VertexOutput) 0;
    o.texcoord = texcoord;
    o.normal = normalize(n);
    o.tangent = normalize(t);
    o.binormal = cross(o.normal, o.tangent);
    o.world_position = float4(p, 0);
    o.position = mul(float4(p, 1.0f), viewProjection);
    o.color = color * materialColor;

    return o;
}
 