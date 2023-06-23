
cbuffer ColorGradingParamater : register(b5)
{
    float brightness;   //輝度
    float contrast;     //コンストラスト
    float saturation;   //彩度
    float dummy;
    float4 filter;       //フィルター
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
    float4 color = tex;

    // 輝度調整
    color.rgb += brightness;

    // コントラスト
    color.rgb = (color.rgb - contrast) * 1.2 + contrast;

    // 彩度
    float gray = dot(color.rgb, float3(0.299f, 0.587f, 0.114f)) * saturation;
    color.rgb = (color.rgb - gray) * 0.3 + gray;
    
	// カラーフィルター
    color.rgb *= float3(filter.x, filter.y, filter.z);
    
    return color;
}