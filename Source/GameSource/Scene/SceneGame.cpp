#include "Graphics/Graphics.h"
#include "GameSource\Render\Effect\EffectManager.h"
#include "Input\Input.h"

#include "GameSource\Stage\StageManager.h"
#include "GameSource\Stage\StageMain.h"
#include "SceneGame.h"
#include "imgui.h"

// 初期化
void SceneGame::Initialize()
{
	//ステージ初期化
	StageManager& stageManager = StageManager::Instance();
	StageMain* stageMain = new StageMain();	//メイン（マップ）
	stageManager.Register(stageMain);

	//カメラコントローラー初期化
	cameraController = new CameraController();
	//プレイヤー初期化
	player = new Player(cameraController);

	//カメラ初期設定
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

// 終了化
void SceneGame::Finalize()
{
	StageManager::Instance().Clear();
	//プレイヤー終了化
	if (player != nullptr)
	{
		delete player;
		player = nullptr;
	}
	//カメラコントローラー終了化
	if (cameraController != nullptr)
	{
		delete cameraController;
		cameraController = nullptr;
	}
}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	//カメラコントローラー更新処理
	DirectX::XMFLOAT3 target = player->GetPosition();
	target.y += 0.5f;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);
	//ステージ更新処理
	StageManager::Instance().Update(elapsedTime);
	//プレイヤー更新処理
	player->Update(elapsedTime);
	//エフェクト更新処理
	EffectManager::Instance().Update(elapsedTime);

	Graphics& graphics = Graphics::Instance();

	Camera& camera = cameraController->GetCamera();

	particle->integrate(elapsedTime, { player->GetPosition().x,player->GetPosition().y,player->GetPosition().z,0 }, camera.GetView(), camera.GetProjection());
}

// 描画処理
void SceneGame::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
	ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

	// 画面クリア＆レンダーターゲット設定
	FLOAT color[] = { 0.0f, 0.0f, 0.5f, 1.0f };	// RGBA(0.0～1.0)
	dc->ClearRenderTargetView(rtv, color);
	dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	dc->OMSetRenderTargets(1, &rtv, dsv);

	// 描画処理
	RenderContext rc;	//描画するために必要な構造体
	rc.lightDirection = { 0.0f, -1.0f, 0.0f, 0.0f };	// ライト方向（下方向）

	//カメラパラメーター設定
	Camera& camera = cameraController->GetCamera();
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();


	// 3Dモデル描画
	{
		Shader* shader = graphics.GetShader();
		shader->Begin(dc, rc);	//シェーダーにカメラの情報を渡す

		//ステージ描画
		StageManager::Instance().Render(dc, shader);
		//プレイヤー描画
		player->Render(dc, shader);

		shader->End(dc);
	}

	//パーティクル
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

	//3Dエフェクト描画
	{
		EffectManager::Instance().Render(rc.view, rc.projection);
	}

	// 3Dデバッグ描画
	{
		//プレイヤーデバッグプリミティブ描画
		player->DrawDebugPrimitive();

		// ラインレンダラ描画実行
		graphics.GetLineRenderer()->Render(dc, rc.view, rc.projection);

		// デバッグレンダラ描画実行
		graphics.GetDebugRenderer()->Render(dc, rc.view, rc.projection);

	}

	// 2Dスプライト描画
	{
	}

	// 2DデバッグGUI描画
	{
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("DebugMenu", nullptr, ImGuiWindowFlags_None))
		{

			//プレイヤーデバッグ描画
			player->DrawDebugGUI();

			//カメラ　
			if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DirectX::XMFLOAT3 eye = camera.GetEye();
				DirectX::XMFLOAT3 focus = camera.GetFocus();
				ImGui::InputFloat3("Eye", &eye.x);
				ImGui::InputFloat3("Focus", &focus.x);
			}

			//カメラコントローラー
			cameraController->DrawDebugGUI();
		}
		ImGui::End();

	}

}

