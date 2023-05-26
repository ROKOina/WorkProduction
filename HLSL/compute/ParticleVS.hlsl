#include "Particle.hlsli"

VS_OUT_PARTICLE main(uint vertexId : SV_VERTEXID)
{
    VS_OUT_PARTICLE vout;
    vout.vertexId = vertexId;
    return vout;
}


//#include "ParticlePS.hlsl"

//cbuffer WVP : register(b5)
//{
//    float4x4 World;
//    float4x4 View;
//    float4x4 Projection;
//};


//struct VS_IN_PARTICLE
//{
//    float4 Position : POSITION0;
//    float2 TexCoord : TEXCOORD0;
//    uint InstanceID : SV_InstanceID;
//};

//// DrawInstanceópç¿ïW
//StructuredBuffer<float3> Position : register(t2);

//void main(in VS_IN_PARTICLE In, out PS_IN_PARTICLE Out)
//{

//    matrix wvp;
//    wvp = mul(World, View);
//    wvp = mul(wvp, Projection);

//    In.Position.xyz += Position[In.InstanceID];

//    Out.Position = mul(In.Position, wvp);
//    Out.WorldPosition = mul(In.Position, World);
//    Out.TexCoord = In.TexCoord;

//}