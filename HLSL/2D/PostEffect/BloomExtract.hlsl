cbuffer ExtractionData : register(b0)
{
    float threshold; // ���P�x���o�̂��߂�臒l
    float intensity; // �u���[���̋��x
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

	// ���P�x���o
    //float d = 0.5;
    //if (tex.r < d)
    //    tex.r = 0;
    //if (tex.g < d)
    //    tex.g = 0;
    //if (tex.b < d)
    //    tex.b = 0;
    
    static const float3 luminanceValue = float3(0.299f, 0.587f, 0.114f);
    
    float luminance = dot(luminanceValue, tex.rgb);
    
    //臒l�Ƃ̍����Z�o
    //float contribution = max(0, luminance - 0.5);
    float contribution = max(0, luminance - threshold);

	// �o�͂���F��␳����
    contribution /= luminance;
    float4 color = float4(0,0,0,0);
    //color.rgb *= contribution * 0.8;
    color.rgb = contribution * intensity;
    color.a = 1;
    
    return color;
}