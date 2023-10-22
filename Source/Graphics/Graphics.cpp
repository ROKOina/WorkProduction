#include "Misc.h"
#include "Graphics/Shaders/3D/LambertShader.h"
#include "Graphics/Shaders/3D/PhongShader.h"
#include "Graphics/Shaders/3D/UnityChanToonShader.h"
#include "Graphics/Graphics.h"

Graphics* Graphics::instance_ = nullptr;

// コンストラクタ
Graphics::Graphics(HWND hWnd)
{
	// インスタンス設定
	_ASSERT_EXPR(instance_ == nullptr, "already instantiated");
	instance_ = this;

	// 画面のサイズを取得する。
	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT screenWidth = rc.right - rc.left;
	UINT screenHeight = rc.bottom - rc.top;

	this->screenWidth_ = static_cast<float>(screenWidth);
	this->screenHeight_ = static_cast<float>(screenHeight);

	HRESULT hr = S_OK;

	// デバイス＆スワップチェーンの生成
	{
		UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
		};

		// スワップチェーンを作成するための設定オプション
		DXGI_SWAP_CHAIN_DESC swapchainDesc;
		{
			swapchainDesc.BufferDesc.Width = screenWidth;
			swapchainDesc.BufferDesc.Height = screenHeight;
			swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
			swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
			swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 1ピクセルあたりの各色(RGBA)を8bit(0〜255)のテクスチャ(バックバッファ)を作成する。
			swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			swapchainDesc.SampleDesc.Count = 1;
			swapchainDesc.SampleDesc.Quality = 0;
			swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchainDesc.BufferCount = 1;		// バックバッファの数
			swapchainDesc.OutputWindow = hWnd;	// DirectXで描いた画を表示するウインドウ
			swapchainDesc.Windowed = TRUE;		// ウインドウモードか、フルスクリーンにするか。
			swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapchainDesc.Flags = 0; // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		}

		D3D_FEATURE_LEVEL featureLevel;

		// デバイス＆スワップチェーンの生成
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,						// どのビデオアダプタを使用するか？既定ならばnullptrで、IDXGIAdapterのアドレスを渡す。
			D3D_DRIVER_TYPE_HARDWARE,		// ドライバのタイプを渡す。D3D_DRIVER_TYPE_HARDWARE 以外は基本的にソフトウェア実装で、特別なことをする場合に用いる。
			nullptr,						// 上記をD3D_DRIVER_TYPE_SOFTWAREに設定した際に、その処理を行うDLLのハンドルを渡す。それ以外を指定している際には必ずnullptrを渡す。
			createDeviceFlags,				// 何らかのフラグを指定する。詳しくはD3D11_CREATE_DEVICE列挙型で検索。
			featureLevels,					// D3D_FEATURE_LEVEL列挙型の配列を与える。nullptrにすることでも上記featureと同等の内容の配列が使用される。
			ARRAYSIZE(featureLevels),		// featureLevels配列の要素数を渡す。
			D3D11_SDK_VERSION,				// SDKのバージョン。必ずこの値。
			&swapchainDesc,					// ここで設定した構造体に設定されているパラメータでSwapChainが作成される。
			swapchain_.GetAddressOf(),		// 作成が成功した場合に、SwapChainのアドレスを格納するポインタ変数へのアドレス。ここで指定したポインタ変数経由でSwapChainを操作する。
			device_.GetAddressOf(),			// 作成が成功した場合に、Deviceのアドレスを格納するポインタ変数へのアドレス。ここで指定したポインタ変数経由でDeviceを操作する。
			&featureLevel,					// 作成に成功したD3D_FEATURE_LEVELを格納するためのD3D_FEATURE_LEVEL列挙型変数のアドレスを設定する。
			immediateContext_.GetAddressOf()	// 作成が成功した場合に、Contextのアドレスを格納するポインタ変数へのアドレス。ここで指定したポインタ変数経由でContextを操作する。
			);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// レンダーターゲットビューの生成
	{
		// スワップチェーンからバックバッファテクスチャを取得する。
		// ※スワップチェーンに内包されているバックバッファテクスチャは'色'を書き込むテクスチャ。
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		hr = swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		// バックバッファテクスチャへの書き込みの窓口となるレンダーターゲットビューを生成する。
		hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// 深度ステンシルビューの生成
	{
		// 深度ステンシル情報を書き込むためのテクスチャを作成する。
		D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
		depthStencilBufferDesc.Width = screenWidth;
		depthStencilBufferDesc.Height = screenHeight;
		depthStencilBufferDesc.MipLevels = 1;
		depthStencilBufferDesc.ArraySize = 1;
		depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 1ピクセルあたり、深度情報を24Bit / ステンシル情報を8bitのテクスチャを作成する。
		depthStencilBufferDesc.SampleDesc.Count = 1;
		depthStencilBufferDesc.SampleDesc.Quality = 0;
		depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;		// 深度ステンシル用のテクスチャを作成する。
		depthStencilBufferDesc.CPUAccessFlags = 0;
		depthStencilBufferDesc.MiscFlags = 0;
		hr = device_->CreateTexture2D(&depthStencilBufferDesc, nullptr, depthStencilBuffer_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		// 深度ステンシルテクスチャへの書き込みに窓口になる深度ステンシルビューを作成する。
		hr = device_->CreateDepthStencilView(depthStencilBuffer_.Get(), nullptr, depthStencilView_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// ビューポートの設定
	{
		// 画面のどの領域にDirectXで描いた画を表示するかの設定。
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(screenWidth);
		viewport.Height = static_cast<float>(screenHeight);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		immediateContext_->RSSetViewports(1, &viewport);
	}

	// シェーダー
	{
		shader_.resize(SHADER_ID::MAX);

		shader_[SHADER_ID::Default] = std::make_unique<LambertShader>(device_.Get());
		shader_[SHADER_ID::Phong] = std::make_unique<PhongShader>(device_.Get());
		shader_[SHADER_ID::UnityChanToon] = std::make_unique<UnityChanToonShader>(device_.Get());

		//パラメーター設定
		{
			//影
			ShadowMapData* sp = &shaderParameter3D_.shadowMapData;
			//	シャドウマップのサイズ
			static	const	UINT	SHADOWMAP_SIZE = 2048;
			sp->height = SHADOWMAP_SIZE;
			sp->width = SHADOWMAP_SIZE;
			sp->shadowRect = 500;

			D3D11_TEXTURE2D_DESC texture2dDesc = {};							
			texture2dDesc.Width = SHADOWMAP_SIZE;
			texture2dDesc.Height = SHADOWMAP_SIZE;
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
			HRESULT hr = device_->CreateTexture2D(&texture2dDesc, 0, texture2d.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

			D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
			ZeroMemory(&shader_resource_view_desc, sizeof(shader_resource_view_desc));
			shader_resource_view_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
			shader_resource_view_desc.Texture2D.MipLevels = 1;
			hr = device_->CreateShaderResourceView(texture2d.Get(), &shader_resource_view_desc, sp->shadowSrvMap.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

			D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
			ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));
			depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_stencil_view_desc.Flags = 0;
			depth_stencil_view_desc.Texture2D.MipSlice = 0;
			hr = device_->CreateDepthStencilView(texture2d.Get(), &depth_stencil_view_desc, sp->shadowDsvMap.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		}

		//ポストエフェクト用
		postEffectModelRenderTarget = std::make_unique<PostRenderTarget>(device_.Get(), static_cast<UINT>(screenWidth_), static_cast<UINT>(screenHeight_), DXGI_FORMAT_R16G16B16A16_FLOAT);
		postEffectModelDepthStencil = std::make_unique<PostDepthStencil>(device_.Get(), static_cast<UINT>(screenWidth_), static_cast<UINT>(screenHeight_));
	}

	// レンダラ
	{
		debugRenderer_ = std::make_unique<DebugRenderer>(device_.Get());
		lineRenderer_ = std::make_unique<LineRenderer>(device_.Get(), 1024);
	}

	//描画周り一括初期化
	{
		dx11State_ = std::make_unique<Dx11StateLib>();
		dx11State_->Dx11StateInit(device_.Get());
	}

	//スレッドプール
	{
		threadPool_ = std::make_unique<ThreadPool>(8);
	}
}

// デストラクタ
Graphics::~Graphics()
{
}

// 描画ターゲットの退避
void Graphics::CacheRenderTargets()
{
	immediateContext_->RSGetViewports(&cachedViewportCount_, cachedViewports_);
	immediateContext_->OMGetRenderTargets(1, cachedRenderTargetView_.ReleaseAndGetAddressOf(), cachedDepthStencilView_.ReleaseAndGetAddressOf());
}

// 描画ターゲットを戻す
void Graphics::RestoreRenderTargets()
{
	immediateContext_->RSSetViewports(cachedViewportCount_, cachedViewports_);
	immediateContext_->OMSetRenderTargets(1, cachedRenderTargetView_.GetAddressOf(), cachedDepthStencilView_.Get());
}


//void Graphics::CreateSubWindowSwapChain(HWND hWnd_,int width, int height)
//{
//	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> r;
//	Microsoft::WRL::ComPtr<IDXGISwapChain> s;
//
//		// スワップチェーンを作成するための設定オプション
//		DXGI_SWAP_CHAIN_DESC swapchainDesc;
//		{
//			swapchainDesc.BufferDesc.Width = 0;
//			swapchainDesc.BufferDesc.Height = 0;
//			swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
//			swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
//			swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 1ピクセルあたりの各色(RGBA)を8bit(0〜255)のテクスチャ(バックバッファ)を作成する。
//			swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
//			swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
//
//			swapchainDesc.SampleDesc.Count = 1;
//			swapchainDesc.SampleDesc.Quality = 0;
//			swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//			swapchainDesc.BufferCount = 1;		// バックバッファの数
//			swapchainDesc.OutputWindow = hWnd_;	// DirectXで描いた画を表示するウインドウ
//			swapchainDesc.Windowed = TRUE;		// ウインドウモードか、フルスクリーンにするか。
//			swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//			swapchainDesc.Flags = 0; // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
//		}
//		
//		Microsoft::WRL::ComPtr<IDXGIDevice1> pDXGI = NULL;
//		Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter = NULL;
//		Microsoft::WRL::ComPtr<IDXGIFactory> pFactory = NULL;
//
//		device_->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pDXGI);
//		pDXGI->GetAdapter(&pAdapter);
//		pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pFactory);
//
//		pFactory->CreateSwapChain(device_.Get(), &swapchainDesc, s.GetAddressOf());
//
//		// レンダーターゲットビューの生成
//		{
//			// スワップチェーンからバックバッファテクスチャを取得する。
//			// ※スワップチェーンに内包されているバックバッファテクスチャは'色'を書き込むテクスチャ。
//			Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
//			HRESULT hr = s->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
//			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//
//			// バックバッファテクスチャへの書き込みの窓口となるレンダーターゲットビューを生成する。
//			hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, r.GetAddressOf());
//			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//		}
//
//		subWswapchain.emplace_back(s);
//		subWrenderTargetView.emplace_back(r);
//
//		//// レンダラ
//		//{
//		//	std::unique_ptr<ImGuiRenderer> imGuiR = std::make_unique<ImGuiRenderer>(hWnd_, device_.Get());
//		//	subWImguiRenderer.emplace_back(std::move(imGuiR));
//		//}
//
//}