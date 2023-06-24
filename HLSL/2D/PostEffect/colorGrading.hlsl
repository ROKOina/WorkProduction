struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

cbuffer ColorGradingParamater : register(b5)
{
    float brightness; //�P�x
    float contrast; //�R���g���X�g
    float saturation; //�ʓx
    float dummy;
    float4 filter; //�t�B���^�[
};

Texture2D mainTexture : register(t0);
SamplerState samplerLiner : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 tex = mainTexture.Sample(samplerLiner, pin.texcoord) * pin.color;
    float4 color = tex;

    // �P�x����
    color.rgb += brightness;

    // �R���g���X�g
    color.rgb = (color.rgb - contrast) * 1.2 + contrast;

    // �ʓx
    float gray = dot(color.rgb, float3(0.299f, 0.587f, 0.114f)) * saturation;
    color.rgb = (color.rgb - gray) * 0.3 + gray;
    
	// �J���[�t�B���^�[
    color.rgb *= float3(filter.x, filter.y, filter.z);
    
    return color;
}