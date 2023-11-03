#include "Graphics/Graphics.h"
#include "GameSource\Render\Effect\EffectManager.h"
#include "Input\Input.h"

#include "SceneGame.h"
#include "SceneTitle.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "imgui.h"

#include "Components\System\GameObject.h"
#include "Components\RendererCom.h"
#include "Components\TransformCom.h"
#include "Components\CameraCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"
#include "Components\ParticleSystemCom.h"

#include "GameSource\ScriptComponents\Player\PlayerCom.h"
#include "GameSource\ScriptComponents\Player\PlayerCameraCom.h"
#include "GameSource\ScriptComponents\Enemy\EnemyNearCom.h"
#include "GameSource\ScriptComponents\Enemy\EnemyFarCom.h"
//#include "GameSource\ScriptComponents\Enemy\EnemyCom.h"
#include "GameSource\ScriptComponents\Weapon\WeaponCom.h"
#include "GameSource\ScriptComponents\Weapon\SwordTrailCom.h"
#include "GameSource\ScriptComponents\CharacterStatusCom.h"

#include "GameSource\ScriptComponents\Enemy\EnemyManager.h"
#include "Components/ParticleComManager.h"

//経路探査
#include "GameSource/Stage/PathSearch.h"

//未完成
//レベルデザイン時に起動
//メモ帳に保存する
//#define StageEdit

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

#if defined(StageEdit)

#else

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
	for(int i = 0;i < 1;++i)
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("picolabo");
		obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });
		obj->transform_->SetWorldPosition({ -10.0f+i*2, 0, 5 });
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
		obj->transform_->SetWorldPosition({ 0, 0, -10 });

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

		//攻撃補正近距離当たり判定
		{
			std::shared_ptr<GameObject> attackAssist = obj->AddChildObject();
			attackAssist->SetName("attackAssistNear");
			std::shared_ptr<SphereColliderCom> attackAssistCol = attackAssist->AddComponent<SphereColliderCom>();
			attackAssistCol->SetMyTag(COLLIDER_TAG::PlayerAttackAssist);
			attackAssistCol->SetJudgeTag(COLLIDER_TAG::Enemy);
			attackAssistCol->SetRadius(3);
		}

		//攻撃補正中距離当たり判定
		{
			std::shared_ptr<GameObject> attackAssist = obj->AddChildObject();
			attackAssist->SetName("attackAssistMedium");
			std::shared_ptr<SphereColliderCom> attackAssistCol = attackAssist->AddComponent<SphereColliderCom>();
			attackAssistCol->SetMyTag(COLLIDER_TAG::PlayerAttackAssist);
			attackAssistCol->SetJudgeTag(COLLIDER_TAG::Enemy);
			attackAssistCol->SetRadius(10);
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

			{
				std::shared_ptr<GameObject> p = sword->AddChildObject();
				p->SetName("Particle");

				std::shared_ptr<ParticleSystemCom> c = p->AddComponent<ParticleSystemCom>(1000);
				c->SetSweetsParticle(true);	//お菓子用

				c->Load("Data/Effect/para/SwordOkasi.ipff");
				//c->LoadTexture("Data/Sprite/sweetsParticle.png");

				p->AddComponent<SphereColliderCom>();
			}

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

		////見る
		//{
		//	std::shared_ptr<GameObject> look = obj->AddChildObject();
		//	look->SetName("look");
		//	look->AddComponent<SphereColliderCom>();
		//}

		//押し出し用当たり判定
		{
			std::shared_ptr<GameObject> pushBack = obj->AddChildObject();
			pushBack->SetName("PushBackObj");
			std::shared_ptr<SphereColliderCom> col = pushBack->AddComponent<SphereColliderCom>();
			col->SetMyTag(COLLIDER_TAG::PlayerPushBack);
			col->SetJudgeTag(COLLIDER_TAG::EnemyPushBack);
			col->SetPushBack(true);
			col->SetPushBackObj(obj);
			col->SetWeight(10.0f);
		}
	}

	//particle
	for(int i=0;i<1;++i)
	{
		std::shared_ptr<GameObject> p = GameObjectManager::Instance().Create();
		p->SetName("Particle");
		p->transform_->SetWorldPosition(DirectX::XMFLOAT3(i * 1.0f, 1, 0));

		std::shared_ptr<ParticleSystemCom> c = p->AddComponent<ParticleSystemCom>(1000);
		c->SetSweetsParticle(true);	//お菓子用

		c->LoadTexture("Data/Sprite/sweetsParticle.png");
		//c->LoadTexture("Data/Sprite/default_eff.png");
		//c->LoadTexture("Data/Sprite/smoke_pop.png");
		//c->Load("Data/Effect/para/honoo.ipff");

		p->AddComponent<SphereColliderCom>();

		//{
		//	std::shared_ptr<GameObject> pChild = p->AddChildObject();
		//	pChild->SetName("ParticleChild");

		//	std::shared_ptr<ParticleSystemCom> c1 = pChild->AddComponent<ParticleSystemCom>(10000);
		//	c1->LoadTexture("Data/Sprite/smoke_pop.png");
		//}
	}

