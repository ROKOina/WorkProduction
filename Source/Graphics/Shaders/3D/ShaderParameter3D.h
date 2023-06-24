#pragma once

#include <DirectXMath.h>
#include <wrl.h>
#include "Misc.h"

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

//カラーグレーディング
struct ColorGradingData
{
	float brightness = 0.0f;							//輝度
	float contrast = 1.3f;								//コンストラスト
	float saturation = 0.5f;							//彩度
	float dummy;
	DirectX::XMFLOAT4 filter = { 0.9f,1.0f,1.1f,0 };	//フィルター
};

//ブルーム
static constexpr u_int KARNEL_SIZE = 17;
static constexpr float PI = 3.141592f;
static constexpr u_int BUFFER_SIZE = KARNEL_SIZE * KARNEL_SIZE;
struct BloomData
{
	DirectX::XMFLOAT4 Weight[BUFFER_SIZE]; //ガウシアンフィルタ X,Y オフセット、Z 重み、W 未使用
	float KarnelSize;
	DirectX::XMFLOAT2 texel;
	float dummy;
};

struct BloomData2
{
	float threshold; // 高輝度抽出のための閾値
	float intensity; // ブルームの強度
	DirectX::XMFLOAT2 dummy;
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
	ShadowMapData shadowMapData;

	//カラーグレーディング
	ColorGradingData colorGradingData;

	//ブルーム
	BloomData bloomData;
	BloomData2 bloomData2;
};

//ポストエフェクト描画情報
struct PostRenderTarget
{
public:
	PostRenderTarget(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format= DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		D3D11_TEXTURE2D_DESC texture2dDesc;

		this->width = width;
		this->height = height;

		texture2dDesc = {};
		texture2dDesc.Width = width;
		texture2dDesc.Height = height;
		texture2dDesc.MipLevels = 1;
		texture2dDesc.ArraySize = 1;
		texture2dDesc.Format = format;
		texture2dDesc.SampleDesc.Count = 1;
		texture2dDesc.SampleDesc.Quality = 0;
		texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
		texture2dDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texture2dDesc.CPUAccessFlags = 0;
		texture2dDesc.MiscFlags = 0;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>	texture2d;
		HRESULT hr = device->CreateTexture2D(&texture2dDesc, 0, texture2d.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
		shader_resource_view_desc.Format = texture2dDesc.Format;
		shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shader_resource_view_desc.Texture2D.MipLevels = 1;
		hr = device->CreateShaderResourceView(texture2d.Get(), &shader_resource_view_desc, shaderResourceView.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc{};
		render_target_view_desc.Format = texture2dDesc.Format;
		render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hr = device->CreateRenderTargetView(texture2d.Get(), &render_target_view_desc, renderTargetView.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		renderTargetView;
	UINT width;
	UINT height;
};
struct PostDepthStencil
{
public:
	PostDepthStencil(ID3D11Device* device, UINT width, UINT height)
	{
		D3D11_TEXTURE2D_DESC texture2dDesc;

		this->width = width;
		this->height = height;

		texture2dDesc = {};
		texture2dDesc.Width = width;
		texture2dDesc.Height = height;
		texture2dDesc.MipLevels = 1;
		texture2dDesc.ArraySize = 1;
		texture2dDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		texture2dDesc.SampleDesc.Count = 1;
		texture2dDesc.SampleDesc.Quality = 0;
		texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
		texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texture2dDesc.CPUAccessFlags = 0;
		texture2dDesc.MiscFlags = 0;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>	texture2d;
		HRESULT hr = device->CreateTexture2D(&texture2dDesc, 0, texture2d.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
		ZeroMemory(&shader_resource_view_desc, sizeof(shader_resource_view_desc));
		shader_resource_view_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
		shader_resource_view_desc.Texture2D.MipLevels = 1;
		hr = device->CreateShaderResourceView(texture2d.Get(), &shader_resource_view_desc, shaderResourceView.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
		ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));
		depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_stencil_view_desc.Flags = 0;
		depth_stencil_view_desc.Texture2D.MipSlice = 0;
		hr = device->CreateDepthStencilView(texture2d.Get(), &depth_stencil_view_desc, depthStencilView.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		depthStencilView;
	UINT width;
	UINT height;
};

