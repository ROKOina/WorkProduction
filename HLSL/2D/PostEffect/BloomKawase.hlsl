struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};
#define BlurCount 6

Texture2D blurTexture[BlurCount] : register(t0);
//Texture2D blurTexture3 : register(t3);
SamplerState samplerLiner[4] : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    //テクスチャを加算
    //float4 tex = blurTexture[0].Sample(samplerLiner[3], pin.texcoord) * pin.color;
    float4 tex = blurTexture[1].Sample(samplerLiner[0], pin.texcoord) * pin.color;
    tex += blurTexture[2].Sample(samplerLiner[0], pin.texcoord) * pin.color;
    tex += blurTexture[3].Sample(samplerLiner[0], pin.texcoord) * pin.color;
    tex += blurTexture[4].Sample(samplerLiner[0], pin.texcoord) * pin.color;
    tex += blurTexture[5].Sample(samplerLiner[0], pin.texcoord) * pin.color;
    //tex.rgb *= 2;

    //平均化する
    float4 color = tex / 4;
    color.a = 1.0f;

    return color;
}