#include "Particle.hlsli"
#include "../Rand.hlsli"

RWStructuredBuffer<particle> particleBuffer : register(u0);

//�x�W�F�Ȑ�
float BezierCurve(float start, float end, float curvePower,float t)
{
    float mid = (end - start) * 0.5f;
    return (1 - t) * (1 - t) * start
    + 2 * (1 - t) * t * (mid + curvePower)
    + t * t * end;
}

//�������Z�b�g����
particle ResetParticle(uint id)
{
    particle p;
    
    const float noiseScale = 1.0;
    float f0 = rand(float2((id + time) * noiseScale,
    rand(float2((id + time) * noiseScale, (id + time) * noiseScale))
    ));
    float f1 = rand(float2(f0 * noiseScale, rand(float2((id + time) * noiseScale, (id + time) * noiseScale))));
    float f2 = rand(float2(f1 * noiseScale, rand(float2((id + time) * noiseScale, (id + time) * noiseScale))));

    p.emitPosition = emitterPosition.xyz;
    p.position = emitterPosition.xyz;

    float tick = particleShape.radiusThickness * 0.9f; //�I�t�Z�b�g�l�@0.9f

    //coneShape
    if (particleShape.shapeID == 0)
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
        p.size = 0;
        if (startSize.w > 0.5f)
        {
            p.startSize = lerp(startSize.x, startSize.y, f1);
        }
        else
            p.startSize = startSize.x;
        
        //�J�[�u�g�p��
        p.randSizeCurve = 0;
        if (scaleLifeTimeRand[0].keyTime >= 0)
        {
            //��Ԓl�ۑ�
            p.randSizeCurve = float(XOrShift32(id) % 100 * 0.01f);
        }
    }
    
    //��]
    {
        if (startAngleRand.w > 0.5f)
            p.startAngle = lerp(startAngle.xyz, startAngleRand.xyz, f1);
        else
            p.startAngle = startAngle.xyz;

        p.angle = p.startAngle;
        
        //���C�t�^�C��
        p.randAngle = float3(0, 0, 0);
        if (rotationLifeTime.rotationRand.w > 0.5f)
            p.randAngle = lerp(rotationLifeTime.rotation.xyz, rotationLifeTime.rotationRand.xyz, float3(f1, f1, f1));
    }
    
    p.color = color;
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
    
    p.startFlag = 1;
    p.lifeTime = lifeTime;
    
    p.age = lifeTime * f0;
    p.saveModel = modelMat;
    
    p.downPP = float4(0, 0, 0, 0);
    
    return p;
}

