#include "Particle.hlsli"

SamplerState samplerState : register(s0);
Texture2D textureMap : register(t0);

float4 main(GS_OUT pin) : SV_TARGET
{
    //UV
    pin.texcoord *= particleUV.size;
    pin.texcoord += particleUV.pos;
    
    float4 pixelColor = pin.color;
    //0.5�ȉ��������ɂȂ�̂ŕ␳
    pixelColor.w = 0.49f + 0.51f * pixelColor.w;
    pixelColor.w = clamp(pixelColor.w, 0, 1);
    
    return textureMap.Sample(samplerState, pin.texcoord) * pixelColor;
}