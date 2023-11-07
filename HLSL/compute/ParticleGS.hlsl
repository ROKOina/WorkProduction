#include "Particle.hlsli"

StructuredBuffer<particle> particleBuffer : register(t9);

float4 rotate(float3 angle,float4 position,float2 scale,float3 corner)
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
    half4x4 rotateMatrixY = half4x4(c , 0, 0, 0,
                                    0, 1 , 0, 0,
                                    -s, 0, 1, 0,
                                    0, 0, 0, 1);
    c = cos(angle.z);
    s = sin(angle.z);
    half4x4 rotateMatrixZ = half4x4(c , -s , 0, 0,
                                    s, c , 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1);

    //拡大行列
    half4x4 scaleMatrix = half4x4(scale.x,0, 0, 0,
                                    0, scale.y, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1);
    
    
    float4 cornerPos = float4(corner, 1);
    
    //cornerPos *= float4(scale, 1, 1);
    
    half4x4 Rota = mul(rotateMatrixZ, mul(rotateMatrixY, rotateMatrixX));
    
    cornerPos = mul(float4(corner.xyz, 1), mul(Rota,scaleMatrix));

    
    //cornerPos = mul(float4(corner.xyz, 1), rotateMatrixX);
    //cornerPos = mul(float4(cornerPos.xyz, 1), rotateMatrixY);
    //cornerPos = mul(float4(cornerPos.xyz, 1), rotateMatrixZ);

    //cornerPos = float4(scale, 1, 1) * float4(cornerPos.xyz, 1);
    
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

    //const float aspectRatio = 1280.0 / 720.0;
    //const float aspectRatio = 1.0f; //(1920 * 0.8f) / (1080 * 0.8f);
    const float aspectRatio = (1920 * 0.8f) / (1080 * 0.8f);
    
    //サイズ算出
    float2 particleScale = float2(p.size.x, p.size.x * aspectRatio);

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
  //          element.position = rotato(p.angle, float4(p.position, 1), particleScale, corners[vertexIndex]);

		//// Transform to clip space
  //          float4x4 m = view;
  //          m._41 = 0;
  //          m._42 = 0;
  //          m._43 = 0;
  //          //m._41 =modelMat._41;
  //          //m._42 = modelMat._42;
  //          //m._43 = modelMat._43;
  //          m = mul(m, modelMat);
            
            //element.position = mul(element.position, mul(m, viewProjection));
            
            element.position = mul(float4(p.position, 1), mul(modelMat, viewProjection));
            //element.position.xy += corners[vertexIndex].xy * particleScale;
            element.position.xy = rotate(p.angle, element.position, particleScale, corners[vertexIndex]).xy;

        }
        else
        {
            element.position = mul(float4(p.position, 1), mul(p.saveModel, viewProjection));
            //element.position.xy += corners[vertexIndex].xy * particleScale;
            element.position.xy = rotate(p.angle, element.position, particleScale, corners[vertexIndex]).xy;
        }
        
        element.color = p.color;
        
        //パーティクル毎のUVスクロール
        float2 uv = texcoords[vertexIndex];
        uv *= p.uvSize;
        uv += p.uvPos;
        element.texcoord = uv;
        
        output.Append(element);
    }

    output.RestartStrip();
}
