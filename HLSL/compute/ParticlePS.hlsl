#include "Particle.hlsli"

SamplerState samplerState : register(s0);
Texture2D texture_map : register(t0);

float4 main(GS_OUT pin) : SV_TARGET
{
    //UV
    pin.texcoord *= texSize;
    pin.texcoord += texPos;
    
    return texture_map.Sample(samplerState, pin.texcoord) * pin.color;
}