#pragma once

#include <DirectXMath.h>
#include <wrl.h>

//	シャドウマップ用情報
struct ShadowMapData
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSrvMap;				
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDsvMap;				
	UINT width;
	UINT height;
	DirectX::XMFLOAT4X4			lightViewProjection;	//	ライトビュープロジェクション行列
	DirectX::XMFLOAT3			shadowColor;			//	影の色
	float						shadowBias;				//	深度比較用のオフセット値
	float						shadowRect;				//	シャドウマップに描画する範囲
};

// コンスタントバッファに使うパラメーター
struct ShaderParameter3D
{
	//視点
	DirectX::XMFLOAT4X4		view;
	DirectX::XMFLOAT4X4		projection;
	DirectX::XMFLOAT4		viewPosition;

	//ライト
	DirectX::XMFLOAT4		lightDirection = { 0.3f,-0.7f,0,0 };
	DirectX::XMFLOAT4		lightColor = { 1,1,1,1 };

	//影
	struct ShadowMapData shadowMapData;
};


