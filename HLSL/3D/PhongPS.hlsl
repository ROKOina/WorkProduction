#include "Phong.hlsli"

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);	// �@���}�b�v
Texture2D shadowMap : register(t2);	// �V���h�E�}�b�v

SamplerState diffuseMapSamplerState : register(s0);
SamplerState shadowMapSamplerState : register(s1);

//�w�ʖ��邢
float3 ClacHalfLambert(float3 normal, float3 lightVector, float3 lightColor, float3 kd)
{
    float d = clamp(
		(dot(-lightVector, normal) * 0.5f + 0.5f), 0, 1)
		* lightColor * kd;

    return lightColor * d * kd;
}

float3 CalcLambertDiffuse(float3 normal, float3 lightVector, float3 lightColor, float3 kd)
{
	// �@���ƃ��C�g�����ւ̃x�N�g���ł̓��� 
	// ���ς������ʁA���C�g���猩�ė������̖ʂ��ƃ}�C�i�X�̌��ʂ��o�Ă��܂�����
	// ��������̂܂܏�Z����ƐF�����������Ȃ�܂��B
	// 0�ȉ��ł����0�ɂȂ�悤�ɐ��������܂��傤�B
    float d = max(dot(-lightVector, normal), 0);
		

	// ���ˌ��F�Ɠ��ς̌��ʁA�y�є��˗������ׂď�Z���ĕԋp���܂��傤�B
    return d * lightColor * kd;
}

float4 main(VS_OUT pin) : SV_TARGET
{
	float4 diffuseColor = diffuseMap.Sample(
		diffuseMapSamplerState, pin.texcoord) * pin.color;

	
	// �@���}�b�v����xyz�������擾����( -1 �` +1 )�̊ԂɃX�P�[�����O
	float3 normal = normalMap.Sample(
		diffuseMapSamplerState, pin.texcoord).xyz
		* 2 - 1;

	// �ϊ��p��3X3�s���p�ӂ���
	// ���ӓ_�Ƃ��āA���_�V�F�[�_�[���ŎZ�o�����P�ʃx�N�g���ނ̓��X�^���C�Y�̍ۂɐ��l�����_�Ԃŕ�Ԃ���܂��B
	// ���̂��߁A�s�N�Z���V�F�[�_�[�̃^�C�~���O�ł͒P�ʃx�N�g���ł͂Ȃ����̂������Ă���\��������̂ŁA
	// ���K����Y��Ȃ��悤�ɂ��Ă��������B
	float3x3 CM = {
		{pin.tangent},
		{pin.binormal},
		{pin.normal},
	};

	// �@���}�b�v�Ŏ擾�����@�����ɕϊ��s����|�����킹�܂��傤�B���K���͖Y�ꂸ��
	float3 N = normalize(mul(normal ,CM));

	//N = normalize(pin.normal);
    float3 L = -normalize(lightDirection.xyz);
	float3 E = normalize(viewPosition.xyz - pin.world_position.xyz);

	// �}�e���A���萔
	float3 ka = float3(1, 1, 1);
	float3 kd = float3(1, 1, 1);
	float3 ks = float3(1, 1, 1);
	float shiness = 70;

	// �����̌v�Z
	float3 ambient = ka * ambientLightColor.rgb;

    float3 directionalDiffuse = 
		CalcLambertDiffuse(N, L, lightColor.rgb, kd);

    float4 color = diffuseColor;	//�A���t�@�l�p
    color.rgb = diffuseColor.rgb * (ambient + directionalDiffuse);
    return color;
}
