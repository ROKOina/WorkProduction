#include <memory>
#include <sstream>

#include "Inspector\Inspector.h"

// �R���X�g���N�^
Inspector::Inspector(HWND hWnd,int ID)
	: hWnd(hWnd),
	windowID(ID)
{
	Graphics::Instance().CreateSubWindowSwapChain(hWnd);
	p = new Model("Data/Model/Jammo/Jammo.fbx");
	cameraController = new CameraController();

	Graphics& graphics = Graphics::Instance();
	Camera& camera = Camera::Instance();
	camera.SetLookAt(
		DirectX::XMFLOAT3(0, 10, -10),
		DirectX::XMFLOAT3(0, 0, 0),
		DirectX::XMFLOAT3(0, 1, 0)
	);
	camera.SetPerspectiveFov(
		DirectX::XMConvertToRadians(45),
		graphics.GetScreenWidth() / graphics.GetScreenHeight(),
		0.1f, 1000.0f
	);
}

// �f�X�g���N�^
Inspector::~Inspector()
{
	delete p;
	//�J�����R���g���[���[�I����
	if (cameraController != nullptr)
	{
		delete cameraController;
		cameraController = nullptr;
	}

}

// �X�V����
void Inspector::Update(float elapsedTime/*Elapsed seconds from last frame*/)
{
	DirectX::XMFLOAT4X4 a{ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	p->UpdateTransform(a);

	cameraController->SetTarget({0,0,-500});
	cameraController->Update(elapsedTime);
}

// �`�揈��
void Inspector::Render(float elapsedTime/*Elapsed seconds from last frame*/)
{
	std::lock_guard<std::mutex> lock(Graphics::Instance().GetMutex());


	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	ID3D11RenderTargetView* rtv = graphics.GetSubWindowRenderTargetView(windowID);
	ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

	// ��ʃN���A�������_�[�^�[�Q�b�g�ݒ�
	FLOAT color[] = { 0.0f, 0.0f, 0.5f, 1.0f };	// RGBA(0.0�`1.0)
	dc->ClearRenderTargetView(rtv, color);
	dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	dc->OMSetRenderTargets(1, &rtv, dsv);

	// �`�揈��
	RenderContext rc;	//�`�悷�邽�߂ɕK�v�ȍ\����
	rc.lightDirection = { 0.0f, -1.0f, 0.0f, 0.0f };	// ���C�g�����i�������j

	//�J�����p�����[�^�[�ݒ�
	Camera& camera = Camera::Instance();
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();

	// 3D���f���`��
	{
		Shader* shader = graphics.GetShader();
		shader->Begin(dc, rc);	//�V�F�[�_�[�ɃJ�����̏���n��

		shader->Draw(dc, p);

		shader->End(dc);
	}


	// �o�b�N�o�b�t�@�ɕ`�悵�������ʂɕ\������B
	Graphics::Instance().GetSubWindowSwapChain(windowID)->Present(syncInterval, 0);
}

// �A�v���P�[�V�������[�v
int Inspector::Run(float elapsedTime)
{
			Update(elapsedTime);
			Render(elapsedTime);
	return 0;
}

