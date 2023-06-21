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

	MAX,	//�ő吔��ێ�����
};

// �O���t�B�b�N�X
class Graphics
{
public:
	Graphics(HWND hWnd);
	~Graphics();

	// �C���X�^���X�擾
	static Graphics& Instance() { return *instance_; }

	// �f�o�C�X�擾
	ID3D11Device* GetDevice() const { return device_.Get(); }

	// �f�o�C�X�R���e�L�X�g�擾
	ID3D11DeviceContext* GetDeviceContext() const { return immediateContext_.Get(); }

	// �X���b�v�`�F�[���擾
	IDXGISwapChain* GetSwapChain() const { return swapchain_.Get(); }

	// �����_�[�^�[�Q�b�g�r���[�擾
	ID3D11RenderTargetView* GetRenderTargetView() const { return renderTargetView_.Get(); }

	// �f�v�X�X�e���V���r���[�擾
	ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView_.Get(); }

	// �V�F�[�_�[�擾
	Shader* GetShader(SHADER_ID shaderID) const { return shader_[shaderID].get(); }

	// �X�N���[�����擾
	float GetScreenWidth() const { return screenWidth_; }

	// �X�N���[�������擾
	float GetScreenHeight() const { return screenHeight_; }

	// �f�o�b�O�����_���擾
	DebugRenderer* GetDebugRenderer() const { return debugRenderer_.get(); }

	// ���C�������_���擾
	LineRenderer* GetLineRenderer() const { return lineRenderer_.get(); }

	//�`�����ݒ�Ăяo��
	const std::unique_ptr<Dx11StateLib>& GetDx11State() { return dx11State_; }

	//�~���[�e�b�N�X�擾
	std::mutex& GetMutex() { return mutex_; }

//public:	//�T�u�E�B���h�E
//	// �X���b�v�`�F�C���쐬
//	void CreateSubWindowSwapChain(HWND hWnd_, int width, int height);
//
//	// �X���b�v�`�F�[���擾
//	IDXGISwapChain* GetSubWindowSwapChain(int index) const { return subWswapchain[index].Get(); }
//
//	// �����_�[�^�[�Q�b�g�r���[�擾
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

	//�`�����ꊇ������
	std::unique_ptr<Dx11StateLib> dx11State_;

	float	screenWidth_;
	float	screenHeight_;

	std::mutex	mutex_;
};

