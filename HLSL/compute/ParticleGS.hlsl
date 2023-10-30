#include "Particle.hlsli"

StructuredBuffer<particle> particle_buffer : register(t9);

float4 rotato(float3 angle,float4 position,float2 scale,float3 corner)
{
    
    // âÒì]çsóÒÇçÏÇÈ
    half c = cos(angle.x);
    half s = sin(angle.x);
    half4x4 rotateMatrixX = half4x4(1, 0, 0, 0,
                                    0, c, 0, 0,
                                    0, s, 1, 0,
                                    0, 0, 0, 1);
    
    c = cos(angle.y);
    s = sin(angle.y);
    half4x4 rotateMatrixY = half4x4(c, 0, 0, 0,
                                    0, 1, 0, 0,
                                    -s, 0, 1, 0,
                                    0, 0, 0, 1);
    c = cos(angle.z);
    s = sin(angle.z);
    half4x4 rotateMatrixZ = half4x4(c, -s, 0, 0,
                                    s, c, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1);
    float4 pos;
    pos = mul(float4(corner.xyz, 1), rotateMatrixX);
    pos = mul(float4(pos.xyz, 1), rotateMatrixY);
    pos = mul(float4(pos.xyz, 1), rotateMatrixZ);
    pos *= float4(scale, 1,1);
    
    return float4(position.xyz + pos.xyz, position.w);
}


[maxvertexcount(4)]
void main(point VS_OUT input[1] : SV_POSITION, inout TriangleStream<GS_OUT> output)
{
    const float3 corners[] =
    {
        float3(-1.0f, -1.0f, 0.0f),
		float3(-1.0f, +1.0f, 0.0f),
		float3(+1.0f, -1.0f, 0.0f),
		float3(+1.0f, +1.0f, 0.0f),
    };
    const float2 texcoords[] =
    {
        float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f),
    };
	
    particle p = particle_buffer[input[0].vertex_id];

    const float aspect_ratio = 1280.0 / 720.0;
    float2 particle_scale = float2(particle_size, particle_size * aspect_ratio);

	[unroll]
    for (uint vertex_index = 0; vertex_index < 4; ++vertex_index)
    {
        GS_OUT element;

		// Transform to clip space
        element.position = mul(float4(p.position, 1), view_projection);

        element.position = rotato(p.angle, element.position, particle_scale, corners[vertex_index]);
        
        element.color = p.color;
        element.texcoord = texcoords[vertex_index];
        output.Append(element);
    }

    output.RestartStrip();
}
