#include "Particle.hlsli"

StructuredBuffer<particle> particleBuffer : register(t9);

float4 rotato(float3 angle,float4 position,float2 scale,float3 corner)
{
    
    angle.x = radians(angle.x);
    angle.y = radians(angle.y);
    angle.z = radians(angle.z);
    
    // 回転行列を作る
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

    float4 cornerPos = float4(corner, 1);
    
    //cornerPos *= float4(scale, 1, 1);
    
    cornerPos = mul(float4(corner.xyz, 1), rotateMatrixX);
    cornerPos = mul(float4(cornerPos.xyz, 1), rotateMatrixY);
    cornerPos = mul(float4(cornerPos.xyz, 1), rotateMatrixZ);

    cornerPos = float4(scale, 1, 1) * float4(cornerPos.xyz, 1);
    
    return float4(position.xyz + cornerPos.xyz, position.w);
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
	
    particle p = particleBuffer[input[0].vertex_id];

    const float aspectRatio = 1280.0 / 720.0;
    
    //サイズ算出
    float2 particleScale = float2(p.size.x, p.size.y * aspectRatio);

    //エミッションが発生してない場合
    if (p.emmitterFlag == 0)
    {
        particleScale = 0;
    }
        
	[unroll]
    for (uint vertexIndex = 0; vertexIndex < 4; ++vertexIndex)
    {
        GS_OUT element;
        
        //エミッションが発生していないか、ワールド基準の場合
        if (p.emmitterFlag == 0 || isWorld == 0)
        {
		// Transform to clip space
            element.position = mul(float4(p.position, 1), mul(modelMat, viewProjection));

        //element.position.xy += corners[vertexIndex].xy * particleScale;
            element.position.xy = rotato(p.angle, element.position, particleScale, corners[vertexIndex]).xy;
        }
        else
        {
            element.position = mul(float4(p.position, 1), mul(p.saveModel, viewProjection));
            element.position.xy = rotato(p.angle, element.position, particleScale, corners[vertexIndex]).xy;
        }
        
        element.color = p.color;
        element.texcoord = texcoords[vertexIndex];
        output.Append(element);
    }

    output.RestartStrip();
}
