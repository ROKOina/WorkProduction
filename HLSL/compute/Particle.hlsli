
struct UV
{
    float2 size;
    float2 pos;
};

struct Shape
{
    int shapeID;
    float3 shaDummy;
    
    float angle;
    float radius;
    float radiusThickness;
    float arc;
};
    
struct VelocityLifeTime
{
    float4 linearVelocity;
    float4 orbitalVelocity;
    
    float radial;
    float3 vDummy;
};

#define scaleKeyCount 5
struct ScaleLifeTime
{
    float keyTime;
    float3 sDummy;
    float4 value;
    float4 curvePower;
};

struct RotationLifeTime
{
    float4 rotation;
    float4 rotationRand;
};

cbuffer PARTICLE_CONSTANTS : register(b9)
{
    int isWorld;
    int isRoop;

    int rateTime;
    float gravity;

    
    float4 emitterPosition;
    
    float4 startAngle;
    float4 startAngleRand;
    
    float4 startSize;
    float4 startSizeRand;
    
    float startSpeed;
    
    float time;
    float elapsedTime;
    float lifeTime;
    
    UV particleUV;
    
    Shape particleShape;
    
    VelocityLifeTime velocityLifeTime;

    ScaleLifeTime scaleLifeTime[scaleKeyCount];
    ScaleLifeTime scaleLifeTimeRand[scaleKeyCount];

    RotationLifeTime rotationLifeTime;
};

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 modelMat;
    row_major float4x4 viewProjection;
    float4 lightDirection;
    float4 cameraPosition;
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
    float3 emitPosition;
    
    float3 angle;
    float3 startAngle;
    float3 randAngle;
    
    float3 size;
    float3 startSize;
    float3 randSizeCurve;

    float3 velocity;
    float age;
    float lifeTime;
    int state;
    int emmitterFlag;
    
    row_major float4x4 saveModel;
};

float rand(float2 co) //引数はシード値と呼ばれる　同じ値を渡せば同じものを返す
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

#define NUMTHREADS_X 16
