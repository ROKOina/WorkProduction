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
    
    float tick = particleShape.radiusThickness * 0.9f; //�I�t�Z�b�g�l�@0.9f

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

    
    //���x�v�Z
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
            //���̂܂�normVelo���X�s�[�h��
        }
    
        p.velocity = normalize(normVelo) * startSpeed;
    }
    
    //�T�C�Y
    {
        p.startSize = float3(0, 0, 0);
        if (startSizeRand.w > 0.5f)
        {
            p.startSize.xyz = lerp(startSize.xyz, startSizeRand.xyz, f1);
        }
        else
            p.startSize = float3(startSize.xy, 0);
    }
    
    p.randAngle = float3(0, 0, 0);
    //��]
    {
        if (startAngleRand.w > 0.5f)
            p.startAngle = lerp(startAngle.xyz, startAngleRand.xyz, f1);
        else
            p.startAngle = startAngle.xyz;

        p.angle = startAngle;
        
        //���C�t�^�C��
        if (rotationLifeTime.rotationRand.w > 0.5f)
            p.randAngle = lerp(rotationLifeTime.rotation.xyz, rotationLifeTime.rotationRand.xyz, f1);
    }
  
    p.color = color;
   
    
    p.size = float3(0,0,0);
    p.randSizeCurve = float3(0, 0, 0);
    
    p.age = lifeTime * f2;
    p.state = 0;
    p.lifeTime = lifeTime;
    
    p.emitPosition = float3(0, 0, 0);
    p.emmitterFlag = 0;
    
    p.uvSize = float2(1, 1);
    p.uvPos = float2(0, 0);

    //���َq�p�[�e�B�N���N�����̎�
    if (sweetsData.isEnable)
    {
        //�T�C�Y
        p.uvSize = 1.0f / sweetsData.uvCount;
        //�X�N���[���ʒu�������_���Ō��߂�
        int index = XOrShift32(id) % sweetsData.sweetsCount;
        p.uvPos.x = int(index % sweetsData.uvCount.x) * p.uvSize.x;
        p.uvPos.y = int(index / sweetsData.uvCount.x) * p.uvSize.y;

    }
    
    p.downPP = float4(0, 0, 0, 0);
    
    p.saveModel = float4x4(
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1);
    
    particleBuffer[id] = p;
}
