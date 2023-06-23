cbuffer ExtractionData : register(b0)
{
    float threshold; // 高輝度抽出のための閾値
    float intensity; // ブルームの強度
    float2 dummy;
};
    
struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 tex = texture0.Sample(sampler0, pin.texcoord) * pin.color;

	// 高輝度抽出
    //float d = 0.5;
    //if (tex.r < d)
    //    tex.r = 0;
    //if (tex.g < d)
    //    tex.g = 0;
    //if (tex.b < d)
    //    tex.b = 0;
    
    static const float3 luminanceValue = float3(0.299f, 0.587f, 0.114f);
    
    float luminance = dot(luminanceValue, tex.rgb);
    
    //閾値との差を算出
    //float contribution = max(0, luminance - 0.5);
    float contribution = max(0, luminance - threshold);

	// 出力する色を補正する
    contribution /= luminance;
    float4 color = float4(0,0,0,0);
    //color.rgb *= contribution * 0.8;
    color.rgb = contribution * intensity;
    color.a = 1;
    
    return color;
}