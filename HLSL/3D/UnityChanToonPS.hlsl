#include "UnityChanToon.hlsli"

Texture2D _MainTex : register(t0);
Texture2D _NormalMap : register(t1);
Texture2D shadowMap : register(t2);

Texture2D _1st_ShadeMap : register(t5);
Texture2D EmissiveMap : register(t6);

SamplerState samplerState : register(s0);
SamplerState shadowState : register(s1);

inline float3 UnpackNormalDXT5nm(float4 packednormal)
{
    float3 normal;
    normal.xy = packednormal.wy * 2 - 1;
    normal.z = sqrt(1 - saturate(dot(normal.xy, normal.xy)));
    return normal;
}

inline float3 UnpackNormal(float4 packednormal)
{
    return UnpackNormalDXT5nm(packednormal);
}

//--------------------------------------------
//	sRGB�F��Ԃ�����`�F��Ԃւ̕ϊ�
//--------------------------------------------
// srgb		: sRGB�F��ԐF
// �Ԃ��l	: ���`�F��ԐF
inline float3 sRGBToLinear(float3 colorsRGB)
{
    return pow(colorsRGB.xyz, 2.2f);
}

float4 main(VertexOutput i) : SV_TARGET
{
    //�@���쐬
    //i.normalDir = normalize(i.normalDir);
    float3x3 tangentTransform = float3x3(i.tangent, i.binormal, i.normal);
    float3 viewDirection = normalize(viewPosition.xyz - i.world_position.xyz);
    float3 _NormalMap_var = UnpackNormal(_NormalMap.Sample(samplerState, i.texcoord));
    float3 normalDirection = normalize(mul(_NormalMap_var, tangentTransform));
    
    //�f�t���[�Y�쐬
    float4 _MainTex_var = _MainTex.Sample(samplerState, i.texcoord) * i.color;
    //_MainTex_var.rgb = sRGBToLinear(_MainTex_var.rgb);
    
    float3 halfDirection = normalize(viewDirection + lightDirection.xyz);
    
    //�J���[�o��
    float3 Set_LightColor = lightColor.rgb;
    float3 Set_BaseColor = lerp((_BaseColor.rgb * _MainTex_var.rgb), ((_BaseColor.rgb * _MainTex_var.rgb) * Set_LightColor), _Is_LightColor_Base);
    
    float4 _1st_ShadeMap_var = _1st_ShadeMap.Sample(samplerState, i.texcoord);
    float3 Set_1st_ShadeColor = (_1st_ShadeColor.rgb * _1st_ShadeMap_var.rgb);
    float _HalfLambert_var = 0.5 * dot(i.normal, lightDirection.xyz) + 0.5;
    
    //�e�̕\���͈� 
    //�������i�����Ŏ����j(UNITY_LIGHT_ATTENUATION�@�Ō���������v�Z���ۂ��̂���)https://zenn.dev/r_ngtm/articles/unity-shader-npr
    //UNITY_LIGHT_ATTENUATION(attenuation, i, i.posWorld.xyz);
    float attenuation = 0.5f;
    float _SystemShadowsLevel_var = (attenuation * 0.5) + 0.5 + _Tweak_SystemShadowsLevel > 0.001 ? (attenuation * 0.5) + 0.5 + _Tweak_SystemShadowsLevel : 0.0001;
    float Set_FinalShadowMask = saturate((1.0 + ((_HalfLambert_var * saturate(_SystemShadowsLevel_var) - (_BaseColor_Step - _BaseShade_Feather)))
                        / (_BaseColor_Step - (_BaseColor_Step - _BaseShade_Feather))));
    
    //�n�C�J���[�i���˂̐F�j
    float3 Set_FinalBaseColor = lerp(Set_BaseColor, Set_1st_ShadeColor, Set_FinalShadowMask); // Final Color
    float4 _Set_HighColorMask_var = _Set_HighColorMask; //�摜�ł͂Ȃ��F�Ŏw��
            
    //�X�y�L����(���˂̌�)
    float _Specular_var = 0.5 * dot(-halfDirection, normalDirection) + 0.5; //  Specular                
    float _TweakHighColorMask_var = (saturate(_Set_HighColorMask_var.g) * (1.0 - step(_Specular_var, (1.0 - pow(_HighColor_Power, 5)))));
    
    float3 _HighColor_var = (_Set_HighColorMask.rgb * Set_LightColor * _MainTex_var.rgb) * _TweakHighColorMask_var;
    
    if(shadowFall > 0.1f)
    {
        // ���s�����̉e�Ȃ̂ŁA���s�����ɑ΂��ĉe��K��
        float3 shadow = CalcShadowColor(shadowMap, shadowState, i.shadowTexcoord, shadowColor, shadowBias);
        Set_FinalBaseColor *= shadow;
        _HighColor_var *= shadow;
    }

    
    float3 Set_HighColor =
    (
    //saturate((Set_FinalBaseColor - _TweakHighColorMask_var))
    Set_FinalBaseColor
    + _HighColor_var);

    
    //�������C�g�i���E���ɂ��炩���F������j
    float3 _Is_LightColor_RimLight_var = _RimLightColor.rgb;
    float _RimArea_var = (1.0 - dot(i.normal, viewDirection));
    float _RimLightPower_var = pow(abs(_RimArea_var), exp2(lerp(3.0, 0.0, _RimLight_Power)));
    float _RimLight_InsideMask = 0.0001;
    float _Rimlight_InsideMask_var = saturate((0.0 + ((_RimLightPower_var - _RimLight_InsideMask) * (1.0 - 0.0)) / (1.0 - _RimLight_InsideMask)));
    float _VertHalfLambert_var = 0.5 * dot(i.normal, lightDirection.xyz) + 0.5;
    float3 _LightDirection_MaskOn_var = (_Is_LightColor_RimLight_var * _Rimlight_InsideMask_var);
    float3 Set_RimLight = _LightDirection_MaskOn_var;

    float3 _RimLight_var = lerp(Set_HighColor, (Set_HighColor + Set_RimLight), _RimLight);
    float3 finalColor = _RimLight_var;
   
    
    //������摜�ɕς����炻�̐F�Ɍ���
    float4 _Emissive_Tex_var = { 1, 1, 1, 1 };
    //�A���t�@�l�ŋ������߂�
    float3 emissive = _Emissive_Tex_var.rgb * _Emissive_Color.rgb * (_Emissive_Color.a * 5);
    finalColor = saturate(finalColor) + emissive;
    
    float4 finalRGBA = float4(finalColor, _MainTex_var.a);
    
    return finalRGBA;
}