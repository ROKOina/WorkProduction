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
		*2-1;

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
    float3 L = normalize(lightDirection.xyz);
	float3 E = normalize(viewPosition.xyz - pin.world_position.xyz);

	// �}�e���A���萔
	float3 ka = float3(1, 1, 1);
	float3 kd = float3(1, 1, 1);
	float3 ks = float3(1, 1, 1);
	float shiness = 128;

	// �����̌v�Z
	float3 ambient = ka * ambientLightColor.rgb;

    float3 directionalDiffuse = //�w�ʁA���邢
		CalcLambertDiffuse(N, L, float3(1, 1, 1), kd);

    float4 color = diffuseColor;
    color.rgb *= ambient + directionalDiffuse;

    return color;
	
	//// ���s�����̃��C�e�B���O�v�Z
	////float3 directionalDiffuse = 
	////	CalcLambertDiffuse(N, L, directionalLightData.color.rgb, kd);
	//float3 directionalDiffuse =		//�w�ʁA���邢
	//	ClacHalfLambert(N, L, float4(1,1,1,1), kd);

	//float3 directionalSpecular = 
	//	CalcPhongSpecular(N, L, directionalLightData.color.rgb, E, shiness, ks);

	//// ���s�����̉e�Ȃ̂ŁA���s�����ɑ΂��ĉe��K��
	//float3 shadow = CalcShadowColor(shadowMap, shadowMapSamplerState, pin.shadowTexcoord, shadowColor, shadowBias);
	//directionalDiffuse *= shadow;
	//directionalSpecular *= shadow;

	////float4 color = float4(ambient, diffuseColor.a);
	////color.rgb += diffuseColor.rgb * directionalDiffuse;
	////color.rgb += directionalSpecular;

	//float4 color = diffuseColor;
	//color.rgb *= ambient + directionalDiffuse;
	//color.rgb += directionalSpecular;

	////return color;



	////** �_�����̏��� **//
	//float3 pointDiffuse = (float3)0;
	//float3 pointSpecular = (float3)0;
	//int i;
	//for (i = 0; i < pointLightCount; ++i)
	//{
	//	// ���C�g�x�N�g�����Z�o
	//	float3 lightVector = pin.world_position.xyz 
	//		- pointLightData[i].position;

	//		// ���C�g�x�N�g���̒������Z�o
	//		float lightLength = length(lightVector);

	//	// ���C�g�̉e���͈͊O�Ȃ��̌v�Z�����Ȃ��B
	//		if (lightLength > pointLightData[i].range)
	//			continue;

	//	// �����������Z�o����
	//		float attenuate = clamp(1.0f
	//			- lightLength / pointLightData[i].range,
	//			0.0f, 1.0f);

	//		lightVector = lightVector / lightLength;
	//	pointDiffuse += CalcLambertDiffuse(N, lightVector,
	//		pointLightData[i].color.rgb, kd.rgb) * attenuate;
	//	pointSpecular += CalcPhongSpecular(N, lightVector,
	//		pointLightData[i].color.rgb, E, shiness, ks.rgb) * attenuate;
	//}

	//color = float4(ambient, diffuseColor.a);
	//color.rgb += diffuseColor.rgb * (directionalDiffuse + pointDiffuse);
	//color.rgb += directionalSpecular + pointSpecular;

	////return color;

	////** �X�|�b�g���C�g�̏��� **//
	//float3 spotDiffuse = (float3)0;
	//float3 spotSpecular = (float3)0;
	//for (i = 0; i < spotLightCount; ++i)
	//{
	//	// ���C�g�x�N�g�����Z�o
	//	float3 lightVector = pin.world_position.xyz
	//		- spotLightData[i].position.xyz;

	//		// ���C�g�x�N�g���̒������Z�o
	//		float lightLength = length(lightVector);

	//	if (lightLength > spotLightData[i].range)
	//		continue;

	//	// �����������Z�o����
	//	float attenuate = clamp(1.0f
	//		- lightLength / spotLightData[i].range,
	//		0.0f, 1.0f);

	//		lightVector = normalize(lightVector);

	//	// �p�x�������Z�o����attenuate�ɏ�Z����
	//		float3 spotDirection = spotLightData[i].direction.xyz;
	//		float angle = dot(lightVector, spotDirection);
	//		float area = spotLightData[i].innerCorn
	//			- spotLightData[i].outerCorn;
	//		attenuate *= clamp(1.0f
	//			- (spotLightData[i].innerCorn - angle) / area
	//			, 0.0f, 1.0f);

	//		spotDiffuse += CalcLambertDiffuse(N, lightVector,
	//			spotLightData[i].color.rgb, kd.rgb) * attenuate;
	//	spotSpecular += CalcPhongSpecular(N, lightVector,
	//		spotLightData[i].color.rgb, E, shiness, ks.rgb) * attenuate;
	//}

	//color = float4(0,0,0, diffuseColor.a);
	//color.rgb += diffuseColor.rgb * ambient;
	//color.rgb += diffuseColor.rgb * (directionalDiffuse + pointDiffuse + spotDiffuse);
	//color.rgb += directionalSpecular + pointSpecular + spotSpecular;

	////	�������C�e�B���O
	//color.rgb += CalcRimLight(N, E, L, directionalLightData.color.rgb);
	//return color;

	//return color;
}
