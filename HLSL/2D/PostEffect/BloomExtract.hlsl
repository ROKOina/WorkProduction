struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

cbuffer ExtractionData : register(b0)
{
    float threshold; // ���P�x���o�̂��߂�臒l
    float intensity; // �u���[���̋��x
    float2 dummy;
};

Texture2D mainTexture : register(t0);
SamplerState samplerLiner : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 tex = mainTexture.Sample(samplerLiner, pin.texcoord) * pin.color;

	// ���P�x���o    
    static const float3 luminanceValue = float3(0.299f, 0.587f, 0.114f);
    float luminance = dot(luminanceValue, tex.rgb);
    
    //臒l�Ƃ̍����Z�o
    float contribution = smoothstep(threshold, threshold + 0.5, luminance);
    //float contribution = max(0, luminance - threshold);
    

    
	//// �o�͂���F��␳����
 //   contribution /= luminance;
    
    float4 color = float4(0,0,0,0);
    color.rgb = contribution * tex.rgb * intensity;
    color.a = 1;
    
    return color;
}