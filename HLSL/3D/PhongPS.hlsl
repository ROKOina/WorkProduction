#include "Phong.hlsli"

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);	// 法線マップ
Texture2D shadowMap : register(t2);	// シャドウマップ

SamplerState diffuseMapSamplerState : register(s0);
SamplerState shadowMapSamplerState : register(s1);

//背面明るい
float3 ClacHalfLambert(float3 normal, float3 lightVector, float3 lightColor, float3 kd)
{
    float3 d = clamp(
		(dot(-lightVector, normal) * 0.5f + 0.5f), 0.0f, 1.0f)
		* lightColor * kd;

    return lightColor * d * kd;
}

float3 CalcLambertDiffuse(float3 normal, float3 lightVector, float3 lightColor, float3 kd)
{
	// 法線とライト方向へのベクトルでの内積 
	// 内積した結果、ライトから見て裏向きの面だとマイナスの結果が出てしまうため
	// それをそのまま乗算すると色がおかしくなります。
	// 0以下であれば0になるように制限をつけましょう。
    float d = max(dot(-lightVector, normal), 0);
		

	// 入射光色と内積の結果、及び反射率をすべて乗算して返却しましょう。
    return d * lightColor * kd;
}

float4 main(VS_OUT pin) : SV_TARGET
{
	float4 diffuseColor = diffuseMap.Sample(
		diffuseMapSamplerState, pin.texcoord) * pin.color;

	
	// 法線マップからxyz成分を取得して( -1 ～ +1 )の間にスケーリング
	float3 normal = normalMap.Sample(
		diffuseMapSamplerState, pin.texcoord).xyz
		* 2 - 1;

	// 変換用の3X3行列を用意する
	// 注意点として、頂点シェーダー内で算出した単位ベクトル類はラスタライズの際に数値が頂点間で補間されます。
	// そのため、ピクセルシェーダーのタイミングでは単位ベクトルではないものが入ってくる可能性があるので、
	// 正規化を忘れないようにしてください。
	float3x3 CM = {
		{pin.tangent},
		{pin.binormal},
		{pin.normal},
	};

	// 法線マップで取得した法線情報に変換行列を掛け合わせましょう。正規化は忘れずに
	float3 N = normalize(mul(normal ,CM));

	//N = normalize(pin.normal);
    float3 L = -normalize(lightDirection.xyz);
	float3 E = normalize(viewPosition.xyz - pin.world_position.xyz);

	// マテリアル定数
	float3 ka = float3(1, 1, 1);
	float3 kd = float3(1, 1, 1);
	float3 ks = float3(1, 1, 1);
	float shiness = 70;

	// 環境光の計算
	float3 ambient = ka * ambientLightColor.rgb;

    float3 directionalDiffuse = 
		CalcLambertDiffuse(N, L, lightColor.rgb, kd);

    float4 color = diffuseColor;	//アルファ値用
    color.rgb = diffuseColor.rgb * (ambient + directionalDiffuse) * materialColor.rgb;
    return float4(color.xyz, materialColor.a);
}
