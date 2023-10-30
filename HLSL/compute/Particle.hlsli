
cbuffer PARTICLE_CONSTANTS : register(b9)
{
    float3 emitter_position;
    float particle_size;
    float time;
    float delta_time;
    
    float2 texSize;
    float2 texPos;

    float2 dummy;
};

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 view_projection;
    float4 light_direction;
    float4 camera_position;
};

struct VS_OUT
{
    uint vertex_id : VERTEXID;
};
struct GS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

struct particle
{
    float4 color;
    float3 position;
    float3 angle;
    float3 velocity;
    float age;
    float lifeTime;
    int state;
};

float rand(float2 co) //引数はシード値と呼ばれる　同じ値を渡せば同じものを返す
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

#define NUMTHREADS_X 16
