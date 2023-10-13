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
#include "Components\AnimatorCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"

#include "GameSource\ScriptComponents\Player\PlayerCom.h"
#include "GameSource\ScriptComponents\Player\PlayerCameraCom.h"
#include "GameSource\ScriptComponents\Enemy\EnemyNearCom.h"
#include "GameSource\ScriptComponents\Enemy\EnemyFarCom.h"
//#include "GameSource\ScriptComponents\Enemy\EnemyCom.h"
#include "GameSource\ScriptComponents\Weapon\WeaponCom.h"
#include "GameSource\ScriptComponents\Weapon\SwordTrailCom.h"
#include "GameSource\ScriptComponents\CharacterStatusCom.h"

#include <thread>

#include "GameSource\ScriptComponents\Enemy\EnemyManager.h"

// 初期化
void SceneGame::Initialize()
{
	//床
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("stageYuka");
		obj->transform_->SetWorldPosition({ 0, 0, 0 });
		obj->transform_->SetScale({ 1.00f, 1.00f, 1.00f });

		const char* filename = "Data/Model/stages/yuka/cookie.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::UnityChanToon);
	}
	//壁
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("stageKabe");
		obj->transform_->SetWorldPosition({ 0, 0, 0 });
		obj->transform_->SetScale({ 1.00f, 1.00f, 1.00f });

		const char* filename = "Data/Model/stages/kabe/kabeSara.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::UnityChanToon);
	}

	//壁当たり判定用+
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("kabePlus");
		obj->transform_->SetWorldPosition({ 28.5f, 0, 23.0f });

		std::shared_ptr<BoxColliderCom> c = obj->AddComponent<BoxColliderCom>();
	}
	//壁当たり判定用-
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("kabeMinas");
		obj->transform_->SetWorldPosition({ -25.0f, 0, -30.0f });

		std::shared_ptr<BoxColliderCom> c = obj->AddComponent<BoxColliderCom>();
	}

	//enemyNear
	for(int i=0;i<5;++i)
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("picolabo");
		obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });
		obj->transform_->SetWorldPosition({ 3, 0, 5 });
		obj->transform_->SetEulerRotation({ 0,180,0 });

		const char* filename = "Data/Model/picolabo/picolabo.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::UnityChanToon);

		////発光を消す
		//std::vector<ModelResource::Material>& materials = r->GetModel()->GetResourceShared()->GetMaterialsEdit();
		//materials[0].toonStruct._Emissive_Color.w = 0;


		std::shared_ptr<MovementCom> m = obj->AddComponent<MovementCom>();
		std::shared_ptr<CharacterStatusCom> status = obj->AddComponent<CharacterStatusCom>();

		std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();
		//a->PlayAnimation(5, true);

		std::shared_ptr<AnimatorCom> animator = obj->AddComponent<AnimatorCom>();

		std::shared_ptr<BoxColliderCom> c = obj->AddComponent<BoxColliderCom>();
		c->SetMyTag(COLLIDER_TAG::Enemy);
		c->SetJudgeTag(COLLIDER_TAG::Player | COLLIDER_TAG::Wall);
		c->SetSize(DirectX::XMFLOAT3(0.5f, 1.2f, 0.5f));
		c->SetOffsetPosition(DirectX::XMFLOAT3(0, 0.9f, 0));

		std::shared_ptr<EnemyNearCom> e = obj->AddComponent<EnemyNearCom>();

		//攻撃当たり判定用
		{
			std::shared_ptr<GameObject> attack = obj->AddChildObject();
			attack->SetName("picolaboAttack");
			std::shared_ptr<SphereColliderCom> attackCol = attack->AddComponent<SphereColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::EnemyAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Player);
		}
		//ジャスト回避用
		{
			std::shared_ptr<GameObject> justAttack = obj->AddChildObject();
			justAttack->SetName("picolaboAttackJust");
			std::shared_ptr<BoxColliderCom> justCol = justAttack->AddComponent<BoxColliderCom>();
			justCol->SetMyTag(COLLIDER_TAG::JustAvoid);
			justCol->SetJudgeTag(COLLIDER_TAG::Player);
			justCol->SetSize({ 1.3f,1,1.3f });

			justAttack->transform_->SetLocalPosition({ -1.569f ,0,95.493f });
		}

		//押し出し用当たり判定
		{
			std::shared_ptr<GameObject> pushBack = obj->AddChildObject();
			pushBack->SetName("PushBackObj");
			std::shared_ptr<SphereColliderCom> col = pushBack->AddComponent<SphereColliderCom>();
			col->SetMyTag(COLLIDER_TAG::EnemyPushBack);
			col->SetJudgeTag(COLLIDER_TAG::PlayerPushBack | COLLIDER_TAG::EnemyPushBack);
			col->SetPushBack(true);
			col->SetPushBackObj(obj);
		}

		//剣("RightHand")
		{
			std::shared_ptr<GameObject> sword = obj->AddChildObject();
			sword->SetName("Banana");
			sword->transform_->SetScale(DirectX::XMFLOAT3(3, 3, 3));
			sword->transform_->SetEulerRotation(DirectX::XMFLOAT3(7, -85, 108));
			sword->transform_->SetLocalPosition(DirectX::XMFLOAT3(11, -6, -15));

			const char* filename = "Data/Model/Swords/banana/banana.mdl";
			std::shared_ptr<RendererCom> r = sword->AddComponent<RendererCom>();
			r->LoadModel(filename);
			r->SetShaderID(SHADER_ID::UnityChanToon);

			std::shared_ptr<CapsuleColliderCom> attackCol = sword->AddComponent<CapsuleColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::EnemyAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Player);
			attackCol->SetRadius(0.19f);

			std::shared_ptr<WeaponCom> weapon = sword->AddComponent<WeaponCom>();
			weapon->SetObject(sword->GetParent());
			weapon->SetNodeName("RightHand");
			weapon->SetColliderUpDown({ 1.36f,0 });

			//std::shared_ptr<SwordTrailCom>  trail = sword->AddComponent<SwordTrailCom>();
			//trail->SetHeadTailNodeName("candy", "head");	//トレイル表示ノード指定
		}
	}

	//enemyFar
	for(int i=0;i<0;++i)
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("picolabo");
		obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });
		obj->transform_->SetWorldPosition({ 3, 0, 5 });
		obj->transform_->SetEulerRotation({ 0,180,0 });

		const char* filename = "Data/Model/picolabo/picolabo.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::UnityChanToon);

		////発光を消す
		//std::vector<ModelResource::Material>& materials = r->GetModel()->GetResourceShared()->GetMaterialsEdit();
		//materials[0].toonStruct._Emissive_Color.w = 0;


		std::shared_ptr<MovementCom> m = obj->AddComponent<MovementCom>();
		std::shared_ptr<CharacterStatusCom> status = obj->AddComponent<CharacterStatusCom>();

		std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();
		//a->PlayAnimation(5, true);

		std::shared_ptr<AnimatorCom> animator = obj->AddComponent<AnimatorCom>();

		std::shared_ptr<BoxColliderCom> c = obj->AddComponent<BoxColliderCom>();
		c->SetMyTag(COLLIDER_TAG::Enemy);
		c->SetJudgeTag(COLLIDER_TAG::Player | COLLIDER_TAG::Wall);
		c->SetSize(DirectX::XMFLOAT3(0.5f, 1.2f, 0.5f));
		c->SetOffsetPosition(DirectX::XMFLOAT3(0, 0.9f, 0));

		std::shared_ptr<EnemyFarCom> e = obj->AddComponent<EnemyFarCom>();

		//攻撃当たり判定用
		{
			std::shared_ptr<GameObject> attack = obj->AddChildObject();
			attack->SetName("picolaboAttack");
			std::shared_ptr<SphereColliderCom> attackCol = attack->AddComponent<SphereColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::EnemyAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Player);
		}
		//ジャスト回避用
		{
			std::shared_ptr<GameObject> justAttack = obj->AddChildObject();
			justAttack->SetName("picolaboAttackJust");
			std::shared_ptr<BoxColliderCom> justCol = justAttack->AddComponent<BoxColliderCom>();
			justCol->SetMyTag(COLLIDER_TAG::JustAvoid);
			justCol->SetJudgeTag(COLLIDER_TAG::Player);
			justCol->SetSize({ 1.3f,1,1.3f });

			justAttack->transform_->SetLocalPosition({ -1.569f ,0,95.493f });
		}

		//押し出し用当たり判定
		{
			std::shared_ptr<GameObject> pushBack = obj->AddChildObject();
			pushBack->SetName("PushBackObj");
			std::shared_ptr<SphereColliderCom> col = pushBack->AddComponent<SphereColliderCom>();
			col->SetMyTag(COLLIDER_TAG::EnemyPushBack);
			col->SetJudgeTag(COLLIDER_TAG::PlayerPushBack | COLLIDER_TAG::EnemyPushBack);
			col->SetPushBack(true);
			col->SetPushBackObj(obj);
		}

		//剣("RightHand")
		{
			std::shared_ptr<GameObject> sword = obj->AddChildObject();
			sword->SetName("Banana");
			sword->transform_->SetScale(DirectX::XMFLOAT3(3, 3, 3));
			sword->transform_->SetEulerRotation(DirectX::XMFLOAT3(7, -85, 108));
			sword->transform_->SetLocalPosition(DirectX::XMFLOAT3(11, -6, -15));

			const char* filename = "Data/Model/Swords/banana/banana.mdl";
			std::shared_ptr<RendererCom> r = sword->AddComponent<RendererCom>();
			r->LoadModel(filename);
			r->SetShaderID(SHADER_ID::UnityChanToon);

			std::shared_ptr<CapsuleColliderCom> attackCol = sword->AddComponent<CapsuleColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::EnemyAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Player);
			attackCol->SetRadius(0.19f);

			std::shared_ptr<WeaponCom> weapon = sword->AddComponent<WeaponCom>();
			weapon->SetObject(sword->GetParent());
			weapon->SetNodeName("RightHand");
			weapon->SetColliderUpDown({ 1.36f,0 });

			//std::shared_ptr<SwordTrailCom>  trail = sword->AddComponent<SwordTrailCom>();
			//trail->SetHeadTailNodeName("candy", "head");	//トレイル表示ノード指定
		}



	}


	//プレイヤー
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("pico");
		obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });

		const char* filename = "Data/Model/pico/picoAnim.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::UnityChanToon);

		std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();
		//a->PlayAnimation(4, true);

		std::shared_ptr<AnimatorCom> animator = obj->AddComponent<AnimatorCom>();

		std::shared_ptr<MovementCom> m = obj->AddComponent<MovementCom>();
		std::shared_ptr<CharacterStatusCom> status = obj->AddComponent<CharacterStatusCom>();

		std::shared_ptr<CapsuleColliderCom> c = obj->AddComponent<CapsuleColliderCom>();
		//std::shared_ptr<BoxColliderCom> c = obj->AddComponent<BoxColliderCom>();
		c->SetMyTag(COLLIDER_TAG::Player);
		c->SetJudgeTag(COLLIDER_TAG::Enemy | COLLIDER_TAG::Wall | COLLIDER_TAG::JustAvoid);
		//c->SetSize(DirectX::XMFLOAT3(0.2f, 0.6f, 0.2f));
		//c->SetOffsetPosition(DirectX::XMFLOAT3(0, 0.8f, 0));

		std::shared_ptr<PlayerCom> p = obj->AddComponent<PlayerCom>();
		std::shared_ptr<PlayerCameraCom> playerCamera = obj->AddComponent<PlayerCameraCom>();

		//攻撃補正距離当たり判定
		{
			std::shared_ptr<GameObject> attackAssist = obj->AddChildObject();
			attackAssist->SetName("attackAssist");
			std::shared_ptr<SphereColliderCom> attackAssistCol = attackAssist->AddComponent<SphereColliderCom>();
			attackAssistCol->SetMyTag(COLLIDER_TAG::PlayerAttackAssist);
			attackAssistCol->SetJudgeTag(COLLIDER_TAG::Enemy);
			attackAssistCol->SetRadius(3);
		}

		//剣("RightHandMiddle2")
		{
			std::shared_ptr<GameObject> sword = obj->AddChildObject();
			sword->SetName("Candy");
			sword->transform_->SetScale(DirectX::XMFLOAT3(2, 2, 2));
			sword->transform_->SetEulerRotation(DirectX::XMFLOAT3(-154, -85, 82));


			const char* filename = "Data/Model/Swords/Candy/Candy.mdl";
			std::shared_ptr<RendererCom> r = sword->AddComponent<RendererCom>();
			r->LoadModel(filename);
			r->SetShaderID(SHADER_ID::UnityChanToon);

			std::shared_ptr<CapsuleColliderCom> attackCol = sword->AddComponent<CapsuleColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::PlayerAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Enemy);
			attackCol->SetRadius(0.19f);

			std::shared_ptr<WeaponCom> weapon = sword->AddComponent<WeaponCom>();
			weapon->SetObject(sword->GetParent());
			weapon->SetNodeName("RightHandMiddle2");
			weapon->SetColliderUpDown({ 2,0 });

			std::shared_ptr<SwordTrailCom>  trail= sword->AddComponent<SwordTrailCom>();
			trail->SetHeadTailNodeName("candy", "head");	//トレイル表示ノード指定
		}

		//ジャスト回避用プレイヤー
		for (int i = 0; i < 4; ++i)
		{
			std::shared_ptr<GameObject> picoJust = obj->AddChildObject();
			std::string s = "picoJust" + std::to_string(i);
			picoJust->SetName(s.c_str());
			picoJust->SetEnabled(false);

			const char* filename = "Data/Model/pico/picoAnim.mdl";
			std::shared_ptr<RendererCom> picoJustRenderer = picoJust->AddComponent<RendererCom>();
			picoJustRenderer->LoadModel(filename);
			picoJustRenderer->SetShaderID(SHADER_ID::Phong);
			picoJustRenderer->GetModel()->SetMaterialColor({ 0.4f,0.3f,0.1f,0.65f });

			picoJust->AddComponent<AnimationCom>();

			picoJust->transform_->SetWorldPosition({ 0,0,100 });
		}

		//見る
		{
			std::shared_ptr<GameObject> look = obj->AddChildObject();
			look->SetName("look");
			look->AddComponent<SphereColliderCom>();
		}

		//押し出し用当たり判定
		{
			std::shared_ptr<GameObject> pushBack = obj->AddChildObject();
			pushBack->SetName("PushBackObj");
			std::shared_ptr<SphereColliderCom> col = pushBack->AddComponent<SphereColliderCom>();
			col->SetMyTag(COLLIDER_TAG::PlayerPushBack);
			col->SetJudgeTag(COLLIDER_TAG::EnemyPushBack);
			col->SetPushBack(true);
			col->SetPushBackObj(obj);
		}
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
			0.1f, 1000.0f
		);
		cameraObj->transform_->SetWorldPosition({ 0, 5, -10 });
	}

	//メインカメラ設定
	std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
	mainCamera_ = camera;

	////ステージ初期化
	//StageManager& stageManager = StageManager::Instance();
	//StageMain* stageMain = new StageMain();	//メイン（マップ）
	//stageManager.Register(stageMain);

	//ポストエフェクト
	{
		Graphics& graphics = Graphics::Instance();
		postEff = std::make_unique<PostEffect>(
			static_cast<UINT>(graphics.GetScreenWidth()) ,
			static_cast<UINT>(graphics.GetScreenHeight()));

		graphics.shaderParameter3D_.bloomData2.intensity = 5;
		graphics.shaderParameter3D_.bloomData2.threshold = 1;
	}

	//std::shared_ptr<GameObject> player = GameObjectManager::Instance().Find("pico");
	//particle_ = std::make_unique<Particle>(DirectX::XMFLOAT4{ player->transform_->GetWorldPosition().x, player->transform_->GetWorldPosition().y, player->transform_->GetWorldPosition().z,0 });
}

