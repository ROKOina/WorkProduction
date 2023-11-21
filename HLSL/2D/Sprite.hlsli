struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 color    : COLOR;
	float2 texcoord : TEXCOORD;
};


//���ʈꗗ
struct DissolveConstant
{
    float isDissolve;
    float dissolveThreshold; // �f�B�]���u��
    float edgeThreshold; // �ӂ���臒l
    
    float dummy;
    
    float4 edgeColor; // �ӂ��̐F
};

//���ʂ܂Ƃ�
cbuffer Effect2DConstant : register(b0)
{
    //�f�B�]���u
    DissolveConstant dissolveConstant;

};