#include "Graphics/Graphics.h"
#include "GameSource\Render\Effect\EffectManager.h"
#include "Input\Input.h"

#include "GameSource\Stage\StageManager.h"
#include "GameSource\Stage\StageMain.h"
#include "SceneGame.h"
#include "imgui.h"

// ������
void SceneGame::Initialize()
{
	//�X�e�[�W������
	StageManager& stageManager = StageManager::Instance();
	StageMain* stageMain = new StageMain();	//���C���i�}�b�v�j
	stageManager.Register(stageMain);

	//�J�����R���g���[���[������
	cameraController = new CameraController();
	//�v���C���[������
	player = new Player(cameraController);

	//�J���������ݒ�
	Graphics& graphics = Graphics::Instance();
	Camera& camera = cameraController->GetCamera();
	camera.SetLookAt(
		DirectX::XMFLOAT3(0, 10, -10),
		DirectX::XMFLOAT3(0, 0, 0),
		DirectX::XMFLOAT3(0, 1, 0)
	);
	camera.SetPerspectiveFov(
		DirectX::XMConvertToRadians(45),
		graphics.GetScreenWidth() / graphics.GetScreenHeight(),
		1.0f, 1000.0f
	);

	particle = std::make_unique<Particle>(DirectX::XMFLOAT4{ player->GetPosition().x,player->GetPosition().y,player->GetPosition().z,0 });
}

// �I����
void SceneGame::Finalize()
{
	StageManager::Instance().Clear();
	//�v���C���[�I����
	if (player != nullptr)
	{
		delete player;
		player = nullptr;
	}
	//�J�����R���g���[���[�I����
	if (cameraController != nullptr)
	{
		delete cameraController;
		cameraController = nullptr;
	}
}

// �X�V����
void SceneGame::Update(float elapsedTime)
{
	//�J�����R���g���[���[�X�V����
	DirectX::XMFLOAT3 target = player->GetPosition();
	target.y += 0.5f;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);
	//�X�e�[�W�X�V����
	StageManager::Instance().Update(elapsedTime);
	//�v���C���[�X�V����
	player->Update(elapsedTime);
	//�G�t�F�N�g�X�V����
	EffectManager::Instance().Update(elapsedTime);

	Graphics& graphics = Graphics::Instance();

	Camera& camera = cameraController->GetCamera();

	particle->integrate(elapsedTime, { player->GetPosition().x,player->GetPosition().y,player->GetPosition().z,0 }, camera.GetView(), camera.GetProjection());
}

// �`�揈��
void SceneGame::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
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
	Camera& camera = cameraController->GetCamera();
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();


	// 3D���f���`��
	{
		Shader* shader = graphics.GetShader();
		shader->Begin(dc, rc);	//�V�F�[�_�[�ɃJ�����̏���n��

		//�X�e�[�W�`��
		StageManager::Instance().Render(dc, shader);
		//�v���C���[�`��
		player->Render(dc, shader);

		shader->End(dc);
	}

	//�p�[�e�B�N��
	{
		Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();
		const float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		dc->OMSetBlendState(
			dx11State->GetBlendState(Dx11StateLib::BLEND_STATE_TYPE::PARTICLE).Get(),
			blend_factor, 0xFFFFFFFF);
		dc->OMSetDepthStencilState(
			dx11State->GetDepthStencilState(Dx11StateLib::DEPTHSTENCIL_STATE_TYPE::PARTICLE).Get(),
			0);
		dc->RSSetState(
			dx11State->GetRasterizerState(Dx11StateLib::RASTERIZER_TYPE::PARTICLE).Get()
		);

		particle->Render(rc);

		dc->OMSetBlendState(
			dx11State->GetBlendState(Dx11StateLib::BLEND_STATE_TYPE::ALPHA).Get(),
			blend_factor, 0xFFFFFFFF);
		dc->OMSetDepthStencilState(
			dx11State->GetDepthStencilState(Dx11StateLib::DEPTHSTENCIL_STATE_TYPE::DEPTH_ON_3D).Get(),
			0);
		dc->RSSetState(
			dx11State->GetRasterizerState(Dx11StateLib::RASTERIZER_TYPE::FRONTCOUNTER_FALSE_CULLBACK).Get()
		);
	}

	//3D�G�t�F�N�g�`��
	{
		EffectManager::Instance().Render(rc.view, rc.projection);
	}

	// 3D�f�o�b�O�`��
	{
		//�v���C���[�f�o�b�O�v���~�e�B�u�`��
		player->DrawDebugPrimitive();

		// ���C�������_���`����s
		graphics.GetLineRenderer()->Render(dc, rc.view, rc.projection);

		// �f�o�b�O�����_���`����s
		graphics.GetDebugRenderer()->Render(dc, rc.view, rc.projection);

	}

	// 2D�X�v���C�g�`��
	{
	}

	// 2D�f�o�b�OGUI�`��
	{
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("DebugMenu", nullptr, ImGuiWindowFlags_None))
		{

			//�v���C���[�f�o�b�O�`��
			player->DrawDebugGUI();

			//�J�����@
			if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DirectX::XMFLOAT3 eye = camera.GetEye();
				DirectX::XMFLOAT3 focus = camera.GetFocus();
				ImGui::InputFloat3("Eye", &eye.x);
				ImGui::InputFloat3("Focus", &focus.x);
			}

			//�J�����R���g���[���[
			cameraController->DrawDebugGUI();
		}
		ImGui::End();

	}

}

