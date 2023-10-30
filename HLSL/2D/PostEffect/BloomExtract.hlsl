struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

cbuffer ExtractionData : register(b0)
{
    float threshold; // 高輝度抽出のための閾値
    float intensity; // ブルームの強度
    float2 dummy;
};

Texture2D mainTexture : register(t0);
SamplerState samplerLiner[4] : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 tex = mainTexture.Sample(samplerLiner[3], pin.texcoord) * pin.color;

	// 高輝度抽出    
    static const float3 luminanceValue = float3(0.299f, 0.587f, 0.114f);
    float luminance = dot(luminanceValue, tex.rgb);
    
    //閾値との差を算出
    float contribution = smoothstep(threshold, threshold + 0.5, luminance);
    
    
    float4 color = float4(0,0,0,0);
    color.rgb = (contribution * tex.rgb) * intensity;
    color.a = pin.color.a;
    
    return color;
}