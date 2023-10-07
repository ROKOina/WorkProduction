#include "SwordTrail.hlsli"

Texture2D swordTex : register(t0);
SamplerState swordSampler : register(s0);

float4 main(VS_OUT input) : SV_TARGET
{
    float4 color = swordTex.Sample(swordSampler, input.uv) * trailColor;
    
    return color;
}