#pragma once

#include <memory>
#include <d3d11.h>
#include <wrl.h>
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Debug/DebugRenderer.h"
#include "Graphics/Debug/LineRenderer.h"
#include "Graphics\Dx11StateLib.h"
#include "Shaders\3D\ShaderParameter3D.h"

#include <mutex>

enum SHADER_ID
{
	Default,
	Phong,

	MAX,	//最大数を保持する
};

// グラフィックス
class Graphics
{
public:
	Graphics(HWND hWnd);
	~Graphics();

	// インスタンス取得
	static Graphics& Instance() { return *instance_; }

	// デバイス取得
	ID3D11Device* GetDevice() const { return device_.Get(); }

	// デバイスコンテキスト取得
	ID3D11DeviceContext* GetDeviceContext() const { return immediateContext_.Get(); }

	// スワップチェーン取得
	IDXGISwapChain* GetSwapChain() const { return swapchain_.Get(); }

	// レンダーターゲットビュー取得
	ID3D11RenderTargetView* GetRenderTargetView() const { return renderTargetView_.Get(); }

	// デプスステンシルビュー取得
	ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView_.Get(); }

	// シェーダー取得
	Shader* GetShader(SHADER_ID shaderID) const { return shader_[shaderID].get(); }

	// スクリーン幅取得
	float GetScreenWidth() const { return screenWidth_; }

	// スクリーン高さ取得
	float GetScreenHeight() const { return screenHeight_; }

	// デバッグレンダラ取得
	DebugRenderer* GetDebugRenderer() const { return debugRenderer_.get(); }

	// ラインレンダラ取得
	LineRenderer* GetLineRenderer() const { return lineRenderer_.get(); }

	//描画周り設定呼び出し
	const std::unique_ptr<Dx11StateLib>& GetDx11State() { return dx11State_; }

	//ミューテックス取得
	std::mutex& GetMutex() { return mutex_; }

//public:	//サブウィンドウ
//	// スワップチェイン作成
//	void CreateSubWindowSwapChain(HWND hWnd_, int width, int height);
//
//	// スワップチェーン取得
//	IDXGISwapChain* GetSubWindowSwapChain(int index) const { return subWswapchain[index].Get(); }
//
//	// レンダーターゲットビュー取得
//	ID3D11RenderTargetView* GetSubWindowRenderTargetView(int index) const { return subWrenderTargetView[index].Get(); }
//
private:
	//std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>>	subWrenderTargetView;
	//std::vector<Microsoft::WRL::ComPtr<IDXGISwapChain>>			subWswapchain;
	//std::vector<std::unique_ptr<ImGuiRenderer>>					subWImguiRenderer;


public:
	ShaderParameter3D shaderParameter3D_;

private:
	static Graphics*								instance_;

	Microsoft::WRL::ComPtr<ID3D11Device>			device_;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		immediateContext_;
	Microsoft::WRL::ComPtr<IDXGISwapChain>			swapchain_;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	renderTargetView_;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>			depthStencilBuffer_;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	depthStencilView_;

	std::vector<std::unique_ptr<Shader>>			shader_;
	std::unique_ptr<DebugRenderer>					debugRenderer_;
	std::unique_ptr<LineRenderer>					lineRenderer_;

	//描画周り一括初期化
	std::unique_ptr<Dx11StateLib> dx11State_;

	float	screenWidth_;
	float	screenHeight_;

	std::mutex	mutex_;
};

