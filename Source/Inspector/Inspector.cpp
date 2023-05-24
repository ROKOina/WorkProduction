#include <memory>
#include <sstream>

#include "Inspector\Inspector.h"

// �R���X�g���N�^
Inspector::Inspector(HWND hWnd, int ID, int width, int height)
	:SubWindow(hWnd, ID, width, height)
{
	//p = std::make_unique<Model>("Data/Model/Jammo/Jammo.fbx");
	//cameraController = std::make_unique<CameraController>();

	//Graphics& graphics = Graphics::Instance();
	//Camera& camera = Camera::Instance();
	//camera.SetLookAt(
	//	DirectX::XMFLOAT3(0, 10, -10),
	//	DirectX::XMFLOAT3(0, 0, 0),
	//	DirectX::XMFLOAT3(0, 1, 0)
	//);
	//camera.SetPerspectiveFov(
	//	DirectX::XMConvertToRadians(45),
	//	graphics.GetScreenWidth() / graphics.GetScreenHeight(),
	//	0.1f, 1000.0f
	//);
}

// �f�X�g���N�^
Inspector::~Inspector()
{
}

// �X�V����
void Inspector::Update(float elapsedTime/*Elapsed seconds from last frame*/)
{
	//DirectX::XMFLOAT4X4 a{ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	//p->UpdateTransform(a);

	//cameraController->SetTarget({0,0,-500});
	//cameraController->Update(elapsedTime);
}

// �`�揈��
void Inspector::Render(float elapsedTime/*Elapsed seconds from last frame*/)
{
	std::lock_guard<std::mutex> lock(Graphics::Instance().GetMutex());

	Graphics& graphics = Graphics::Instance();

	//// IMGUI�t���[���J�n����
	//graphics.GetSubWindowImGuiRenderer(windowID)->NewFrame();

	//ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	//ID3D11RenderTargetView* rtv = graphics.GetSubWindowRenderTargetView(windowID);
	//ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

	//// ��ʃN���A�������_�[�^�[�Q�b�g�ݒ�
	//FLOAT color[] = { 0.0f, 0.0f, 0.5f, 1.0f };	// RGBA(0.0�`1.0)
	//dc->ClearRenderTargetView(rtv, color);
	//dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//dc->OMSetRenderTargets(1, &rtv, dsv);

	// �`�揈��
	RenderContext rc;	//�`�悷�邽�߂ɕK�v�ȍ\����
	rc.lightDirection = { 0.0f, -1.0f, 0.0f, 0.0f };	// ���C�g�����i�������j

	////�J�����p�����[�^�[�ݒ�
	//Camera& camera = Camera::Instance();
	//rc.view = camera.GetView();
	//rc.projection = camera.GetProjection();

	// 3D���f���`��
	{
		//Shader* shader = graphics.GetShader();
		//shader->Begin(dc, rc);	//�V�F�[�_�[�ɃJ�����̏���n��

		//shader->Draw(dc, p.get());

		//shader->End(dc);
	}

	//// 2D�f�o�b�OGUI�`��
	//{
	//	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	//	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	//	if (ImGui::Begin("DebugMenu2", nullptr, ImGuiWindowFlags_None))
	//	{
	//		static float a;
	//		ImGui::DragFloat("a", &a);
	//	}
	//	ImGui::End();

	//}


	//// IMGUI�`��
	//graphics.GetSubWindowImGuiRenderer(windowID)->Render(dc);

	SubWindow::Render(elapsedTime);
}
