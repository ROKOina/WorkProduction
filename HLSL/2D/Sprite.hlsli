struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 color    : COLOR;
	float2 texcoord : TEXCOORD;
};


//効果一覧
struct DissolveConstant
{
    float isDissolve;
    float dissolveThreshold; // ディゾルブ量
    float edgeThreshold; // ふちの閾値
    
    float dummy;
    
    float4 edgeColor; // ふちの色
};

//効果まとめ
cbuffer Effect2DConstant : register(b0)
{
    //ディゾルブ
    DissolveConstant dissolveConstant;

};