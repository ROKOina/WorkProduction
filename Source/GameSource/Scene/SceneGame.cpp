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
#include "GameSource\ScriptComponents\Player\PlayerWeapon\CandyPushCom.h"
#include "GameSource\ScriptComponents\Enemy\EnemyNearCom.h"
#include "GameSource\ScriptComponents\Enemy\EnemyFarCom.h"
#include "GameSource\ScriptComponents\Enemy\EnemyAppleNearCom.h"
#include "GameSource\ScriptComponents\Enemy\EnemyGrapeFarCom.h"
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

		const char* filename = "Data/Model/stages/yuka/cookie1.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->GetModel()->SetMaterialColor({ 0.82f, 1.19f, 0.15f, 1 });
		r->SetShaderID(SHADER_ID::UnityChanToon);
		r->SetIsShadowDraw(true);
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
	for (int i = 0; i < 2; ++i)
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("picolaboNear");
		obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });

		obj->transform_->SetWorldPosition({ (rand() % (230 * 2) - 230) * 0.1f,
			0,  (rand() % (230 * 2) - 230) * 0.1f });
		//obj->transform_->SetWorldPosition({ 0, 0, 5 });

		obj->transform_->SetEulerRotation({ 0,180,0 });

		const char* filename = "Data/Model/picolabo/picolabo.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::UnityChanToon);
		r->SetIsShadowFall(true);

		std::shared_ptr<MovementCom> m = obj->AddComponent<MovementCom>();
		std::shared_ptr<CharacterStatusCom> status = obj->AddComponent<CharacterStatusCom>();

		std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();

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
			justAttack->SetName("attackJust");
			std::shared_ptr<BoxColliderCom> justCol = justAttack->AddComponent<BoxColliderCom>();
			justCol->SetMyTag(COLLIDER_TAG::JustAvoid);
			justCol->SetJudgeTag(COLLIDER_TAG::Player);
			justCol->SetSize({ 1.3f,1,1.3f });

			justAttack->transform_->SetLocalPosition({ -1.569f ,0 ,95.493f });
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
			weapon->SetNodeParent(sword->GetParent());
			weapon->SetNodeName("RightHand");
			weapon->SetColliderUpDown({ 1.36f,0 });
			weapon->SetIsForeverUse(true);
		}

		//パーティクルを子に
		{
			std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect(ParticleComManager::DAMAGE_SWETTS_ENEMY, { 0,0,0 }, obj);
			particle->transform_->SetScale(DirectX::XMFLOAT3(100, 100, 100));
			particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
		}
	}

	//enemyFar
	for (int i = 0; i < 0; ++i)
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("picolaboFar");
		obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });
		obj->transform_->SetWorldPosition({ 3, 0, 5 });
		obj->transform_->SetEulerRotation({ 0,180,0 });

		const char* filename = "Data/Model/picolabo/picolabo.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::UnityChanToon);
		r->SetIsShadowFall(true);


		std::shared_ptr<MovementCom> m = obj->AddComponent<MovementCom>();
		std::shared_ptr<CharacterStatusCom> status = obj->AddComponent<CharacterStatusCom>();

		std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();

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
			justAttack->SetName("attackJust");
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
			weapon->SetNodeParent(sword->GetParent());
			weapon->SetNodeName("RightHand");
			weapon->SetColliderUpDown({ 1.36f,0 });
			weapon->SetIsForeverUse(true);
		}

		//パーティクルを子に
		{
			std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect(ParticleComManager::DAMAGE_SWETTS_ENEMY, { 0,0,0 }, obj);
			particle->transform_->SetScale(DirectX::XMFLOAT3(100, 100, 100));
			particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
		}
	}

	//enemyAppleNear
	for (int i = 0; i < 5; ++i)
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("appleNear");
		obj->transform_->SetScale({ 0.1f, 0.1f, 0.1f });

		obj->transform_->SetWorldPosition({ (rand() % (230 * 2) - 230) * 0.1f,
			0,  (rand() % (230 * 2) - 230) * 0.1f });
		//obj->transform_->SetWorldPosition({ 0, 0, 5 });

		obj->transform_->SetEulerRotation({ 0,180,0 });

		const char* filename = "Data/Model/enemies/apple/appleEnemy.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::UnityChanToon);
		r->SetIsShadowFall(true);

		std::shared_ptr<MovementCom> m = obj->AddComponent<MovementCom>();
		std::shared_ptr<CharacterStatusCom> status = obj->AddComponent<CharacterStatusCom>();

		std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();

		std::shared_ptr<AnimatorCom> animator = obj->AddComponent<AnimatorCom>();

		std::shared_ptr<BoxColliderCom> c = obj->AddComponent<BoxColliderCom>();
		c->SetMyTag(COLLIDER_TAG::Enemy);
		c->SetJudgeTag(COLLIDER_TAG::Player | COLLIDER_TAG::Wall);
		c->SetSize(DirectX::XMFLOAT3(0.5f, 1.2f, 0.5f));
		c->SetOffsetPosition(DirectX::XMFLOAT3(0, 0.9f, 0));

		std::shared_ptr<EnemyAppleNearCom> e = obj->AddComponent<EnemyAppleNearCom>();

		//ジャスト回避用
		{
			std::shared_ptr<GameObject> justAttack = obj->AddChildObject();
			justAttack->SetName("attackJust");
			std::shared_ptr<BoxColliderCom> justCol = justAttack->AddComponent<BoxColliderCom>();
			justCol->SetMyTag(COLLIDER_TAG::JustAvoid);
			justCol->SetJudgeTag(COLLIDER_TAG::Player);
			justCol->SetSize({ 1.3f,1,1.3f });

			justAttack->transform_->SetLocalPosition({ 0 ,5.5f ,11.8f });
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

		//攻撃用オブジェクト
		{
			std::shared_ptr<GameObject> attackObj = obj->AddChildObject();
			attackObj->SetName("attack");

			std::shared_ptr<SphereColliderCom> attackCol = attackObj->AddComponent<SphereColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::EnemyAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Player);
			attackCol->SetRadius(1);

			std::shared_ptr<WeaponCom> weapon = attackObj->AddComponent<WeaponCom>();
			weapon->SetObject(attackObj->GetParent());
			weapon->SetNodeParent(attackObj->GetParent());
			weapon->SetNodeName("mixamorig:Hips");
			weapon->SetIsParentAnimUse(true);
			weapon->SetIsForeverUse(true);
		}

		//ダメージパーティクルを子に
		{
			std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect(ParticleComManager::DAMAGE_SWETTS_ENEMY, { 0,0,0 }, obj);
			particle->transform_->SetScale(DirectX::XMFLOAT3(1, 1, 1));
			particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
		}
	}

	//EnemyGrapeFar
	for (int i = 0; i < 5; ++i)
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("grapeFar");
		obj->transform_->SetScale({ 0.05f, 0.05f, 0.05f });

		obj->transform_->SetWorldPosition({ (rand() % (230 * 2) - 230) * 0.1f,
			0,  (rand() % (230 * 2) - 230) * 0.1f });
		//obj->transform_->SetWorldPosition({ 0, 0, 5 });

		obj->transform_->SetEulerRotation({ 0,180,0 });

		const char* filename = "Data/Model/enemies/grape/grapeEnemy.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::UnityChanToon);
		r->SetIsShadowFall(true);

		std::shared_ptr<MovementCom> m = obj->AddComponent<MovementCom>();
		std::shared_ptr<CharacterStatusCom> status = obj->AddComponent<CharacterStatusCom>();

		std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();

		std::shared_ptr<AnimatorCom> animator = obj->AddComponent<AnimatorCom>();

		std::shared_ptr<BoxColliderCom> c = obj->AddComponent<BoxColliderCom>();
		c->SetMyTag(COLLIDER_TAG::Enemy);
		c->SetJudgeTag(COLLIDER_TAG::Player | COLLIDER_TAG::Wall);
		c->SetSize(DirectX::XMFLOAT3(0.5f, 1.2f, 0.5f));
		c->SetOffsetPosition(DirectX::XMFLOAT3(0, 0.9f, 0));

		std::shared_ptr<EnemyGrapeFarCom> e = obj->AddComponent<EnemyGrapeFarCom>();

		//ジャスト回避用
		{
			std::shared_ptr<GameObject> justAttack = obj->AddChildObject();
			justAttack->SetName("attackJust");
			std::shared_ptr<BoxColliderCom> justCol = justAttack->AddComponent<BoxColliderCom>();
			justCol->SetMyTag(COLLIDER_TAG::JustAvoid);
			justCol->SetJudgeTag(COLLIDER_TAG::Player);
			justCol->SetSize({ 1.3f,1,1.3f });

			justAttack->transform_->SetLocalPosition({ 0 ,5.5f ,11.8f });
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

		//攻撃用オブジェクト
		{
			std::shared_ptr<GameObject> attackObj = obj->AddChildObject();
			attackObj->SetName("attack");

			std::shared_ptr<SphereColliderCom> attackCol = attackObj->AddComponent<SphereColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::EnemyAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Player);
			attackCol->SetRadius(1);

			std::shared_ptr<WeaponCom> weapon = attackObj->AddComponent<WeaponCom>();
			weapon->SetObject(attackObj->GetParent());
			weapon->SetNodeParent(attackObj->GetParent());
			weapon->SetNodeName("mixamorig:Hips");
			weapon->SetIsParentAnimUse(true);
			weapon->SetIsForeverUse(true);
		}

		//ダメージパーティクルを子に
		{
			std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect(ParticleComManager::DAMAGE_SWETTS_ENEMY, { 0,0,0 }, obj);
			particle->transform_->SetScale(DirectX::XMFLOAT3(20, 20, 20));
			particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
		}
	}

	//グレープ玉デバッグ用
	if(0)
	{
		std::shared_ptr<GameObject> grapeBall = GameObjectManager::Instance().Create();
		grapeBall->SetName("grapeBall");

		grapeBall->transform_->SetScale(DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f));

		const char* filename = "Data/Model/enemies/grape/grapeBall.mdl";
		std::shared_ptr<RendererCom> r = grapeBall->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::UnityChanToon);
		r->SetIsShadowFall(true);

		grapeBall->AddComponent<GrapeBallCom>()->SetEnabled(false);

		//当たり判定
		{
			std::shared_ptr<GameObject> attack = grapeBall->AddChildObject();
			attack->SetName("attack");

			std::shared_ptr<SphereColliderCom> attackCol = attack->AddComponent<SphereColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::EnemyAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Player);
			attackCol->SetRadius(0.4f);

			std::shared_ptr<WeaponCom> weapon = attack->AddComponent<WeaponCom>();
			weapon->SetObject(grapeBall);
			weapon->SetNodeParent(grapeBall);
			weapon->SetIsForeverUse(true);
			weapon->SetAttackDefaultStatus(1, 0);
		}

		//ジャスト回避用
		{
			std::shared_ptr<GameObject> justAttack = grapeBall->AddChildObject();
			justAttack->SetName("attackJust");
			std::shared_ptr<BoxColliderCom> justCol = justAttack->AddComponent<BoxColliderCom>();
			justCol->SetMyTag(COLLIDER_TAG::JustAvoid);
			justCol->SetJudgeTag(COLLIDER_TAG::Player);
			justCol->SetSize({ 2.0f,1,2.0f });

			justAttack->transform_->SetLocalPosition({ 0 ,0 ,119.5f });
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
		r->SetSilhouetteFlag(true);
		r->SetIsShadowFall(true);

		std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();

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



		//剣(Candy)
		{
			std::shared_ptr<GameObject> sword = obj->AddChildObject();
			sword->SetName("Candy");
			sword->transform_->SetScale(DirectX::XMFLOAT3(2, 2, 2));
			sword->transform_->SetEulerRotation(DirectX::XMFLOAT3(-154, -85, 82));


			const char* filename = "Data/Model/Swords/Candy/Candy.mdl";
			std::shared_ptr<RendererCom> r = sword->AddComponent<RendererCom>();
			r->LoadModel(filename);
			r->SetShaderID(SHADER_ID::UnityChanToon);
			r->SetEnabled(false);

			std::shared_ptr<CapsuleColliderCom> attackCol = sword->AddComponent<CapsuleColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::PlayerAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Enemy);
			attackCol->SetRadius(0.19f);
			attackCol->SetEnabled(false);

			std::shared_ptr<WeaponCom> weapon = sword->AddComponent<WeaponCom>();
			weapon->SetObject(sword->GetParent());
			weapon->SetNodeParent(sword->GetParent());
			weapon->SetNodeName("RightHandMiddle2");
			weapon->SetColliderUpDown({ 2,0 });

			std::shared_ptr<SwordTrailCom>  trail = sword->AddComponent<SwordTrailCom>();
			trail->SetEnabled(false);
			trail->SetHeadTailNodeName("candy", "head");	//トレイル表示ノード指定

			{
				//パーティクルを子に
				std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect(ParticleComManager::SWORD_SWEETS, { 0,0,0 }, sword);
				particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
			}

		}
		//剣(CandyCircle)
		{
			std::shared_ptr<GameObject> sword = obj->AddChildObject();
			sword->SetName("CandyCircle");
			sword->transform_->SetEulerRotation(DirectX::XMFLOAT3(-63, 0, 0));
			sword->transform_->SetScale(DirectX::XMFLOAT3(0.2f, 0.2f, 0.2f));

			const char* filename = "Data/Model/Swords/CandyCircle/candyCircle.mdl";
			std::shared_ptr<RendererCom> r = sword->AddComponent<RendererCom>();
			r->LoadModel(filename);
			r->SetShaderID(SHADER_ID::UnityChanToon);
			r->SetEnabled(false);

			std::shared_ptr<CapsuleColliderCom> attackCol = sword->AddComponent<CapsuleColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::PlayerAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Enemy);
			attackCol->SetRadius(0.19f);
			attackCol->SetEnabled(false);

			std::shared_ptr<WeaponCom> weapon = sword->AddComponent<WeaponCom>();
			weapon->SetObject(sword->GetParent());
			weapon->SetNodeParent(sword->GetParent());
			weapon->SetNodeName("RightHandMiddle2");
			weapon->SetColliderUpDown({ 2,0 });

			std::shared_ptr<SwordTrailCom>  trail = sword->AddComponent<SwordTrailCom>();
			trail->SetEnabled(false);
			trail->SetHeadTailNodeName("candyCircle", "head");	//トレイル表示ノード指定

			{
				//パーティクルを子に
				std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect(ParticleComManager::SWORD_SWEETS, { 0,0,0 }, sword);
				particle->transform_->SetScale(DirectX::XMFLOAT3(10, 10, 10));
				particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
			}
		}
		//剣(CandyStick)
		{
			std::shared_ptr<GameObject> sword = obj->AddChildObject();
			sword->SetName("CandyStick");
			sword->transform_->SetEulerRotation(DirectX::XMFLOAT3(-63, 0, 0));

			const char* filename = "Data/Model/Swords/CandyStick/candyStick.mdl";
			std::shared_ptr<RendererCom> r = sword->AddComponent<RendererCom>();
			r->LoadModel(filename);
			r->SetShaderID(SHADER_ID::UnityChanToon);
			r->SetEnabled(false);

			std::shared_ptr<CapsuleColliderCom> attackCol = sword->AddComponent<CapsuleColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::PlayerAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Enemy);
			attackCol->SetRadius(0.19f);
			attackCol->SetEnabled(false);

			std::shared_ptr<WeaponCom> weapon = sword->AddComponent<WeaponCom>();
			weapon->SetObject(sword->GetParent());
			weapon->SetNodeParent(sword->GetParent());
			weapon->SetNodeName("RightHandMiddle2");
			weapon->SetColliderUpDown({ 2,0 });

			std::shared_ptr<SwordTrailCom>  trail = sword->AddComponent<SwordTrailCom>();
			trail->SetEnabled(false);
			trail->SetHeadTailNodeName("stick", "head");	//トレイル表示ノード指定

			{
				//パーティクルを子に
				std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect(ParticleComManager::SWORD_SWEETS, { 0,0,0 }, sword);
				particle->transform_->SetScale(DirectX::XMFLOAT3(2, 2, 2));
				particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
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

	//プレイヤーの手にパーティクル
	{
		std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect
		(ParticleComManager::COMBO_TAME, {0,0,0},nullptr
			,GameObjectManager::Instance().Find("pico"), "RightHand");
		particle->SetName("playerHandParticle");
		particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
	}

	//移動用剣
	{
		std::shared_ptr<GameObject> moveObj = GameObjectManager::Instance().Create();
		moveObj->SetName("Push");
		moveObj->transform_->SetScale(DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f));

		moveObj->AddComponent<PushWeaponCom>();

		{
			std::shared_ptr<GameObject> sword = moveObj->AddChildObject();
			sword->SetName("CandyPush");
			sword->transform_->SetScale(DirectX::XMFLOAT3(2, 2, 2));
			sword->transform_->SetEulerRotation(DirectX::XMFLOAT3(0, -37, -62));


			const char* filename = "Data/Model/Swords/Candy/Candy.mdl";
			std::shared_ptr<RendererCom> r = sword->AddComponent<RendererCom>();
			r->LoadModel(filename);
			r->SetShaderID(SHADER_ID::UnityChanToon);
			r->SetEnabled(false);

			//std::shared_ptr<CapsuleColliderCom> attackCol = sword->AddComponent<CapsuleColliderCom>();
			//attackCol->SetMyTag(COLLIDER_TAG::PlayerAttack);
			//attackCol->SetJudgeTag(COLLIDER_TAG::Enemy);
			//attackCol->SetRadius(0.19f);
			//attackCol->SetEnabled(false);

			std::shared_ptr<WeaponCom> weapon = sword->AddComponent<WeaponCom>();
			weapon->SetObject(GameObjectManager::Instance().Find("pico"));
			weapon->SetNodeParent(moveObj);
			weapon->SetNodeName("");

			//std::shared_ptr<SwordTrailCom>  trail= sword->AddComponent<SwordTrailCom>();
			//trail->SetHeadTailNodeName("candy", "head");	//トレイル表示ノード指定

			//{
			//	//パーティクルを子に
			//	std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect(ParticleComManager::SWORD_SWEETS, { 0,0,0 }, sword);
			//	particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
			//}

		}
	}

	//ジャスト回避用パーティクル
	{
		std::shared_ptr<GameObject> particle = ParticleComManager::Instance().SetEffect(ParticleComManager::JUST_UNDER, { 0,0,0 });
		particle->SetName("justUnderParticle");
		particle->SetEnabled(false);
		particle->GetComponent<ParticleSystemCom>()->SetIsWorldSpeed(false);
	}


	////particle
	//for (int i = 0; i < 0; ++i)
	//{
	//	std::shared_ptr<GameObject> p = GameObjectManager::Instance().Create();
	//	std::string n = "Particle" + std::to_string(i);
	//	p->SetName(n.c_str());
	//	p->transform_->SetWorldPosition(DirectX::XMFLOAT3(i * 1.0f, 1, 0));

	//	std::shared_ptr<ParticleSystemCom> c = p->AddComponent<ParticleSystemCom>(100000);
	//	c->SetSweetsParticle(true);	//お菓子用

	//	c->LoadTexture("./Data/Sprite/sweetsParticle.png");

	//}

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

	//2Dマスク用カメラを生成
	{
		std::shared_ptr<GameObject> cameraMaskObj = GameObjectManager::Instance().Create();
		cameraMaskObj->SetName("MaskCamera");

		Graphics& graphics = Graphics::Instance();
		std::shared_ptr<CameraCom> c = cameraMaskObj->AddComponent<CameraCom>();
		c->SetPerspectiveFov(
			DirectX::XMConvertToRadians(45),
			graphics.GetScreenWidth() / graphics.GetScreenHeight(),
			0.1f, 1000.0f
		);
	}

	//マスク用プレイヤー
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("picoMask");
		obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });
		obj->transform_->SetWorldPosition({ 0.15f, -1.4f, 1.2f });
		obj->transform_->SetEulerRotation({ 0, 180, 0 });

		const char* filename = "Data/Model/pico/picoAnim.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::MaskUnityChan);

		std::shared_ptr<AnimationCom> a = obj->AddComponent<AnimationCom>();
	}



	//メインカメラ設定
	std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
	mainCamera_ = camera;

	//ポストエフェクト
	{
		Graphics& graphics = Graphics::Instance();
		postEff_ = std::make_unique<PostEffect>(
			static_cast<UINT>(graphics.GetScreenWidth()),
			static_cast<UINT>(graphics.GetScreenHeight()));

		//ブルーム
		graphics.shaderParameter3D_.bloomLuminance.intensity = 5;
		graphics.shaderParameter3D_.bloomLuminance.threshold = 1;

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

	//遷移処理
	startSprite_ = std::make_unique<Sprite>("./Data/Sprite/START.png");
	std::shared_ptr<ParticleSystemCom> particle = SceneManager::Instance().GetParticleObj()->GetComponent<ParticleSystemCom>();
	sceneTransitionTimer_ = 3;	//パーティクル後の演出開始時間
	startSpriteSize_ = 0;	//開始画像のサイズ
	particle->GetSaveParticleData().particleData.isRoop = false;
	SceneManager::Instance().SetParticleUpdate(true);
	GameObjectManager::Instance().SetIsSceneGameStart(false);

	gameStartFlag_ = false;
	gameEndFlag_ = false;
}

