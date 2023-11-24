#include "Misc.h"
#include "Dx11StateLib.h"
#include <cstdio>
#include <memory>

#include <filesystem>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <stb_image.h>
#include "Logger.h"

HRESULT Dx11StateLib::createVsFromCso(ID3D11Device* device, const char* cso_name, ID3D11VertexShader** vertex_shader,
	ID3D11InputLayout** inputLayout_, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	hr = device->CreateVertexShader(cso_data.get(), cso_sz, nullptr, vertex_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	if (inputLayout_)
	{
		hr = device->CreateInputLayout(input_element_desc, num_elements,
			cso_data.get(), cso_sz, inputLayout_);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	return hr;
}

HRESULT Dx11StateLib::createPsFromCso(ID3D11Device* device, const char* cso_name, ID3D11PixelShader** pixel_shader)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	hr = device->CreatePixelShader(cso_data.get(), cso_sz, nullptr, pixel_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	return hr;
}

HRESULT Dx11StateLib::createGsFromCso(ID3D11Device* device, const char* cso_name, ID3D11GeometryShader** geometry_shader)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	hr = device->CreateGeometryShader(cso_data.get(), cso_sz, nullptr, geometry_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	return hr;
}

HRESULT Dx11StateLib::createCsFromCso(ID3D11Device* device, const char* cso_name, ID3D11ComputeShader** compute_shader)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	hr = device->CreateComputeShader(cso_data.get(), cso_sz, nullptr, compute_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	return hr;
}

HRESULT Dx11StateLib::load_texture_from_file(ID3D11Device* device, const char* filename,
	ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc)
{
	HRESULT hr{ S_OK };
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	//ダミーテクスチャを作るか
	bool dummy = false;

	if (filename != nullptr)
	{
		std::filesystem::path dds_filename(filename);
		dds_filename.replace_extension("dds");
		if (std::filesystem::exists(dds_filename.c_str()))
		{
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediate_context;
			device->GetImmediateContext(immediate_context.GetAddressOf());
			hr = DirectX::CreateDDSTextureFromFile(device, immediate_context.Get(), dds_filename.c_str(), resource.GetAddressOf(), shader_resource_view);
		}
		else
		{
			wchar_t wfilename[256];
			::MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, 256);

			hr = DirectX::CreateWICTextureFromFile(device, wfilename, resource.GetAddressOf(), shader_resource_view);

		}
	}
	else
	{
		dummy = true;
	}

	//読み込み失敗時
	if (FAILED(hr))
	{
		// WICでサポートされていないフォーマットの場合（TGAなど）は
		// STBで画像読み込みをしてテクスチャを生成する
		int width, height, bpp;
		unsigned char* pixels = stbi_load(filename, &width, &height, &bpp, STBI_rgb_alpha);
		if (pixels != nullptr)
		{
			D3D11_TEXTURE2D_DESC desc = { 0 };
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
			D3D11_SUBRESOURCE_DATA data;
			::memset(&data, 0, sizeof(data));
			data.pSysMem = pixels;
			data.SysMemPitch = width * 4;

			Microsoft::WRL::ComPtr<ID3D11Texture2D>	texture;
			HRESULT hr = device->CreateTexture2D(&desc, &data, texture.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

			hr = device->CreateShaderResourceView(texture.Get(), nullptr, shader_resource_view);
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

			// 後始末
			stbi_image_free(pixels);
		}
		else
		{
			dummy = true;
		}
	}

	// 読み込み失敗したらダミーテクスチャを作る
	if (dummy)
	{
		LOG("load failed : %s\n", filename);

		const int width = 8;
		const int height = 8;
		UINT pixels[width * height];
		::memset(pixels, 0xFF, sizeof(pixels));

		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA data;
		::memset(&data, 0, sizeof(data));
		data.pSysMem = pixels;
		data.SysMemPitch = width;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>	texture;
		HRESULT hr = device->CreateTexture2D(&desc, &data, texture.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		hr = device->CreateShaderResourceView(texture.Get(), nullptr, shader_resource_view);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
	if (resource)
	{
		hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		texture2d->GetDesc(texture2d_desc);
	}

	return hr;
}

HRESULT Dx11StateLib::createConstantBuffer(ID3D11Device* device, UINT ByteWidth, ID3D11Buffer** buffer)
{
	D3D11_BUFFER_DESC desc;
	::memset(&desc, 0, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.ByteWidth = ByteWidth;
	desc.StructureByteStride = 0;

	HRESULT hr{ S_OK };
	hr = device->CreateBuffer(&desc, 0, buffer);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	return hr;
}


void Dx11StateLib::Dx11StateInit(ID3D11Device* device)
{
#pragma region ブレンドステートの設定
	{
		D3D11_BLEND_DESC blendDesc;
		::memset(&blendDesc, 0, sizeof(blendDesc));
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0].BlendEnable = TRUE;

		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		{	//透過
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

			HRESULT hr = device->CreateBlendState(&blendDesc, blendState_[static_cast<int>(BLEND_STATE_TYPE::ALPHA)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{	//透過_AlphaToCoverageEnable
			blendDesc.AlphaToCoverageEnable = TRUE;

			HRESULT hr = device->CreateBlendState(&blendDesc, blendState_[static_cast<int>(BLEND_STATE_TYPE::ALPHA_ATC)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{	//加算
			blendDesc.AlphaToCoverageEnable = FALSE;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

			HRESULT hr = device->CreateBlendState(&blendDesc, blendState_[static_cast<int>(BLEND_STATE_TYPE::ADDITION)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{	//加算（透過）
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

			HRESULT hr = device->CreateBlendState(&blendDesc, blendState_[static_cast<int>(BLEND_STATE_TYPE::ADDITION_ALPHA)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{	//減算
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_COLOR;

			HRESULT hr = device->CreateBlendState(&blendDesc, blendState_[static_cast<int>(BLEND_STATE_TYPE::SUBTRACTION)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{	//スクリーン
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

			HRESULT hr = device->CreateBlendState(&blendDesc, blendState_[static_cast<int>(BLEND_STATE_TYPE::SCREEN)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{	//パーティクル
			blendDesc.AlphaToCoverageEnable = FALSE;
			blendDesc.IndependentBlendEnable = FALSE;
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			HRESULT hr = device->CreateBlendState(&blendDesc, blendState_[static_cast<int>(BLEND_STATE_TYPE::PARTICLE)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{	//無し
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;

			HRESULT hr = device->CreateBlendState(&blendDesc, blendState_[static_cast<int>(BLEND_STATE_TYPE::DEFALT)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
	}
#pragma endregion


#pragma region 深度ステンシルステートの設定
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		{ //無し
			HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState_[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::DEFALT)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //ON_3D
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState_[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::DEPTH_ON_3D)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //ON_2D
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
			HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState_[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::DEPTH_ON_2D)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //DEPTH_ON_2D_EQUAL
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
			HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState_[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::DEPTH_ON_2D_EQUAL)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //ON_PARTICLE
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState_[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::PARTICLE)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //SKYMAP
			depthStencilDesc.DepthEnable = FALSE;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState_[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::SKYMAP)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //OFF
			depthStencilDesc.DepthEnable = FALSE;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
			HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState_[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::DEPTH_OFF)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{//シルエット
			depthStencilDesc.DepthEnable = TRUE;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
			HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState_[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::SILHOTTE)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
	}
#pragma endregion


#pragma region ラスタライザーステートの設定
	{
		D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());

		{ //無し
			HRESULT hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState_[static_cast<int>(RASTERIZER_TYPE::DEFALT)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //FRONTCOUNTER_FALSE_CULLBACK
			rasterizerDesc.FrontCounterClockwise = FALSE;
			rasterizerDesc.CullMode = D3D11_CULL_BACK;		
			rasterizerDesc.MultisampleEnable = TRUE;
			HRESULT hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState_[static_cast<int>(RASTERIZER_TYPE::FRONTCOUNTER_FALSE_CULLBACK)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //FRONTCOUNTER_FALSE_CULLNONE
			rasterizerDesc.FrontCounterClockwise = FALSE;
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			rasterizerDesc.MultisampleEnable = TRUE;
			HRESULT hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState_[static_cast<int>(RASTERIZER_TYPE::FRONTCOUNTER_FALSE_CULLNONE)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //FRONTCOUNTER_TRUE_CULLNONE
			rasterizerDesc.FrontCounterClockwise = TRUE;
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			rasterizerDesc.MultisampleEnable = TRUE;
			HRESULT hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState_[static_cast<int>(RASTERIZER_TYPE::FRONTCOUNTER_TRUE_CULLNONE)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //FRONTCOUNTER_FALSE_CULLNONE_WIREFRAME
			rasterizerDesc.FrontCounterClockwise = FALSE;
			rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			rasterizerDesc.MultisampleEnable = FALSE;
			HRESULT hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState_[static_cast<int>(RASTERIZER_TYPE::FRONTCOUNTER_FALSE_CULLNONE_WIREFRAME)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //SKYMAP
			rasterizerDesc.FillMode = D3D11_FILL_SOLID;
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			rasterizerDesc.FrontCounterClockwise = TRUE;
			rasterizerDesc.DepthBias = 0;
			rasterizerDesc.DepthBiasClamp = 0;
			rasterizerDesc.SlopeScaledDepthBias = 0;
			rasterizerDesc.DepthClipEnable = TRUE;
			rasterizerDesc.ScissorEnable = FALSE;
			rasterizerDesc.MultisampleEnable = FALSE;
			rasterizerDesc.AntialiasedLineEnable = TRUE;

			HRESULT hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState_[static_cast<int>(RASTERIZER_TYPE::SKYMAP)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //PARTICLE
			rasterizerDesc.FillMode = D3D11_FILL_SOLID;
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			rasterizerDesc.FrontCounterClockwise = TRUE;
			rasterizerDesc.DepthBias = 0;
			rasterizerDesc.DepthBiasClamp = 0;
			rasterizerDesc.SlopeScaledDepthBias = 0;
			rasterizerDesc.DepthClipEnable = TRUE;
			rasterizerDesc.ScissorEnable = FALSE;
			rasterizerDesc.MultisampleEnable = TRUE;
			rasterizerDesc.AntialiasedLineEnable = FALSE;

			HRESULT hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState_[static_cast<int>(RASTERIZER_TYPE::PARTICLE)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
	}
#pragma endregion


#pragma region サンプラーステートの設定
	{
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MaxAnisotropy = 16;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		{ //無し
			HRESULT hr = device->CreateSamplerState(&samplerDesc, samplerState_[static_cast<int>(SAMPLER_TYPE::DEFALT)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //TEXTURE_ADDRESS_WRAP
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			HRESULT hr = device->CreateSamplerState(&samplerDesc, samplerState_[static_cast<int>(SAMPLER_TYPE::TEXTURE_ADDRESS_WRAP)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //TEXTURE_ADDRESS_WRAP_POINT
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			HRESULT hr = device->CreateSamplerState(&samplerDesc, samplerState_[static_cast<int>(SAMPLER_TYPE::TEXTURE_ADDRESS_WRAP_POINT)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //TEXTURE_ADDRESS_BORDER_LINER
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			HRESULT hr = device->CreateSamplerState(&samplerDesc, samplerState_[static_cast<int>(SAMPLER_TYPE::TEXTURE_ADDRESS_BORDER_LINER)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //TEXTURE_ADDRESS_CLAMP
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			HRESULT hr = device->CreateSamplerState(&samplerDesc, samplerState_[static_cast<int>(SAMPLER_TYPE::TEXTURE_ADDRESS_CLAMP)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //TEXTURE_ADDRESS_WRAP_ANISO
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			HRESULT hr = device->CreateSamplerState(&samplerDesc, samplerState_[static_cast<int>(SAMPLER_TYPE::TEXTURE_ADDRESS_WRAP_ANISO)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //TEXTURE_ADDRESS_BORDER_POINT
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			samplerDesc.BorderColor[0] = FLT_MAX;
			samplerDesc.BorderColor[1] = FLT_MAX;
			samplerDesc.BorderColor[2] = FLT_MAX;
			samplerDesc.BorderColor[3] = FLT_MAX;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			HRESULT hr = device->CreateSamplerState(&samplerDesc, samplerState_[static_cast<int>(SAMPLER_TYPE::TEXTURE_ADDRESS_BORDER_POINT)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //TEXTURE_ADDRESS_BORDER_LINER
			samplerDesc.BorderColor[0] = 0;
			samplerDesc.BorderColor[1] = 0;
			samplerDesc.BorderColor[2] = 0;
			samplerDesc.BorderColor[3] = 0;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.MaxAnisotropy = 16;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			HRESULT hr = device->CreateSamplerState(&samplerDesc, samplerState_[static_cast<int>(SAMPLER_TYPE::TEXTURE_TRAIL)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
	}
#pragma endregion

}
