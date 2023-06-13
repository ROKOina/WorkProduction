#include "Graphics/Graphics.h"
#include "GameSource\Render\Effect\EffectManager.h"
#include "Input\Input.h"

#include "GameSource\Stage\StageManager.h"
#include "GameSource\Stage\StageMain.h"
#include "SceneGame.h"
#include "imgui.h"

#include "Components\System\GameObject.h"
#include "Components\RendererCom.h"
#include "Components\TransformCom.h"
#include "Components\CameraCom.h"
#include "Components\AnimationCom.h"

#include "GameSource\ScriptComponents\Player\PlayerCom.h"

// 初期化
void SceneGame::Initialize()
{
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("miru");
	}

	//仮オブジェクト
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("pico");
		obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });

		const char* filename = "Data/Model/pico/picoAnim.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);

		std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();

		std::shared_ptr<PlayerCom> p = obj->AddComponent<PlayerCom>();



		////でばふ
		//std::shared_ptr<GameObject> o[5];
		//for (int i = 0; i < 3; ++i)
		//{
		//	std::shared_ptr<GameObject> obj2;
		//	if (i == 0)
		//		obj2 = obj->AddChildObject();
		//	else
		//		obj2 = o[i - 1]->AddChildObject();


		//	obj2->SetPosition({ 10, 0, 0 });

		//	const char* filename = "Data/Model/pico/picoAnim.mdl";
		//	std::shared_ptr<RenderderCom> r = obj2->AddComponent<RenderderCom>();
		//	r->LoadModel(filename);
		//	o[i] = obj2;
		//}
	}

	//カメラを生成
	{
		std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Create();
		cameraObj->SetName("Camera");

		Graphics& graphics = Graphics::Instance();
		std::shared_ptr<CameraCom> c = cameraObj->AddComponent<CameraCom>();
		c->SetPerspectiveFov(
			DirectX::XMConvertToRadians(45),
			graphics.GetScreenWidth() / graphics.GetScreenHeight(),
			1.0f, 1000.0f
		);
		cameraObj->transform_->SetWorldPosition({ 0, 5, -10 });
	}

	//ステージ初期化
	StageManager& stageManager = StageManager::Instance();
	StageMain* stageMain = new StageMain();	//メイン（マップ）
	stageManager.Register(stageMain);

	//particle_ = std::make_unique<Particle>(DirectX::XMFLOAT4{ player->GetPosition().x,player->GetPosition().y,player->GetPosition().z,0 });
}

// 終了化
void SceneGame::Finalize()
{
	StageManager::Instance().Clear();
}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	GameObjectManager::Instance().Update(elapsedTime);

	//ステージ更新処理
	StageManager::Instance().Update(elapsedTime);
	//エフェクト更新処理
	EffectManager::Instance().Update(elapsedTime);

	Graphics& graphics = Graphics::Instance();


	//particle_->integrate(elapsedTime, { player->GetPosition().x,player->GetPosition().y,player->GetPosition().z,0 }, camera->GetView(), camera->GetProjection());
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
	std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
	rc.view = camera->GetView();
	rc.projection = camera->GetProjection();

	GameObjectManager::Instance().UpdateTransform();
	GameObjectManager::Instance().Render(camera->GetView(), camera->GetProjection());

	// 3Dモデル描画
	{
		Shader* shader = graphics.GetShader(0);
		shader->Begin(dc, rc);	//シェーダーにカメラの情報を渡す

		//ステージ描画
		StageManager::Instance().Render(dc, shader);
		////プレイヤー描画
		//player->Render(dc, shader_);

		shader->End(dc);
	}

	////パーティクル
	//{
	//	Dx11StateLib* dx11State_ = Graphics::Instance().GetDx11State().get();
	//	const float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//	dc->OMSetBlendState(
	//		dx11State_->GetBlendState(Dx11StateLib::BLEND_STATE_TYPE::PARTICLE).Get(),
	//		blend_factor, 0xFFFFFFFF);
	//	dc->OMSetDepthStencilState(
	//		dx11State_->GetDepthStencilState(Dx11StateLib::DEPTHSTENCIL_STATE_TYPE::PARTICLE).Get(),
	//		0);
	//	dc->RSSetState(
	//		dx11State_->GetRasterizerState(Dx11StateLib::RASTERIZER_TYPE::PARTICLE).Get()
	//	);

	//	particle_->Render(rc_);

	//	dc->OMSetBlendState(
	//		dx11State_->GetBlendState(Dx11StateLib::BLEND_STATE_TYPE::ALPHA).Get(),
	//		blend_factor, 0xFFFFFFFF);
	//	dc->OMSetDepthStencilState(
	//		dx11State_->GetDepthStencilState(Dx11StateLib::DEPTHSTENCIL_STATE_TYPE::DEPTH_ON_3D).Get(),
	//		0);
	//	dc->RSSetState(
	//		dx11State_->GetRasterizerState(Dx11StateLib::RASTERIZER_TYPE::FRONTCOUNTER_FALSE_CULLBACK).Get()
	//	);
	//}

	//3Dエフェクト描画
	{
		EffectManager::Instance().Render(rc.view, rc.projection);
	}

	// 3Dデバッグ描画
	{
		// ラインレンダラ描画実行
		graphics.GetLineRenderer()->Render(dc, rc.view, rc.projection);

		// デバッグレンダラ描画実行
		graphics.GetDebugRenderer()->Render(dc, rc.view, rc.projection);

	}

	// 2Dスプライト描画
	{
	}


}