// 終了化
void SceneGame::Finalize()
{

}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	//遷移処理
	if (!GameObjectManager::Instance().GetIsSceneGameStart())
	{
		sceneTransitionTimer_ -= elapsedTime;
	}

#if defined(StageEdit)

#else
	//1フレームは初期化のため待機
	//終了処理
	if ((EnemyManager::Instance().GetEnemyCount() <= 0 || GameObjectManager::Instance().Find("pico")->GetComponent<CharacterStatusCom>()->GetHP() <= 0)
		&& gameStartFlag_ && !gameEndFlag_)	//敵の数0の時
	{
		gameEndFlag_ = true;

		std::shared_ptr<ParticleSystemCom> particle = SceneManager::Instance().GetParticleObj()->GetComponent<ParticleSystemCom>();
		particle->GetSaveParticleData().particleData.isRoop = true;
		particle->IsRestart();
		particle->SetEnabled(true);
		SceneManager::Instance().SetParticleUpdate(true);
		transitionOutTimer_ = 2.5f;
	}
	//終了演出
	if(gameEndFlag_)
	{
		transitionOutTimer_ -= elapsedTime;
		if (transitionOutTimer_ < 0)
		{
			GameObjectManager::Instance().AllRemove();
			SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTitle));
			SceneManager::Instance().SetParticleUpdate(false);
		}
	}

	//一回目通るときにスタートフラグをON
	if (!gameStartFlag_)
	{
		gameStartFlag_ = true;
	}
