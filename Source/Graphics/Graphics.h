#pragma once

#include <memory>
#include <d3d11.h>
#include <wrl.h>
#include "Graphics/Shaders/Shader.h"
#include "Graphics/DebugRenderer/DebugRenderer.h"
#include "Graphics/DebugRenderer/LineRenderer.h"
#include "Graphics\Dx11StateLib.h"
#include "Shaders\3D\ShaderParameter3D.h"

#include "ThreadPool/ThreadPool.h"

#include <mutex>

enum SHADER_ID
{
	Default,
	Phong,
	UnityChanToon,

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

	// �`��^�[�Q�b�g�̑ޔ�
	void CacheRenderTargets();

	// �`��^�[�Q�b�g��߂�
	void RestoreRenderTargets();

	//�|�X�g�G�t�F�N�g
	std::unique_ptr<PostRenderTarget>& GetPostEffectModelRenderTarget() { return postEffectModelRenderTarget; }
	std::unique_ptr<PostDepthStencil>& GetPostEffectModelDepthStencilView() { return postEffectModelDepthStencil; }

	//FPS(�Z�b�g�̓t���[�����[�N�ł����g��Ȃ��\��)
	void SetFPSFramework(float fps) { this->fps_ = fps; }
	float GetFPS() { return fps_; }

	//�~���[�e�b�N�X�擾
	std::mutex& GetMutex() { return mutex_; }

	//�X���b�h�v�[���擾
	ThreadPool* GetThreadPool() { return threadPool_.get(); }

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
	static Graphics* instance_;

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

	//�`��^�[�Q�b�g���p
	UINT			cachedViewportCount_{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
	D3D11_VIEWPORT	cachedViewports_[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	cachedRenderTargetView_;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	cachedDepthStencilView_;

	//�|�X�g�G�t�F�N�g�p
	std::unique_ptr<PostRenderTarget> postEffectModelRenderTarget;
	std::unique_ptr<PostDepthStencil> postEffectModelDepthStencil;

private:
	float	screenWidth_;
	float	screenHeight_;

	float fps_;
	std::mutex	mutex_;

	//�X���b�h�v�[��
	std::unique_ptr<ThreadPool> threadPool_;

	//���E�̑��x
public:
	float GetWorldSpeed() { return worldSpeed_; }
	void SetWorldSpeed(float speed) { worldSpeed_ = speed; }

private:
	float worldSpeed_ = 1.0f;
};

