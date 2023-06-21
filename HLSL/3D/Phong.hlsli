struct VS_OUT
{
	float4 position : SV_POSITION;
	float3 world_position : POSITION;
	float3 normal	: NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 texcoord : TEXCOORD;
	float4 color    : COLOR;
};

cbuffer CbScene : register(b0)
{
	float4					viewPosition;
	row_major float4x4		viewProjection;
    float4 ambientLightColor;
    float4 lightDirection;
    float4 lightColor;
};

#define MAX_BONES 128
cbuffer CbMesh : register(b1)
{
	row_major float4x4	boneTransforms[MAX_BONES];
};

cbuffer CbSubset : register(b2)
{
	float4				materialColor;
};

cbuffer CbShadowmap : register(b3)
{
	row_major float4x4	lightViewProjection;	// ���C�g�r���[�v���W�F�N�V�����s��
	float3				shadowColor;			// �e�̐F
	float				shadowBias;				// �[�x�l��r���̃I�t�Z�b�g
};
