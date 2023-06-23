struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};
Texture2D texture0 : register(t0);
SamplerState sampler0[3] : register(s0);


#define KERNEL_SIZE (5)
#define BUFFER_SIZE (KERNEL_SIZE * KERNEL_SIZE)
cbuffer BlurData : register(b7)
{
    float4 Weight[BUFFER_SIZE];
    float KarnelSize;
    float2 texel;
    float dummy;
};

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = float4(0, 0, 0, 1);
    for (int i = 0; i < KarnelSize * KarnelSize; i++)
    {
        float2 offset = texel * Weight[i].xy;
		//offset *= 4.0f; //”ÍˆÍ’²®
        float weight = Weight[i].z;
        color.rgb +=
			texture0.Sample(
				sampler0[1], pin.texcoord + offset).rgb * weight;
    }
    return color;
}