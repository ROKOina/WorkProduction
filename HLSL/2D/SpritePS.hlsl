
#include "sprite.hlsli"

Texture2D texture0 : register(t0);
Texture2D dissolve : register(t1);
SamplerState sampler0 : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = texture0.Sample(sampler0, pin.texcoord) * pin.color;
    if (color.a < 0.00001)
        discard;    //アルファ値がないなら書き込まない
    
    //ディゾルブ処理
    if (dissolveConstant.isDissolve > 0.1f)
    {
        //マスク画像から赤色を取得
        float mask = dissolve.Sample(sampler0, pin.texcoord).r;
    
        //step関数を用いてmask値とdissolveThresholdの値を比較して透過値を0 or 1にする
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

