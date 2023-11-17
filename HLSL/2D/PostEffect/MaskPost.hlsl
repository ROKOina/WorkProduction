struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

Texture2D maskMainTexture : register(t0);
Texture2D maskTexture : register(t1);
SamplerState samplerLiner : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 mainTex = maskMainTexture.Sample(samplerLiner, pin.texcoord) * pin.color;
    float4 maskTex = maskTexture.Sample(samplerLiner, pin.texcoord) * pin.color;
    
    float4 color;
    //ƒ}ƒXƒNˆ—
    color = maskTex.a > 0 ? (mainTex.a > 0 ? maskTex : float4(0, 0, 0, 0)) : mainTex;

    return color;
}