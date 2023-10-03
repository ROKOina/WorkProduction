struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

cbuffer ColorGradingParamater : register(b5)
{
    float brightness; //輝度
    float contrast; //コントラスト
    float saturation; //彩度
    float dummy;
    float4 filter; //フィルター
};

//--------------------------------------------
//	sRGB色空間から線形色空間への変換
//--------------------------------------------
// srgb		: sRGB色空間色
// 返す値	: 線形色空間色
inline float3 sRGBToLinear(float3 colorsRGB)
{
    return pow(colorsRGB.xyz, 2.2f);
}

Texture2D mainTexture : register(t0);
SamplerState samplerLiner : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 tex = mainTexture.Sample(samplerLiner, pin.texcoord) * pin.color;
    float4 color = tex;

    // 輝度調整
    color.rgb += brightness;

    // コントラスト
    color.rgb = (color.rgb - 0.5) * contrast + 0.5;

    // 彩度
    float gray = dot(color.rgb, float3(0.299f, 0.587f, 0.114f));
    color.rgb = (color.rgb - gray) * saturation + gray;
    
	// カラーフィルター
    color.rgb *= float3(filter.x, filter.y, filter.z);
    
    return color;
    //return float4(sRGBToLinear(color.rgb), color.a);
}