#pragma once

#include <DirectXMath.h>
#include <wrl.h>
#include "Misc.h"


//ポストエフェクト描画情報
struct PostRenderTarget
{
public:
	PostRenderTarget(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM)
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
		hr = device->CreateShaderResourceView(texture2d.Get(), &shader_resource_view_desc, diffuseMap.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc{};
		render_target_view_desc.Format = texture2dDesc.Format;
		render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hr = device->CreateRenderTargetView(texture2d.Get(), &render_target_view_desc, renderTargetView.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	diffuseMap;
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
		hr = device->CreateShaderResourceView(texture2d.Get(), &shader_resource_view_desc, diffuseMap.ReleaseAndGetAddressOf());
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

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	diffuseMap;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		depthStencilView;
	UINT width;
	UINT height;
};



//	シャドウマップ用情報
struct ShadowMapData
{
	std::unique_ptr<PostDepthStencil> shadowmapDepthStencil;

	DirectX::XMFLOAT4X4			lightViewProjection;	//	ライトビュープロジェクション行列
	DirectX::XMFLOAT3			shadowColor;			//	影の色
	float						shadowBias;				//	深度比較用のオフセット値
	float						shadowRect;				//	シャドウマップに描画する範囲

	DirectX::XMFLOAT4			shadowCameraPos;		//　影のカメラポジション
};

//カラーグレーディング
struct ColorGradingData
{
	float brightness = 0.0f;							//輝度
	float contrast = 1.0f;								//コンストラスト
	float saturation = 1.5f;							//彩度
	float dummy;
	DirectX::XMFLOAT4 filter = { 1.0f,1.0f,1.0f,0 };	//フィルター
};

//ブルーム
static constexpr u_int KARNEL_SIZE = 17;
static constexpr float PI_ANGLE = 3.141592f;
static constexpr u_int BUFFER_SIZE = KARNEL_SIZE * KARNEL_SIZE;
struct BloomBlur
{
	DirectX::XMFLOAT4 Weight[BUFFER_SIZE]; //ガウシアンフィルタ X,Y オフセット、Z 重み、W 未使用
	float KarnelSize;
	DirectX::XMFLOAT2 texel;
	float dummy;
};

struct BloomLuminance
{
	float threshold; // 高輝度抽出のための閾値
	float intensity; // ブルームの強度
	DirectX::XMFLOAT2 dummy;
};

//スカイマップ
struct SkymapData
{
	DirectX::XMFLOAT4X4			invMat;
};

//太陽周り
struct SunAtmosphere
{
	DirectX::XMFLOAT4 mistColor = { 0.226f, 0.273f, 0.344f, 1.000f };
	DirectX::XMFLOAT2 mistDensity = { 0.020f, 0.020f }; // x:extinction, y:inscattering
	DirectX::XMFLOAT2 mist_heightFalloff = { 1000.000f, 1000.000f }; // x:extinction, y:inscattering
	DirectX::XMFLOAT2 heightMistOffset = { 244.300f, 335.505f }; // x:extinction, y:inscattering

	float mistCutoffDistance = -0.1f;

	float mistFlowSpeed = 118.123f;
	float mistFlowNoiseScaleFactor = 0.015f;
	float mistFlowDensityLowerLimit = 0.330f;

	float distanceToSun = 500.0f;
	float sunHighlightExponentialFactor = 38.000f; // Affects the area of influence of the sun's highlights.
	float sunHighlightIntensity = 1.200f;


	float enabled = -1;

	DirectX::XMFLOAT2 dummy;

	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 inverseProjection;
	DirectX::XMFLOAT4X4 inverseViewProjection;
};

//放射線ブラー
struct RadialBlur
{
	DirectX::XMFLOAT2 blurPos = { (1920 * 0.8f) * 0.5f,(1080 * 0.8f) * 0.5f };
	float power = 0;
	float enabled = -1;
};

//ビネット
struct Vignette
{
	float power = 0;	//0~1
	float enabled = -1;
	DirectX::XMFLOAT2 dummyV;
};

//ユニティちゃんトゥーン
struct UnityChanToonStruct
{
	//ライトの色を反映するか（ ベースと影両方に使う）
	float _Is_LightColor_Base = 1;
	//影のレベル調整
	float _Tweak_SystemShadowsLevel = 0;
	//影の表示閾値
	float _BaseColor_Step = 0.5f;
	//影のぼかし
	float _BaseShade_Feather = 0.001f;

	DirectX::XMFLOAT4 _BaseColor = { 1,1,1,1 };

	DirectX::XMFLOAT4 _1st_ShadeColor =
	{ 153.0f / 255.0f,197.0f / 255.0f,202.0f / 255.0f,1 };
	//DirectX::XMFLOAT4 _1st_ShadeColor = { 1,0.8f,0.7f,1 };

	//ハイカラーの色
	DirectX::XMFLOAT4 _Set_HighColorMask = { 1,1,1,1 };

	//ハイカラーの強さ（ 範囲）
	float _HighColor_Power = 0.3f;

	float dummy = 0;

	//リムライト強さ
	float _RimLight_Power = 0.25f;
	//リムライトON,OFF
	float _RimLight = 1;

	//リムライトカラー
	DirectX::XMFLOAT4 _RimLightColor = { 1,1,1,1 };
	//エミッシブカラー
	DirectX::XMFLOAT4 _Emissive_Color = { 0,0,0,1 };

	template<class Archive>
	void serialize(Archive& archive, int version);
};


// コンスタントバッファに使うパラメーター
struct ShaderParameter3D
{
	//視点
	DirectX::XMFLOAT4X4		view;
	DirectX::XMFLOAT4X4		projection;
	DirectX::XMFLOAT4		viewPosition;

	//ライト
	DirectX::XMFLOAT4		lightDirection = { 0.626f,-0.55f,-0.533f,0 };
	DirectX::XMFLOAT4		lightColor = { 1,1,1,1 };

	//影
	ShadowMapData shadowMapData;

	//カラーグレーディング
	ColorGradingData colorGradingData;

	//ブルーム
	BloomBlur bloomBlur;
	BloomLuminance bloomLuminance;

	//太陽周り
	SunAtmosphere sunAtmosphere;

	//放射線ブラー
	RadialBlur radialBlur;

	//ビネット
	Vignette vignette;
};

