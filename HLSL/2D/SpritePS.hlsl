
#include "sprite.hlsli"

Texture2D texture0 : register(t0);
Texture2D dissolve : register(t1);
SamplerState sampler0 : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = texture0.Sample(sampler0, pin.texcoord) * pin.color;
    if (color.a < 0.00001)
        discard;    //�A���t�@�l���Ȃ��Ȃ珑�����܂Ȃ�
    
    //�f�B�]���u����
    if (dissolveConstant.isDissolve > 0.1f)
    {
        //�}�X�N�摜����ԐF���擾
        float mask = dissolve.Sample(sampler0, pin.texcoord).r;
    
        //step�֐���p����mask�l��dissolveThreshold�̒l���r���ē��ߒl��0 or 1�ɂ���
        float alpha = step(mask, dissolveConstant.dissolveThreshold);
        
        float edgeValue = step(mask - dissolveConstant.dissolveThreshold, dissolveConstant.dissolveThreshold)
        * step(dissolveConstant.dissolveThreshold, mask)
        * step(mask, dissolveConstant.dissolveThreshold + dissolveConstant.edgeThreshold);

        color.rbg += dissolveConstant.edgeColor.rbg * edgeValue;

        if (edgeValue == 1)
            color.rbg = dissolveConstant.edgeColor.rbg;
            
        alpha = saturate(alpha + edgeValue);
    
        color.a *= alpha;
    }
    
    return color;
}

