#include "SwordTrail.hlsli"

VS_OUT main(
    float3 position : POSITION,
    float2 texcoord : TEXCOORD
    )
{
    VS_OUT result;
    float4 p = float4(position, 1);
    result.pos = mul(p, VP);
    result.uv = texcoord;

    return result;
}