// 終了化
void SceneGame::Finalize()
{
	//StageManager::Instance().Clear();
}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	GameObjectManager::Instance().Update(elapsedTime);

	////ステージ更新処理
	//StageManager::Instance().Update(elapsedTime);
	//エフェクト更新処理
	EffectManager::Instance().Update(elapsedTime);

	Graphics& graphics = Graphics::Instance();

	//std::shared_ptr<GameObject> player = GameObjectManager::Instance().Find("pico");
	//std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
	//particle_->integrate(elapsedTime, { player->transform_->GetWorldPosition().x, player->transform_->GetWorldPosition().y, player->transform_->GetWorldPosition().z, 0 }, camera->GetView(), camera->GetProjection());
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
	ShaderParameter3D& rc = graphics.shaderParameter3D_;
	rc.lightDirection = graphics.shaderParameter3D_.lightDirection;	// ライト方向

	//カメラパラメーター設定
	rc.view = mainCamera_->GetView();
	rc.projection = mainCamera_->GetProjection();
	DirectX::XMFLOAT3 cameraPos = mainCamera_->GetGameObject()->transform_->GetWorldPosition();
	rc.viewPosition = { cameraPos.x,cameraPos.y,cameraPos.z,1 };


	//バッファ避難
	Graphics::Instance().CacheRenderTargets();

	//ポストエフェクト用切り替え
	PostRenderTarget* ps = Graphics::Instance().GetPostEffectModelRenderTarget().get();
	PostDepthStencil* ds = Graphics::Instance().GetPostEffectModelDepthStencilView().get();

	// 画面クリア＆レンダーターゲット設定
	rtv = {};
	dsv = {};
	rtv = ps->renderTargetView.Get();
	dsv = ds->depthStencilView.Get();
	dc->ClearRenderTargetView(rtv, color);
	dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	dc->OMSetRenderTargets(1, &rtv, dsv);

	// ビューポートの設定
	D3D11_VIEWPORT	vp = {};
	vp.Width = static_cast<float>(ps->width);
	vp.Height = static_cast<float>(ps->height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	dc->RSSetViewports(1, &vp);

	//スカイマップ描画
	postEff->SkymapRender();

	GameObjectManager::Instance().UpdateTransform();

	GameObjectManager::Instance().Render(mainCamera_->GetView(), mainCamera_->GetProjection());

	// デバッグレンダラ描画実行
	graphics.GetDebugRenderer()->Render(dc, mainCamera_->GetView(), mainCamera_->GetProjection());



	//// 3Dモデル描画
	//{
	//	Shader* shader = graphics.GetShader(SHADER_ID::Phong);
	//	shader->Begin(dc, rc);	//シェーダーにカメラの情報を渡す

	//	////ステージ描画
	//	//StageManager::Instance().Render(dc, shader);
	//	////プレイヤー描画
	//	//player->Render(dc, shader_);

	//	shader->End(dc);
	//}

	//	//パーティクル
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

	//	particle_->Render(graphics.shaderParameter3D_);

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

	//バッファ戻す
	Graphics::Instance().RestoreRenderTargets();

	postEff->Render();
	postEff->ImGuiRender();
	

	//3Dエフェクト描画
	{
		EffectManager::Instance().Render(mainCamera_->GetView(), mainCamera_->GetProjection());
	}

	// 3Dデバッグ描画
	{
		// ラインレンダラ描画実行
		graphics.GetLineRenderer()->Render(dc, mainCamera_->GetView(), mainCamera_->GetProjection());

		ImGui::SetNextWindowPos(ImVec2(950, 50), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		ImGui::Begin("WorldSetting", nullptr, ImGuiWindowFlags_None);

		//世界のスピード
		float worldSpeed = graphics.GetWorldSpeed();
		ImGui::DragFloat("worldSpeed", &worldSpeed, 0.01f, 0, 2);
		graphics.SetWorldSpeed(worldSpeed);

		ImGui::End();

	}

	// 2Dスプライト描画
	{
	}


}

