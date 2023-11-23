#include "ShadowMapCasterFunctions.hlsli"

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 world_position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
    float3 shadowTexcoord : TEXCOORD1; //	�V���h�E�}�b�v��������擾���邽�߂�UV���W�{�[�x�l
};

cbuffer CbScene : register(b0)
{
    float4 viewPosition;
    row_major float4x4 viewProjection;
    float4 ambientLightColor;
    float4 lightDirection;
    float4 lightColor;
};

#define MAX_BONES 128
cbuffer CbMesh : register(b1)
{
    row_major float4x4 boneTransforms[MAX_BONES];
};

cbuffer CbSubset : register(b2)
{
    float4 materialColor;

};

cbuffer CbShadowmap : register(b3)
{
    row_major float4x4 lightViewProjection; // ���C�g�r���[�v���W�F�N�V�����s��
    float3 shadowColor; // �e�̐F
    float shadowBias; // �[�x�l��r���̃I�t�Z�b�g
    
    //�e��`�悷�邩
    float shadowFall;
    
    float3 dummyShadow;
};

cbuffer UnityChanToon : register(b4)
{
    //���C�g�̐F�𔽉f���邩�i �x�[�X�Ɖe�����Ɏg���j
    float _Is_LightColor_Base;
    //�e�̃��x������
    float _Tweak_SystemShadowsLevel;
    //�e�̕\��臒l
    float _BaseColor_Step;
    //�e�̂ڂ���
    float _BaseShade_Feather;

    float4 _BaseColor;

    float4 _1st_ShadeColor;
    
    //�n�C�J���[�̐F
    float4 _Set_HighColorMask;

    //�n�C�J���[�̋����i �͈́j
    float _HighColor_Power;
    
    float dummy;
    
    //�������C�g����
    float _RimLight_Power;
    //�������C�gON,OFF
    float _RimLight;
    
    //�������C�g�J���[
    float4 _RimLightColor;
    
    //�G�~�b�V�u�J���[
    float4 _Emissive_Color;
}

cbuffer CbShape : register(b5)
{
    float shapeLerp0;
    float shapeLerp1;
    float shapeLerp2;
    float shapeLerp3;
    float shapeLerp4;
    float shapeLerp5;
    float shapeLerp6;
    float shapeLerp7;
    float shapeLerp8;
    float shapeLerp9;
    float shapeLerp10;
    float shapeLerp11;
    float shapeLerp12;
    float shapeLerp13;
    float shapeLerp14;
    float shapeLerp15;
}