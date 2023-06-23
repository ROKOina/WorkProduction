#include "Misc.h"
#include "Graphics/Shaders/3D/LambertShader.h"
#include "Graphics/Shaders/3D/PhongShader.h"
#include "Graphics/Graphics.h"

Graphics* Graphics::instance_ = nullptr;

// �R���X�g���N�^
Graphics::Graphics(HWND hWnd)
{
	// �C���X�^���X�ݒ�
	_ASSERT_EXPR(instance_ == nullptr, "already instantiated");
	instance_ = this;

	// ��ʂ̃T�C�Y���擾����B
	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT screenWidth = rc.right - rc.left;
	UINT screenHeight = rc.bottom - rc.top;

	this->screenWidth_ = static_cast<float>(screenWidth);
	this->screenHeight_ = static_cast<float>(screenHeight);

	HRESULT hr = S_OK;

	// �f�o�C�X���X���b�v�`�F�[���̐���
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

		// �X���b�v�`�F�[�����쐬���邽�߂̐ݒ�I�v�V����
		DXGI_SWAP_CHAIN_DESC swapchainDesc;
		{
			swapchainDesc.BufferDesc.Width = screenWidth;
			swapchainDesc.BufferDesc.Height = screenHeight;
			swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
			swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
			swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 1�s�N�Z��������̊e�F(RGBA)��8bit(0�`255)�̃e�N�X�`��(�o�b�N�o�b�t�@)���쐬����B
			swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			swapchainDesc.SampleDesc.Count = 1;
			swapchainDesc.SampleDesc.Quality = 0;
			swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchainDesc.BufferCount = 1;		// �o�b�N�o�b�t�@�̐�
			swapchainDesc.OutputWindow = hWnd;	// DirectX�ŕ`�������\������E�C���h�E
			swapchainDesc.Windowed = TRUE;		// �E�C���h�E���[�h���A�t���X�N���[���ɂ��邩�B
			swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapchainDesc.Flags = 0; // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		}

		D3D_FEATURE_LEVEL featureLevel;

		// �f�o�C�X���X���b�v�`�F�[���̐���
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,						// �ǂ̃r�f�I�A�_�v�^���g�p���邩�H����Ȃ��nullptr�ŁAIDXGIAdapter�̃A�h���X��n���B
			D3D_DRIVER_TYPE_HARDWARE,		// �h���C�o�̃^�C�v��n���BD3D_DRIVER_TYPE_HARDWARE �ȊO�͊�{�I�Ƀ\�t�g�E�F�A�����ŁA���ʂȂ��Ƃ�����ꍇ�ɗp����B
			nullptr,						// ��L��D3D_DRIVER_TYPE_SOFTWARE�ɐݒ肵���ۂɁA���̏������s��DLL�̃n���h����n���B����ȊO���w�肵�Ă���ۂɂ͕K��nullptr��n���B
			createDeviceFlags,				// ���炩�̃t���O���w�肷��B�ڂ�����D3D11_CREATE_DEVICE�񋓌^�Ō����B
			featureLevels,					// D3D_FEATURE_LEVEL�񋓌^�̔z���^����Bnullptr�ɂ��邱�Ƃł���Lfeature�Ɠ����̓��e�̔z�񂪎g�p�����B
			ARRAYSIZE(featureLevels),		// featureLevels�z��̗v�f����n���B
			D3D11_SDK_VERSION,				// SDK�̃o�[�W�����B�K�����̒l�B
			&swapchainDesc,					// �����Őݒ肵���\���̂ɐݒ肳��Ă���p�����[�^��SwapChain���쐬�����B
			swapchain_.GetAddressOf(),		// �쐬�����������ꍇ�ɁASwapChain�̃A�h���X���i�[����|�C���^�ϐ��ւ̃A�h���X�B�����Ŏw�肵���|�C���^�ϐ��o�R��SwapChain�𑀍삷��B
			device_.GetAddressOf(),			// �쐬�����������ꍇ�ɁADevice�̃A�h���X���i�[����|�C���^�ϐ��ւ̃A�h���X�B�����Ŏw�肵���|�C���^�ϐ��o�R��Device�𑀍삷��B
			&featureLevel,					// �쐬�ɐ�������D3D_FEATURE_LEVEL���i�[���邽�߂�D3D_FEATURE_LEVEL�񋓌^�ϐ��̃A�h���X��ݒ肷��B
			immediateContext_.GetAddressOf()	// �쐬�����������ꍇ�ɁAContext�̃A�h���X���i�[����|�C���^�ϐ��ւ̃A�h���X�B�����Ŏw�肵���|�C���^�ϐ��o�R��Context�𑀍삷��B
			);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// �����_�[�^�[�Q�b�g�r���[�̐���
	{
		// �X���b�v�`�F�[������o�b�N�o�b�t�@�e�N�X�`�����擾����B
		// ���X���b�v�`�F�[���ɓ����Ă���o�b�N�o�b�t�@�e�N�X�`����'�F'���������ރe�N�X�`���B
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		hr = swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		// �o�b�N�o�b�t�@�e�N�X�`���ւ̏������݂̑����ƂȂ郌���_�[�^�[�Q�b�g�r���[�𐶐�����B
		hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// �[�x�X�e���V���r���[�̐���
	{
		// �[�x�X�e���V�������������ނ��߂̃e�N�X�`�����쐬����B
		D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
		depthStencilBufferDesc.Width = screenWidth;
		depthStencilBufferDesc.Height = screenHeight;
		depthStencilBufferDesc.MipLevels = 1;
		depthStencilBufferDesc.ArraySize = 1;
		depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 1�s�N�Z��������A�[�x����24Bit / �X�e���V������8bit�̃e�N�X�`�����쐬����B
		depthStencilBufferDesc.SampleDesc.Count = 1;
		depthStencilBufferDesc.SampleDesc.Quality = 0;
		depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;		// �[�x�X�e���V���p�̃e�N�X�`�����쐬����B
		depthStencilBufferDesc.CPUAccessFlags = 0;
		depthStencilBufferDesc.MiscFlags = 0;
		hr = device_->CreateTexture2D(&depthStencilBufferDesc, nullptr, depthStencilBuffer_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		// �[�x�X�e���V���e�N�X�`���ւ̏������݂ɑ����ɂȂ�[�x�X�e���V���r���[���쐬����B
		hr = device_->CreateDepthStencilView(depthStencilBuffer_.Get(), nullptr, depthStencilView_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// �r���[�|�[�g�̐ݒ�
	{
		// ��ʂ̂ǂ̗̈��DirectX�ŕ`�������\�����邩�̐ݒ�B
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(screenWidth);
		viewport.Height = static_cast<float>(screenHeight);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		immediateContext_->RSSetViewports(1, &viewport);
	}

	// �V�F�[�_�[
	{
		shader_.resize(SHADER_ID::MAX);

		shader_[SHADER_ID::Default] = std::make_unique<LambertShader>(device_.Get());
		shader_[SHADER_ID::Phong] = std::make_unique<PhongShader>(device_.Get());

		//�p�����[�^�[�ݒ�
		{
			//�e
			ShadowMapData* sp = &shaderParameter3D_.shadowMapData;
			//	�V���h�E�}�b�v�̃T�C�Y
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

		//�|�X�g�G�t�F�N�g�p
		postEffectModelRenderTarget = std::make_unique<PostRenderTarget>(device_.Get(), screenWidth_, screenHeight_,  DXGI_FORMAT_R8G8B8A8_UNORM);
		postEffectModelDepthStencil = std::make_unique<PostDepthStencil>(device_.Get(), screenWidth_, screenHeight_);
	}

	// �����_��
	{
		debugRenderer_ = std::make_unique<DebugRenderer>(device_.Get());
		lineRenderer_ = std::make_unique<LineRenderer>(device_.Get(), 1024);
	}

	//�`�����ꊇ������
	{
		dx11State_ = std::make_unique<Dx11StateLib>();
		dx11State_->Dx11StateInit(device_.Get());
	}
}

// �f�X�g���N�^
Graphics::~Graphics()
{
}

// �`��^�[�Q�b�g�̑ޔ�
void Graphics::CacheRenderTargets()
{
	immediateContext_->RSGetViewports(&cachedViewportCount_, cachedViewports_);
	immediateContext_->OMGetRenderTargets(1, cachedRenderTargetView_.ReleaseAndGetAddressOf(), cachedDepthStencilView_.ReleaseAndGetAddressOf());
}

// �`��^�[�Q�b�g��߂�
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
//		// �X���b�v�`�F�[�����쐬���邽�߂̐ݒ�I�v�V����
//		DXGI_SWAP_CHAIN_DESC swapchainDesc;
//		{
//			swapchainDesc.BufferDesc.Width = 0;
//			swapchainDesc.BufferDesc.Height = 0;
//			swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
//			swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
//			swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 1�s�N�Z��������̊e�F(RGBA)��8bit(0�`255)�̃e�N�X�`��(�o�b�N�o�b�t�@)���쐬����B
//			swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
//			swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
//
//			swapchainDesc.SampleDesc.Count = 1;
//			swapchainDesc.SampleDesc.Quality = 0;
//			swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//			swapchainDesc.BufferCount = 1;		// �o�b�N�o�b�t�@�̐�
//			swapchainDesc.OutputWindow = hWnd_;	// DirectX�ŕ`�������\������E�C���h�E
//			swapchainDesc.Windowed = TRUE;		// �E�C���h�E���[�h���A�t���X�N���[���ɂ��邩�B
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
//		// �����_�[�^�[�Q�b�g�r���[�̐���
//		{
//			// �X���b�v�`�F�[������o�b�N�o�b�t�@�e�N�X�`�����擾����B
//			// ���X���b�v�`�F�[���ɓ����Ă���o�b�N�o�b�t�@�e�N�X�`����'�F'���������ރe�N�X�`���B
//			Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
//			HRESULT hr = s->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
//			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//
//			// �o�b�N�o�b�t�@�e�N�X�`���ւ̏������݂̑����ƂȂ郌���_�[�^�[�Q�b�g�r���[�𐶐�����B
//			hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, r.GetAddressOf());
//			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//		}
//
//		subWswapchain.emplace_back(s);
//		subWrenderTargetView.emplace_back(r);
//
//		//// �����_��
//		//{
//		//	std::unique_ptr<ImGuiRenderer> imGuiR = std::make_unique<ImGuiRenderer>(hWnd_, device_.Get());
//		//	subWImguiRenderer.emplace_back(std::move(imGuiR));
//		//}
//
//}