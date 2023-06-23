
struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

Texture2D bloomTexture : register(t1);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 tex = texture0.Sample(sampler0, pin.texcoord) * pin.color;
    float4 color = tex;
    
    //ブルームを足す
    tex = bloomTexture.Sample(sampler0, pin.texcoord) * pin.color;
    color.rgb += tex.rgb;
    
    //格子
    //color.rgb *= step(abs(sin(pin.texcoord.x * 50)), 0.8);
    
    //真ん中だけ明るい
    //color.rgb *= 1 - pow(abs((pin.texcoord * 2 - 1) / 2).x, 2 /* ここで強さ変える */);
    //color.rgb *= 1 - pow(abs((pin.texcoord * 2 - 1) / 2).y,2);

    return color;
}