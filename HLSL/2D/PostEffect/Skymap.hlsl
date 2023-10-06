#include "FullScreen.hlsli"

cbuffer Skymap : register(b0)
{
    row_major float4x4 invMat;
};

SamplerState samplerLiner[3] : register(s0);
Texture2D skybox : register(t0);

float4 sample_skybox(float3 v, float roughness)
{
    const float PI = 3.14159265358979;

    uint width, height, number_of_levels;
    skybox.GetDimensions(0, width, height, number_of_levels);

    float lod = roughness * float(number_of_levels - 1);
    v = normalize(v);

	// Blinn/Newell Latitude Mapping
    float2 sample_point;
    sample_point.x = (atan2(v.z, v.x) + PI) / (PI * 2.0);
    sample_point.y = 1.0 - ((asin(v.y) + PI * 0.5) / PI);
    return skybox.SampleLevel(samplerLiner[0], sample_point, lod);
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 ndc;
    ndc.x = (pin.texcoord.x * 2.0) - 1.0;
    ndc.y = 1.0 - (pin.texcoord.y * 2.0);
    ndc.z = 1;
    ndc.w = 1;

    float4 R = mul(ndc, invMat);
    R /= R.w;
    const float roughness = 0;
    return sample_skybox(R.xyz, roughness);
   
}
