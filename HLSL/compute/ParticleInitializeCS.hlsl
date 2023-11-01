#include "Particle.hlsli"
#include "../Rand.hlsli"

RWStructuredBuffer<particle> particleBuffer : register(u0);

[numthreads(NUMTHREADS_X, 1, 1)]
void main(uint3 dtid : SV_DISPATCHTHREADID)
{
    uint id = dtid.x;
    
    particle p;
    
    const float noiseScale = 1.0;
    float f0 = rand(float2((id + time) * noiseScale,
    rand(float2((id + time) * noiseScale, (id + time) * noiseScale))
    ));
    float f1 = rand(float2(f0 * noiseScale, rand(float2((id + time) * noiseScale, (id + time) * noiseScale))));
    float f2 = rand(float2(f1 * noiseScale, rand(float2((id + time) * noiseScale, (id + time) * noiseScale))));

    p.position = emitterPosition.xyz;
    
    float tick = particleShape.radiusThickness * 0.9f; //オフセット値　0.9f

    //coneShape
    if(particleShape.shapeID == 0)
    {
        p.position.x += ((rand(float2(f1, time)) * tick + (1 - tick)) //radiusThickness
            * sin(6.28318530718 * f0 * particleShape.arc)) * particleShape.radius;
        p.position.z += ((rand(float2(f1, time)) * tick + (1 - tick)) //radiusThickness
            * cos(6.28318530718 * f0 * particleShape.arc)) * particleShape.radius;
    }

    //sphereShape
    if (particleShape.shapeID == 1)
    {
        float r = sin(3.141592654 * (rand(float2(f1, time))) * particleShape.arc) * (rand(float2(f2, time)) * tick + (1 - tick)) * particleShape.radius;
        p.position.x += r * sin(3.141592654 * 2 * (rand(float2(f0, time))));
        p.position.y += cos(3.141592654 * (rand(float2(f1, time))) * particleShape.arc) * (rand(float2(f2, time)) * tick + (1 - tick)) * particleShape.radius;
        p.position.z += r * cos(3.141592654 * 2 * (rand(float2(f0, time))));
    }

    
    //速度計算
    {
        float3 velo = p.position - emitterPosition.xyz;
        float3 normVelo = normalize(velo);
    
        if (particleShape.shapeID == 0)
        {
            float ratio = 1 - (length(velo) / particleShape.radius);
    
            normVelo.y = radians(particleShape.angle + 20 * ratio);
    
            if (particleShape.angle > 89.9f)
                normVelo = float3(0, 1, 0);
    
            if (particleShape.angle < 0.1f)
                normVelo.y = 0;
        }
        if (particleShape.shapeID == 1)
        {
            //そのままnormVeloをスピードに
        }
    
        p.velocity = normalize(normVelo) * startSpeed;
    }
    
    //サイズ
    {
        p.startSize = float3(0, 0, 0);
        if (startSizeRand.w > 0.5f)
        {
            p.startSize.xy = lerp(startSize.xy, startSizeRand.xy, f1);
        }
        else
            p.startSize.xy = float3(startSize.xy, 0);
    }
    
    p.randAngle = float3(0, 0, 0);
    //回転
    {
        if (startAngleRand.w > 0.5f)
            p.startAngle = lerp(startAngle.xyz, startAngleRand.xyz, f1);
        else
            p.startAngle = startAngle.xyz;

        p.angle = startAngle;
        
        //ライフタイム
        if (rotationLifeTime.rotationRand.w > 0.5f)
            p.randAngle = lerp(rotationLifeTime.rotation.xyz, rotationLifeTime.rotationRand.xyz, f1);
    }
  
    //p.color.x = 1.0;
    //p.color.y = f0 * 0.5;
    //p.color.z = f0 * 0.05;
    //p.color.xyz *= 3.0;
    //p.color.w = 1.0f;
    p.color = float4(1, 1, 1, 1);
   
    
    p.size = float3(0,0,0);
    p.randSizeCurve = float3(0, 0, 0);
    
    p.age = lifeTime * f2;
    p.state = 0;
    p.lifeTime = lifeTime;
    
    p.emitPosition = float3(0, 0, 0);
    p.emmitterFlag = 0;
    
    p.saveModel = float4x4(
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1);
    
    particleBuffer[id] = p;
}
