#include "Misc.h"
#include "Dx11StateLib.h"
#include <cstdio>
#include <memory>

HRESULT Dx11StateLib::createVsFromCso(ID3D11Device* device, const char* cso_name, ID3D11VertexShader** vertex_shader,
	ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements)
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

	if (input_layout)
	{
		hr = device->CreateInputLayout(input_element_desc, num_elements,
			cso_data.get(), cso_sz, input_layout);
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
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());

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
		{ //ON_PARTICLE
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState_[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::PARTICLE)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //OFF
			depthStencilDesc.DepthEnable = FALSE;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
			HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState_[static_cast<int>(DEPTHSTENCIL_STATE_TYPE::DEPTH_OFF)].GetAddressOf());
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
		{ //TEXTURE_ADDRESS_CLAMP
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			HRESULT hr = device->CreateSamplerState(&samplerDesc, samplerState_[static_cast<int>(SAMPLER_TYPE::TEXTURE_ADDRESS_CLAMP)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		{ //TEXTURE_ADDRESS_BORDER_POINT
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
	}
#pragma endregion

}
