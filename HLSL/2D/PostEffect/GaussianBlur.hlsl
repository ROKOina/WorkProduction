struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

#define KERNEL_SIZE (17)
#define BUFFER_SIZE (KERNEL_SIZE * KERNEL_SIZE)
cbuffer BlurData : register(b7)
{
    float4 Weight[BUFFER_SIZE]; 
    float KarnelSize;           
    float2 texel;
    float dummy;
};

Texture2D blurTexture : register(t0);
SamplerState samplers[3] : register(s0);    //0:WRAP 1:CLAMP 2:BORDER,POINT

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = float4(0, 0, 0, 1);
    for (int i = 0; i < KarnelSize * KarnelSize; i++)
    {
        float2 offset = texel * Weight[i].xy;
        //offset *= 4.0f; //”ÍˆÍ’²®
        float weight = Weight[i].z;
        color.rgb +=
			blurTexture.Sample(
				samplers[0], pin.texcoord + offset).rgb * weight;
    }
    return color;
}