[numthreads(NUMTHREADS_X, 1, 1)]
void main(uint3 dtid : SV_DISPATCHTHREADID)
{
    uint id = dtid.x;
    
    //�o�����𔛂�
    if (uint(rateTime * lifeTime) < id)
        return;
    
    particle p = particleBuffer[id];
    
    //p.angle.z += elapsedTime * 100;
    //p.angle.x += elapsedTime * 100;
    
    //���쒆
    if (p.startFlag == 1)
    {
        //�o���t���O
        if (p.emmitterFlag == 0)
        {
            p.emmitterFlag = 1;
            p.saveModel = modelMat;
        }
        
        //���쒆�̕�Ԓl
        float lifeRatio = (lifeTime - p.lifeTime) / lifeTime;

        //�J�[�u�ŃT�C�Y��ς���
        if(scaleLifeTime[0].keyTime>=0)  //�L�[���ł���Ă�����
        {
            for (int scaleIndex = 0; scaleIndex < 5; ++scaleIndex)
            {
                if (lifeRatio < scaleLifeTime[scaleIndex].keyTime)  //���݃��C�t���Ԃ��L�[�������̏ꍇ����
                {
                    float keyRatio; //�L�[���̕�Ԓl
                    if (scaleIndex != 0)
                    {
                        keyRatio = (lifeRatio - scaleLifeTime[scaleIndex - 1].keyTime)
                            / (scaleLifeTime[scaleIndex].keyTime - scaleLifeTime[scaleIndex - 1].keyTime);
                        
                        p.size = BezierCurve(scaleLifeTime[scaleIndex - 1].value, scaleLifeTime[scaleIndex].value
                            , scaleLifeTime[scaleIndex].curvePower, keyRatio) * p.startSize;
                    }
                    else
                    {
                        keyRatio = lifeRatio / scaleLifeTime[scaleIndex].keyTime;

                        p.size = BezierCurve(1, scaleLifeTime[scaleIndex].value
                            , scaleLifeTime[scaleIndex].curvePower, keyRatio) * p.startSize;
                    }
                        
                    //�ʂ��break
                    break;
                }
            }
            
            //�����_���J�[�u
            if (scaleLifeTimeRand[0].keyTime >= 0)  //�L�[���ł���Ă�����
            {
                for (int scaleIndex = 0; scaleIndex < 5; ++scaleIndex)
                {
                    if (lifeRatio < scaleLifeTimeRand[scaleIndex].keyTime)  //���݃��C�t���Ԃ��L�[�������̏ꍇ����
                    {
                        float keyRatio; //�L�[���̕�Ԓl
                        if (scaleIndex != 0)
                        {
                            keyRatio = (lifeRatio - scaleLifeTimeRand[scaleIndex - 1].keyTime)
                            / (scaleLifeTimeRand[scaleIndex].keyTime - scaleLifeTimeRand[scaleIndex - 1].keyTime);
                        
                            float randSize;
                            randSize = BezierCurve(scaleLifeTimeRand[scaleIndex - 1].value, scaleLifeTimeRand[scaleIndex].value
                            , scaleLifeTimeRand[scaleIndex].curvePower, keyRatio) * p.startSize;

                            p.size = lerp(p.size, randSize, p.randSizeCurve);

                        }
                        else
                        {
                            keyRatio = lifeRatio / scaleLifeTimeRand[scaleIndex].keyTime;
                            
                            float randSize;
                            randSize = BezierCurve(1, scaleLifeTimeRand[scaleIndex].value
                            , scaleLifeTimeRand[scaleIndex].curvePower, keyRatio) * p.startSize;

                            p.size = lerp(p.size, randSize, p.randSizeCurve);
                        }
                        
                    //�ʂ��break
                        break;
                    }
                }
            }
            
        }
        else
        {
            p.size = p.startSize;
        }
        
        //��Ԃŉ�]
        if (rotationLifeTime.rotationRand.w > 0.5f)
        {
            p.angle += p.randAngle * elapsedTime;
        }
        else
        {
            p.angle += rotationLifeTime.rotation.xyz * elapsedTime;
        }
        
        //�J�[�u�ŃJ���[��ς���
        if (colorLifeTime[0].keyTime >= 0)  //�L�[���ł���Ă�����
        {
            for (int colorIndex = 0; colorIndex < 5; ++colorIndex)
            {
                if (lifeRatio < colorLifeTime[colorIndex].keyTime)  //���݃��C�t���Ԃ��L�[�������̏ꍇ����
                {
                    float keyRatio; //�L�[���̕�Ԓl
                    if (colorIndex != 0)
                    {
                        keyRatio = (lifeRatio - colorLifeTime[colorIndex - 1].keyTime)
                            / (colorLifeTime[colorIndex].keyTime - colorLifeTime[colorIndex - 1].keyTime);
                        
                        p.color.x = BezierCurve(colorLifeTime[colorIndex - 1].value.x, colorLifeTime[colorIndex].value.x
                            , colorLifeTime[colorIndex].curvePower.x, keyRatio);
                        p.color.y = BezierCurve(colorLifeTime[colorIndex - 1].value.y, colorLifeTime[colorIndex].value.y
                            , colorLifeTime[colorIndex].curvePower.x, keyRatio);
                        p.color.z = BezierCurve(colorLifeTime[colorIndex - 1].value.z, colorLifeTime[colorIndex].value.z
                            , colorLifeTime[colorIndex].curvePower.x, keyRatio);
                        p.color.w = BezierCurve(colorLifeTime[colorIndex - 1].value.w, colorLifeTime[colorIndex].value.w
                            , colorLifeTime[colorIndex].curvePower.x, keyRatio);
                    }
                    else
                    {
                        keyRatio = lifeRatio / colorLifeTime[colorIndex].keyTime;

                        p.color.x = BezierCurve(1, colorLifeTime[colorIndex].value.x
                            , colorLifeTime[colorIndex].curvePower.x, keyRatio);
                        p.color.y = BezierCurve(1, colorLifeTime[colorIndex].value.y
                            , colorLifeTime[colorIndex].curvePower.x, keyRatio);
                        p.color.z = BezierCurve(1, colorLifeTime[colorIndex].value.z
                            , colorLifeTime[colorIndex].curvePower.x, keyRatio);
                        p.color.w = BezierCurve(1, colorLifeTime[colorIndex].value.w
                            , colorLifeTime[colorIndex].curvePower.x, keyRatio);
                    }
                        
                    //�ʂ��break
                    break;
                }
            }
            
        }
            
        //�p�[�e�B�N�������̃m�[�}���x�N�g��
        float3 normVec = normalize(p.emitPosition - p.position);
        
        //���ŉ�]
        float3 orbZ = cross(normVec, float3(0, 1, 0));
        float3 orbX = cross(normVec, float3(1, 0, 0));
        float3 orbY = cross(normVec, float3(0, 0, 1));
        float3 orbVelo =
        orbZ * velocityLifeTime.orbitalVelocity.z +
        orbX * velocityLifeTime.orbitalVelocity.x +
        orbY * velocityLifeTime.orbitalVelocity.y;
        
        //���S�����ɓ���
        float3 radialVec = -normVec * velocityLifeTime.radial;

        //�d�͂킩���
        float4 downVecCS = mul(float4(p.position, 1), inverseViweProj * inverseModelMat);
        //float4 downVecCS = mul(float4(0, 0, 0, 1),  inverseModelMat*inverseViweProj);

        //float4 downVecCS1 = downVecCS;
        downVecCS.y -= 1;
        float4 downVecCS2 = downVecCS;

        //downVecCS1 = mul(downVecCS1, modelMat*viewProjection);
        downVecCS2 = mul(downVecCS2, modelMat * viewProjection);

        p.velocity += normalize(downVecCS2.xyz - downVecCS.xyz) * gravity;
        
        
        
        p.position += (p.velocity + velocityLifeTime.linearVelocity.xyz + orbVelo + radialVec) * elapsedTime;

        p.lifeTime -= elapsedTime;
    }
    
    p.age += elapsedTime;
    if (p.age > lifeTime && p.startFlag==0)
    {
        //��������
        p = ResetParticle(id);
    }
    
    if (p.lifeTime < 0)
    {
        p.emmitterFlag = 0;
        if (isRoop == 0)    //���[�v�I��
        {
            p.velocity = float3(0, 0, 0);
            p.position = float3(0, 0, 0);
            p.size = 0;
            p.age = 0;
            
        }
        else    
        {
            //���[�v����
            const float noiseScale = 1.0;
            float f0 = rand(float2((id + time) * noiseScale,
                rand(float2((id + time) * noiseScale, (id + time) * noiseScale))
                ));

            p.age = lifeTime * f0;
            p.startFlag = 0;
        }
    }

    particleBuffer[id] = p;
}
