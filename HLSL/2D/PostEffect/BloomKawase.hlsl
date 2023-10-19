struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

Texture2D blurTexture0 : register(t0);
Texture2D blurTexture1 : register(t1);
Texture2D blurTexture2 : register(t2);
//Texture2D blurTexture3 : register(t3);
SamplerState samplerLiner[3] : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    //テクスチャを加算
    float4 tex = blurTexture0.Sample(samplerLiner[0], pin.texcoord) * pin.color;
    tex += blurTexture1.Sample(samplerLiner[0], pin.texcoord) * pin.color;
    tex += blurTexture2.Sample(samplerLiner[0], pin.texcoord) * pin.color;
    //tex += blurTexture3.Sample(samplerLiner[0], pin.texcoord) * pin.color;

    //平均化する
    float4 color = tex / 4;
    color.a = 1.0f;

    return color;
}