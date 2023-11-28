struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

Texture2D sunTexture : register(t0);
SamplerState samplerLiner : register(s0);

Texture2D bloomTexture : register(t1);
Texture2D radialTexture : register(t2);

cbuffer VIGNETTE_CONSTANS : register(b0)
{
    float vignettePower;
    float vignetteEnabled;
    float2 dummyV;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 sunTex = sunTexture.Sample(samplerLiner, pin.texcoord) * pin.color;
    float4 radialTex = radialTexture.Sample(samplerLiner, pin.texcoord) * pin.color;
    float4 bloomTex = bloomTexture.Sample(samplerLiner, pin.texcoord) * pin.color;
    
    //ブルームと画面を足す
    float4 color = float4(0, 0, 0, 1);
    color.rgb = sunTex.rgb + radialTex.rgb + bloomTex.rgb;
    
    //格子
    //color.rgb *= step(abs(sin(pin.texcoord.x * 50)), 0.8);
    
    //真ん中だけ明るい
    if (vignetteEnabled > 0.1f)
    {
        float vignetteOffsetPower = 3;
        float xV = 1 - pow(abs(pin.texcoord - 0.5f).x, vignetteOffsetPower - vignettePower * vignetteOffsetPower);
        float yV = 1 - pow(abs(pin.texcoord - 0.5f).y, vignetteOffsetPower - vignettePower * vignetteOffsetPower);
        
        color.gb *= xV*yV;
        color.r += (1 - xV) * (1 - yV);
    }

    return color;
}