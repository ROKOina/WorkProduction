struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

Texture2D mainTexture : register(t0);
SamplerState samplerState[6] : register(s0);

cbuffer BLUR_CONSTANS : register(b0)
{
    float2 blurPos;
    float power;
    float enabled;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    if (enabled > 0)
    {
        float4 radialcolor = 0;
        //���W�A���u���[�̒��S�ʒu �� ���݂̃e�N�Z���ʒu
    //float2 screenposRatio = float2(0.5f, 0.5f);
        float2 screenposRatio = float2(blurPos.x / (1920 * 0.8f), blurPos.y / (1080 * 0.8f));
        float2 dir = screenposRatio - pin.texcoord;

        //�������v�Z����
        float len = length(dir);

        //�����x�N�g���̐��K�����A�P�e�N�Z�����̒����ƂȂ�����x�N�g�����v�Z����
        dir = normalize(dir) * float2(0.01f, 0.01f);

       //������ώZ���邱�Ƃɂ��A�����̒��S�ʒu�ɋ߂��قǃu���[�̉e�����������Ȃ�悤�ɂ���
        dir *= power * len;

        //��������
        float4 Color[10];
        Color[0] = mainTexture.Sample(samplerState[5], pin.texcoord) * 0.19f;
        Color[1] = mainTexture.Sample(samplerState[5], pin.texcoord + dir) * 0.17f;
        Color[2] = mainTexture.Sample(samplerState[5], pin.texcoord + dir * 2.0f) * 0.15f;
        Color[3] = mainTexture.Sample(samplerState[5], pin.texcoord + dir * 3.0f) * 0.13f;
        Color[4] = mainTexture.Sample(samplerState[5], pin.texcoord + dir * 4.0f) * 0.11f;
        Color[5] = mainTexture.Sample(samplerState[5], pin.texcoord + dir * 5.0f) * 0.09f;
        Color[6] = mainTexture.Sample(samplerState[5], pin.texcoord + dir * 6.0f) * 0.07f;
        Color[7] = mainTexture.Sample(samplerState[5], pin.texcoord + dir * 7.0f) * 0.05f;
        Color[8] = mainTexture.Sample(samplerState[5], pin.texcoord + dir * 8.0f) * 0.03f;
        Color[9] = mainTexture.Sample(samplerState[5], pin.texcoord + dir * 9.0f) * 0.01f;

        //�ŏI�̐F��ۑ�����
        radialcolor = Color[0] + Color[1] + Color[2] + Color[3] + Color[4] + Color[5] + Color[6] + Color[7] + Color[8] + Color[9];
    
        radialcolor.a = 1;
        return radialcolor;
    }
    else
    {
        return mainTexture.Sample(samplerState[0], pin.texcoord);

    }
}