#endif

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

	//ポストエフェクト
	{
		Graphics& graphics = Graphics::Instance();
		postEff_ = std::make_unique<PostEffect>(
			static_cast<UINT>(graphics.GetScreenWidth()) ,
			static_cast<UINT>(graphics.GetScreenHeight()));

		//ブルーム
		graphics.shaderParameter3D_.bloomData2.intensity = 5;
		graphics.shaderParameter3D_.bloomData2.threshold = 1;

		//太陽
		graphics.shaderParameter3D_.lightDirection = { 0.626f,-0.55f,-0.533f,0 };
	}


#if defined(StageEdit)

#else

	//経路探査
	SeachGraph::Instance().InitSub(54, 53, 2, -4,
		1.0f, 0.08f, GameObjectManager::Instance().Find("pico"));

	//EnemyManagerにプレイヤー登録
	EnemyManager::Instance().RegisterPlayer(GameObjectManager::Instance().Find("pico"));
#endif
}

// 終了化
void SceneGame::Finalize()
{

}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
#if defined(StageEdit)

#else
	////1フレームは初期化のため待機
	////終了処理
	//if (EnemyManager::Instance().GetEnemyCount() <= 0 && gameStartFlag_)	//敵の数0の時
	//{
	//	GameObjectManager::Instance().AllRemove();
	//	SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTitle));
	//	gameEndFlag_ = true;
	//}
	//一回目通るときにスタートフラグをON
	if (!gameStartFlag_)
	{
		gameStartFlag_ = true;
	}
#endif

	GameObjectManager::Instance().Update(elapsedTime);
	
#if defined(StageEdit)
	
#else
	if (!gameEndFlag_)
		//経路探査
		SeachGraph::Instance().UpdatePath();
#endif

	//エフェクト更新処理
	EffectManager::Instance().Update(elapsedTime);

	ParticleComManager::Instance().Update(elapsedTime);
}

// 描画処理
void SceneGame::Render()
{
	if (gameEndFlag_)return;
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
	ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

	// 画面クリア＆レンダーターゲット設定
	FLOAT color[] = { 0.0f, 0.0f, 0.5f, 1.0f };	// RGBA(0.0〜1.0)
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
	postEff_->SkymapRender(mainCamera_);

	GameObjectManager::Instance().UpdateTransform();

	GameObjectManager::Instance().Render(mainCamera_->GetView(), mainCamera_->GetProjection());

	// デバッグレンダラ描画実行
	graphics.GetDebugRenderer()->Render(dc, mainCamera_->GetView(), mainCamera_->GetProjection());


	//バッファ戻す
	Graphics::Instance().RestoreRenderTargets();

	postEff_->Render(mainCamera_);
	postEff_->ImGuiRender();
	

	//3Dエフェクト描画
	{
		EffectManager::Instance().Render(mainCamera_->GetView(), mainCamera_->GetProjection());
	}

	// 3Dデバッグ描画
	{
		//経路探査
		if (1)
			SeachGraph::Instance().RenderPath();

		// ラインレンダラ描画実行
		if (1)
			graphics.GetLineRenderer()->Render(dc, mainCamera_->GetView(), mainCamera_->GetProjection());

		//ゲーム設定
		if (1)
		{	
			ImGui::SetNextWindowPos(ImVec2(950, 50), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

			if (ImGui::Begin("WorldSetting", nullptr, ImGuiWindowFlags_None))
			{
				//世界のスピード
				float worldSpeed = graphics.GetWorldSpeed();
				if (ImGui::DragFloat("worldSpeed", &worldSpeed, 0.01f, 0, 2))
					graphics.SetWorldSpeed(worldSpeed);
			}
			ImGui::End();
		}

		//敵マネージャーGUI
		if (1)
		{
			//敵マネージャー
			EnemyManager::Instance().OnGui();
		}

		//パーティクルマネージャー
		ParticleComManager::Instance().OnGui();
	}

	// 2Dスプライト描画
	{
	}


}

