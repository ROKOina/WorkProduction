struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};


cbuffer Trail : register(b0)
{
    row_major float4x4 VP;
    float4 trailColor;
};