#endif

	GameObjectManager::Instance().Update(elapsedTime);

	if (gameEndFlag_)return;

	EnemyManager::Instance().Update(elapsedTime);

#if defined(StageEdit)
	
#else
	
		//経路探査
	SeachGraph::Instance().UpdatePath();
#endif

	//エフェクト更新処理
	EffectManager::Instance().Update(elapsedTime);

	ParticleComManager::Instance().Update(elapsedTime);
}

// 描画処理
void SceneGame::Render(float elapsedTime)
{
	if (gameEndFlag_)return;
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
	ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

	// 画面クリア＆レンダーターゲット設定
	FLOAT color[] = { 0.0f, 0.0f, 0.5f, 1.0f };	// RGBA(0.0～1.0)
	dc->OMSetRenderTargets(1, &rtv, dsv);

	// 描画処理
	ShaderParameter3D& rc = graphics.shaderParameter3D_;
	rc.lightDirection = graphics.shaderParameter3D_.lightDirection;	// ライト方向

	//カメラパラメーター設定
	rc.view = mainCamera_->GetView();
	rc.projection = mainCamera_->GetProjection();
	DirectX::XMFLOAT3 cameraPos = mainCamera_->GetGameObject()->transform_->GetWorldPosition();
	rc.viewPosition = { cameraPos.x,cameraPos.y,cameraPos.z,1 };

	//影設定   
	std::shared_ptr<GameObject> pico = GameObjectManager::Instance().Find("pico");
	DirectX::XMFLOAT3  pPos = pico->transform_->GetWorldPosition();
	rc.shadowMapData.shadowCameraPos = { pPos.x,pPos.y,pPos.z,0 };
	rc.shadowMapData.shadowRect = 50;

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
	if (graphics.IsDebugGUI())
		postEff_->ImGuiRender();
	

	//3Dエフェクト描画
	{
		EffectManager::Instance().Render(mainCamera_->GetView(), mainCamera_->GetProjection());
	}

	// 3Dデバッグ描画
	if(graphics.IsDebugGUI())
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
		//プレイヤージャスト回避演出
		std::shared_ptr<PlayerCom> pico = GameObjectManager::Instance().Find("pico")->GetComponent<PlayerCom>();
		pico->GetJustAvoidPlayer()->justDirectionRender2D();

		GameObjectManager::Instance().Render2D(elapsedTime);
		EnemyManager::Instance().Render2D(elapsedTime);
	}


	//マスク用
	{
		//マスク処理
		{
			std::shared_ptr<CameraCom> maskCamera = GameObjectManager::Instance().Find("MaskCamera")->GetComponent<CameraCom>();

			EnemyManager::Instance().EnemyMaskRender(postEff_.get(), maskCamera);

			GameObjectManager::Instance().Find("pico")->GetComponent<PlayerCom>()->MaskRender(postEff_.get(), maskCamera);

			if (graphics.IsDebugGUI())
				postEff_->DrawMaskGui();
		}
	}

	//遷移処理
	if (!GameObjectManager::Instance().GetIsSceneGameStart())
	{
		//スタート画像
		if (sceneTransitionTimer_ < 0)
		{
			startSpriteSize_ += elapsedTime;
			float sinSize = sin(startSpriteSize_);
			if (sinSize <= 0)
			{
				GameObjectManager::Instance().SetIsSceneGameStart(true);
				std::shared_ptr<ParticleSystemCom> particle = SceneManager::Instance().GetParticleObj()->GetComponent<ParticleSystemCom>();
				particle->SetEnabled(false);
				SceneManager::Instance().SetParticleUpdate(false);
			}
			DirectX::XMFLOAT2 size{startSprite_->GetTextureWidth()* sinSize, startSprite_->GetTextureHeight()* sinSize};
			startSprite_->Render(dc, graphics.GetScreenWidth()/2- size.x / 2.0f, graphics.GetScreenHeight() / 2 - size.y / 2.0f, size.x, size.y
				, 0, 0, static_cast<float>(startSprite_->GetTextureWidth()), static_cast<float>(startSprite_->GetTextureHeight())
				, 0, 1, 1, 1, 1);
		}
	}
}

