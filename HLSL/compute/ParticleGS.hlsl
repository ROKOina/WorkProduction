#include "../Lambert.hlsli"
#include "Particle.hlsli"

StructuredBuffer<particle> particleBuffer : register(t9);

[maxvertexcount(4)]
void main(point VS_OUT_PARTICLE input[1] : SV_POSITION,
inout TriangleStream<GS_OUT> output)
{
    const float3 billboard[] =
    {
        float3(-1.0f, -1.0f, 0.0f), // Bottom-left corner
		float3(+1.0f, -1.0f, 0.0f), // Bottom-right corner
		float3(-1.0f, +1.0f, 0.0f), // Top-left corner
		float3(+1.0f, +1.0f, 0.0f), // Top-right corner
    };
    const float2 texcoords[] =
    {
        float2(0.0f, 1.0f), // Bottom-left 
		float2(1.0f, 1.0f), // Bottom-right
		float2(0.0f, 0.0f), // Top-left
		float2(1.0f, 0.0f), // Top-right
    };

    particle p = particleBuffer[input[0].vertexId];
    
    float3 viewSpaceVelocity = mul(float4(p.velocity, 0.0f), view).xyz;
    float4 viewSpacePos = mul(float4(p.position, 1.0), view);

    [unroll]
    for (uint vertex_index = 0; vertex_index < 4; ++vertex_index)
    {
        GS_OUT element;
        
        float3 corner_pos = billboard[vertex_index] * particleSize;
        
        element.position = mul(float4(viewSpacePos.xyz + corner_pos, 1), projection);
		
        element.color = float4(1.0, 1.0, 1.0, 0.5);
        element.texcoord = texcoords[vertex_index];
        output.Append(element);
    }
    output.RestartStrip();
}

//struct GSOutput
//{
//	float4 pos : SV_POSITION;
//};

//[maxvertexcount(3)]
//void main(
//	triangle float4 input[3] : SV_POSITION, 
//	inout TriangleStream< GSOutput > output
//)
//{
//	for (uint i = 0; i < 3; i++)
//	{
//		GSOutput element;
//		element.pos = input[i];
//		output.Append(element);
//	}
//}