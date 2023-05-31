#include <memory>
#include <sstream>
#include <tchar.h>

#include "Graphics/Graphics.h"
#include "Input/Input.h"
#include "Framework.h"
#include "GameSource\Render\Effect\EffectManager.h"
#include "GameSource\Scene\SceneGame.h"
#include "GameSource\Scene\SceneTitle.h"
#include "GameSource\Scene\SceneManager.h"

#include "imgui.h"
#include "backends\imgui_impl_dx11.h"
#include "backends\imgui_impl_win32.h"

//#include "ModelAnimation\ModelWatch.h"
//ModelWatch mw;

// 垂直同期間隔設定
static const int syncInterval = 1;

// コンストラクタ
Framework::Framework(HWND hWnd)
	: hWnd(hWnd)
	, input(hWnd)
	, graphics(hWnd)
{
	//エフェクトマネージャー初期化
	EffectManager::Instance().Initialize();

	//sceneGame.Initialize();
	SceneManager::Instance().ChangeScene(new SceneGame);

}

// デストラクタ
Framework::~Framework()
{
	//sceneGame.Finalize();
	SceneManager::Instance().Clear();

	//エフェクトマネージャー終了化
	EffectManager::Instance().Finalize();
}

// 更新処理
void Framework::Update(float elapsedTime/*Elapsed seconds from last frame*/)
{
	// 入力更新処理
	input.Update();

	// シーン更新処理
	//sceneGame.Update(elapsedTime);
	SceneManager::Instance().Update(elapsedTime);

	//mw.Update(elapsedTime);
}

// 描画処理
void Framework::Render(float elapsedTime/*Elapsed seconds from last frame*/)
{
	//別スレッド中にデバイスコンテキストが使われていた場合に
	//同時アクセスしないように排他制御する
	std::lock_guard<std::mutex> lock(graphics.GetMutex());

	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();


	// シーン描画処理
	//sceneGame.Render();
	SceneManager::Instance().Render();

	// IMGUIデモウインドウ描画（IMGUI機能テスト用）
	//ImGui::ShowDemoWindow();

	//mw.Render(elapsedTime);

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// バックバッファに描画した画を画面に表示する。
	graphics.GetSwapChain()->Present(syncInterval, 0);

	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

// フレームレート計算
void Framework::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.
	static int frames = 0;
	static float time_tlapsed = 0.0f;

	frames++;

	// Compute averages over one second period.
	if ((timer.TimeStamp() - time_tlapsed) >= 1.0f)
	{
		float fps = static_cast<float>(frames); // fps = frameCnt / 1
		float mspf = 1000.0f / fps;
		std::ostringstream outs;
		outs.precision(6);
		outs << "FPS : " << fps << " / " << "Frame Time : " << mspf << " (ms)";
		SetWindowTextA(hWnd, outs.str().c_str());

		// Reset for next average.
		frames = 0;
		time_tlapsed += 1.0f;
	}
}

// アプリケーションループ
int Framework::Run()
{
	MSG msg = {};

	////サブウィンドウ追加
	//AddSubWindow();
	//AddSubWindow(1080,720);
	
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	//ここでドッキングと画面外出せる設定
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(graphics.GetDevice(), graphics.GetDeviceContext());

	//終了コードならwhileぬける　
	while (WM_QUIT != msg.message)
	{
		//PeekMessage = メッセージ受信
		//メッセージを取得しなかった場合、0 が返る
		//第3,4引数、両方0の場合すべてのメッセージを返す
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	//PM_REMOVE = PeekMessage 関数がメッセージを処理した後、そのメッセージをキューから削除
		{
			TranslateMessage(&msg);	//メッセージ変換
			DispatchMessage(&msg);	//プロシージャへコールバック
		}
		else
		{
			timer.Tick();
			CalculateFrameStats();

			float elapsedTime = syncInterval == 0
				? timer.TimeInterval()
				: syncInterval / 60.0f
				;
			Update(elapsedTime);
			Render(elapsedTime);

			////サブウィンドウ更新
			//SubWindowManager::Instance().SetSyncInterval(syncInterval);
			//SubWindowManager::Instance().Run(elapsedTime);
		}
	}

	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//SubWindowManager::Instance().Clear();	//サブウィンドウクリア

	//// ComPtr用リーク型名表示　作成
	//typedef HRESULT(__stdcall* fPtr)(const IID&, void**);
	//HMODULE hDll = GetModuleHandleW(L"dxgidebug.dll");
	//fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");

	//DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&debugGI);

	//debugGI->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_DETAIL);

	//Graphics::Instance().GetDevice()->QueryInterface(__uuidof(ID3D11Debug), (void**)&debugID);
	//debugID->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
	//debugID->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

	return static_cast<int>(msg.wParam);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// メッセージハンドラ
LRESULT CALLBACK Framework::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	case WM_ENTERSIZEMOVE:
		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
		timer.Stop();
		break;
	case WM_EXITSIZEMOVE:
		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
		timer.Start();
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

//サブウィンドウ
void Framework::AddSubWindow(int width, int height)
{
	RECT rc = { 0, 0, width, height };
	HWND hWnd2 = CreateWindow(_T("Game"), _T(""), 
		WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX | WS_VISIBLE | BS_PUSHBUTTON,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, ::GetModuleHandle(NULL), NULL);
	ShowWindow(hWnd2, __argc);

	Inspector* i = new Inspector(hWnd2, countSubWindow, width, height);
	SetWindowLongPtr(hWnd2, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&i));

	SubWindowManager::Instance().AddSubWindow(i);
	countSubWindow++;
}