struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

Texture2D mainTexture : register(t0);
SamplerState samplerLiner : register(s0);

Texture2D bloomTexture : register(t1);
float4 main(VS_OUT pin) : SV_TARGET
{
    float4 tex = mainTexture.Sample(samplerLiner, pin.texcoord) * pin.color;
    float4 bloomTex = bloomTexture.Sample(samplerLiner, pin.texcoord) * pin.color;
    
    //ブルームと画面を足す
    float4 color = float4(0, 0, 0, 1);
    color.rgb = tex.rgb + bloomTex.rgb;
    
    //格子
    //color.rgb *= step(abs(sin(pin.texcoord.x * 50)), 0.8);
    
    //真ん中だけ明るい
    //color.rgb *= 1 - pow(abs((pin.texcoord * 2 - 1) / 2).x, 2 /* ここで強さ変える */);
    //color.rgb *= 1 - pow(abs((pin.texcoord * 2 - 1) / 2).y, 2);

    return color;
}