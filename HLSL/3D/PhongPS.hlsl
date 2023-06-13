#include "Phong.hlsli"

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);	// 法線マップ
Texture2D shadowMap : register(t2);	// シャドウマップ

SamplerState diffuseMapSamplerState : register(s0);
SamplerState shadowMapSamplerState : register(s1);

//背面明るい
float3 ClacHalfLambert(float3 normal, float3 lightVector, float3 lightColor, float3 kd)
{
    float d = clamp(
		(dot(-lightVector, normal) * 0.5f + 0.5f), 0, 1)
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
		*2-1;

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
    float3 L = normalize(lightDirection.xyz);
	float3 E = normalize(viewPosition.xyz - pin.world_position.xyz);

	// マテリアル定数
	float3 ka = float3(1, 1, 1);
	float3 kd = float3(1, 1, 1);
	float3 ks = float3(1, 1, 1);
	float shiness = 128;

	// 環境光の計算
	float3 ambient = ka * ambientLightColor.rgb;

    float3 directionalDiffuse = //背面、明るい
		CalcLambertDiffuse(N, L, float3(1, 1, 1), kd);

    float4 color = diffuseColor;
    color.rgb *= ambient + directionalDiffuse;

    return color;
	
	//// 平行光源のライティング計算
	////float3 directionalDiffuse = 
	////	CalcLambertDiffuse(N, L, directionalLightData.color.rgb, kd);
	//float3 directionalDiffuse =		//背面、明るい
	//	ClacHalfLambert(N, L, float4(1,1,1,1), kd);

	//float3 directionalSpecular = 
	//	CalcPhongSpecular(N, L, directionalLightData.color.rgb, E, shiness, ks);

	//// 平行光源の影なので、平行光源に対して影を適応
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



	////** 点光源の処理 **//
	//float3 pointDiffuse = (float3)0;
	//float3 pointSpecular = (float3)0;
	//int i;
	//for (i = 0; i < pointLightCount; ++i)
	//{
	//	// ライトベクトルを算出
	//	float3 lightVector = pin.world_position.xyz 
	//		- pointLightData[i].position;

	//		// ライトベクトルの長さを算出
	//		float lightLength = length(lightVector);

	//	// ライトの影響範囲外なら後の計算をしない。
	//		if (lightLength > pointLightData[i].range)
	//			continue;

	//	// 距離減衰を算出する
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

	////** スポットライトの処理 **//
	//float3 spotDiffuse = (float3)0;
	//float3 spotSpecular = (float3)0;
	//for (i = 0; i < spotLightCount; ++i)
	//{
	//	// ライトベクトルを算出
	//	float3 lightVector = pin.world_position.xyz
	//		- spotLightData[i].position.xyz;

	//		// ライトベクトルの長さを算出
	//		float lightLength = length(lightVector);

	//	if (lightLength > spotLightData[i].range)
	//		continue;

	//	// 距離減衰を算出する
	//	float attenuate = clamp(1.0f
	//		- lightLength / spotLightData[i].range,
	//		0.0f, 1.0f);

	//		lightVector = normalize(lightVector);

	//	// 角度減衰を算出してattenuateに乗算する
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

	////	リムライティング
	//color.rgb += CalcRimLight(N, E, L, directionalLightData.color.rgb);
	//return color;

	//return color;
}
