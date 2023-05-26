struct VS_OUT_PARTICLE
{
    uint vertexId : VERTEXID;
};

struct GS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

struct particle
{
    float3 position;
    float3 velocity;
};

cbuffer PARTICLE_CONSTANTS : register(b9)
{
    float4 currentEyePosition;
    float4 previousEyePosition;
    
	// Radius of outermost orbit 
    float outermostRadius;
	// Height of snowfall area
    float snowfallAreaHeight;

    float particleSize;
    uint particleCount;
    
    float4x4 view;
    float4x4 projection;
    float sceneTimer;
    float deltaTime;
    float2 dummy;
}