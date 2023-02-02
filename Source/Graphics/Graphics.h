#pragma once

#include <memory>
#include <d3d11.h>
#include <wrl.h>
#include "Graphics/Shader.h"
#include "Graphics/DebugRenderer.h"
#include "Graphics/LineRenderer.h"
//#include "Graphics/ImGuiRenderer.h"

#include <mutex>

// �O���t�B�b�N�X
class Graphics
{
public:
	Graphics(HWND hWnd);
	~Graphics();

	// �C���X�^���X�擾
	static Graphics& Instance() { return *instance; }

	// �f�o�C�X�擾
	ID3D11Device* GetDevice() const { return device.Get(); }

	// �f�o�C�X�R���e�L�X�g�擾
	ID3D11DeviceContext* GetDeviceContext() const { return immediateContext.Get(); }

	// �X���b�v�`�F�[���擾
	IDXGISwapChain* GetSwapChain() const { return swapchain.Get(); }

	// �����_�[�^�[�Q�b�g�r���[�擾
	ID3D11RenderTargetView* GetRenderTargetView() const { return renderTargetView.Get(); }

	// �f�v�X�X�e���V���r���[�擾
	ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView.Get(); }

	// �V�F�[�_�[�擾
	Shader* GetShader() const { return shader.get(); }

	// �X�N���[�����擾
	float GetScreenWidth() const { return screenWidth; }

	// �X�N���[�������擾
	float GetScreenHeight() const { return screenHeight; }

	// �f�o�b�O�����_���擾
	DebugRenderer* GetDebugRenderer() const { return debugRenderer.get(); }

	// ���C�������_���擾
	LineRenderer* GetLineRenderer() const { return lineRenderer.get(); }

	//// ImGui�����_���擾
	//ImGuiRenderer* GetImGuiRenderer() const { return imguiRenderer.get(); }

	//�~���[�e�b�N�X�擾
	std::mutex& GetMutex() { return mutex; }

public:	//�T�u�E�B���h�E
	// �X���b�v�`�F�C���쐬
	void CreateSubWindowSwapChain(HWND hWnd, int width, int height);

	// �X���b�v�`�F�[���擾
	IDXGISwapChain* GetSubWindowSwapChain(int index) const { return subWswapchain[index].Get(); }

	// �����_�[�^�[�Q�b�g�r���[�擾
	ID3D11RenderTargetView* GetSubWindowRenderTargetView(int index) const { return subWrenderTargetView[index].Get(); }

	//// ImGui�����_���擾
	//ImGuiRenderer* GetSubWindowImGuiRenderer(int index) const { return subWImguiRenderer[index].get(); }

private:
	std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>>	subWrenderTargetView;
	std::vector<Microsoft::WRL::ComPtr<IDXGISwapChain>>			subWswapchain;
	//std::vector<std::unique_ptr<ImGuiRenderer>>					subWImguiRenderer;

private:
	static Graphics*								instance;

	Microsoft::WRL::ComPtr<ID3D11Device>			device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		immediateContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain>			swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>			depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	depthStencilView;

	std::unique_ptr<Shader>							shader;
	std::unique_ptr<DebugRenderer>					debugRenderer;
	std::unique_ptr<LineRenderer>					lineRenderer;
	//std::unique_ptr<ImGuiRenderer>					imguiRenderer;

	float	screenWidth;
	float	screenHeight;

	std::mutex	mutex;
};

