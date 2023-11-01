#include "Particle.hlsli"

SamplerState samplerState : register(s0);
Texture2D textureMap : register(t0);

float4 main(GS_OUT pin) : SV_TARGET
{
    //UV
    pin.texcoord *= particleUV.size;
    pin.texcoord += particleUV.pos;
    
    return textureMap.Sample(samplerState, pin.texcoord) * pin.color;
}