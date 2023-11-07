
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
    float value;
    float curvePower;
    float sDummy;
};

#define colorKeyCount 5
struct ColorLifeTime
{
    float keyTime;
    float3 cDummy;
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

    float4 color;
    float4 emitterPosition;
    
    float4 startAngle;
    float4 startAngleRand;
    
    float4 startSize;
    
    float startSpeed;
    
    float time;
    float elapsedTime;
    float lifeTime;
    
    UV particleUV;
    
    Shape particleShape;
    
    VelocityLifeTime velocityLifeTime;

    ScaleLifeTime scaleLifeTime[scaleKeyCount];
    ScaleLifeTime scaleLifeTimeRand[scaleKeyCount];

    ColorLifeTime colorLifeTime[colorKeyCount];
    
    RotationLifeTime rotationLifeTime;
};

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 modelMat;
    row_major float4x4 viewProjection;

    //重力用
    row_major float4x4 inverseModelMat;
    row_major float4x4 inverseViweProj;
    row_major float4x4 view;
    row_major float4x4 gProj;

    float4 lightDirection;
    float4 cameraPosition;
    float4 downVec;
};


//お菓子をだすパーティクル用
struct SweetsParticleData
{
    int isEnable;
    int sweetsCount;
    float2 uvCount;
};

//このゲーム用定数バッファ
cbuffer GameParticleData : register(b10)
{
    SweetsParticleData sweetsData;
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
    
    float size;
    float startSize;
    float randSizeCurve;

    float3 velocity;
    float age;
    float lifeTime;
    int startFlag;
    int emmitterFlag;
    
    float2 uvSize;
    float2 uvPos;
    
    float4 downPP;
    
    row_major float4x4 saveModel;
};

float rand(float2 co) //引数はシード値と呼ばれる　同じ値を渡せば同じものを返す
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

#define NUMTHREADS_X 16
