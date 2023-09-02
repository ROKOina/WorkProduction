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
#include "GameSource\ScriptComponents\Enemy\EnemyCom.h"
#include "GameSource\ScriptComponents\Weapon\WeaponCom.h"
#include "GameSource\ScriptComponents\CharacterStatusCom.h"

// ������
void SceneGame::Initialize()
{
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("hanbai");
		obj->transform_->SetScale({ 0.1f, 0.1f, 0.1f });

		const char* filename = "Data/Model/stages/FloorSand.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::Phong);
	}

	//enemy
	//for(int i=0;i<50;++i)
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("picolabo");
		obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });
		obj->transform_->SetWorldPosition({ 3, 0, 5 });
		obj->transform_->SetEulerRotation({ 0,180,0 });

		const char* filename = "Data/Model/picolabo/picolabo.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::Phong);

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

		std::shared_ptr<EnemyCom> e = obj->AddComponent<EnemyCom>();

		//�U�������蔻��p
		{
			std::shared_ptr<GameObject> attack = obj->AddChildObject();
			attack->SetName("picolaboAttack");
			std::shared_ptr<SphereColliderCom> attackCol = attack->AddComponent<SphereColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::EnemyAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Player);

		}
		//�W���X�g���p
		{
			std::shared_ptr<GameObject> justAttack = obj->AddChildObject();
			justAttack->SetName("picolaboAttackJust");
			std::shared_ptr<BoxColliderCom> justCol = justAttack->AddComponent<BoxColliderCom>();
			justCol->SetMyTag(COLLIDER_TAG::JustAvoid);
			justCol->SetSize({ 1.3f,1,1.3f });

			justAttack->transform_->SetLocalPosition({ -1.569f ,0,95.493f });
		}
	}

	//�v���C���[
	{
		std::shared_ptr<GameObject> obj = GameObjectManager::Instance().Create();
		obj->SetName("pico");
		obj->transform_->SetScale({ 0.01f, 0.01f, 0.01f });

		const char* filename = "Data/Model/pico/picoAnim.mdl";
		std::shared_ptr<RendererCom> r = obj->AddComponent<RendererCom>();
		r->LoadModel(filename);
		r->SetShaderID(SHADER_ID::Phong);

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

		//�U���␳���������蔻��
		{
			std::shared_ptr<GameObject> attackAssist = obj->AddChildObject();
			attackAssist->SetName("attackAssist");
			std::shared_ptr<SphereColliderCom> attackAssistCol = attackAssist->AddComponent<SphereColliderCom>();
			attackAssistCol->SetMyTag(COLLIDER_TAG::PlayerAttackAssist);
			attackAssistCol->SetJudgeTag(COLLIDER_TAG::Enemy);
			attackAssistCol->SetRadius(3);
		}

		//�f��U�������蔻��p
		{
			std::shared_ptr<GameObject> attack = obj->AddChildObject();
			attack->SetName("picoAttack");
			std::shared_ptr<SphereColliderCom> attackCol = attack->AddComponent<SphereColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::PlayerAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Enemy);

		}

		//��("RightHandMiddle2")
		{
			std::shared_ptr<GameObject> sword = obj->AddChildObject();
			sword->SetName("greatSword");

			const char* filename = "Data/Model/Swords/GreatSword/GreatSword.mdl";
			std::shared_ptr<RendererCom> r = sword->AddComponent<RendererCom>();
			r->LoadModel(filename);
			r->SetShaderID(SHADER_ID::Phong);

			std::shared_ptr<CapsuleColliderCom> attackCol = sword->AddComponent<CapsuleColliderCom>();
			attackCol->SetMyTag(COLLIDER_TAG::PlayerAttack);
			attackCol->SetJudgeTag(COLLIDER_TAG::Enemy);

			std::shared_ptr<WeaponCom> weapon = sword->AddComponent<WeaponCom>();
			weapon->SetObject(sword->GetParent());
			weapon->SetNodeName("RightHandMiddle2");
		}
	}

	//�J�����𐶐�
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

	//���C���J�����ݒ�
	std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
	mainCamera_ = camera;

	////�X�e�[�W������
	//StageManager& stageManager = StageManager::Instance();
	//StageMain* stageMain = new StageMain();	//���C���i�}�b�v�j
	//stageManager.Register(stageMain);

	//�|�X�g�G�t�F�N�g
	{
		Graphics& graphics = Graphics::Instance();
		postEff = std::make_unique<PostEffect>(
			static_cast<UINT>(graphics.GetScreenWidth()) ,
			static_cast<UINT>(graphics.GetScreenHeight()));
	}

	//particle_ = std::make_unique<Particle>(DirectX::XMFLOAT4{ player->GetLocalPosition().x,player->GetLocalPosition().y,player->GetLocalPosition().z,0 });
}

// �I����
void SceneGame::Finalize()
{
	//StageManager::Instance().Clear();
}

// �X�V����
void SceneGame::Update(float elapsedTime)
{
	GameObjectManager::Instance().Update(elapsedTime);

	////�X�e�[�W�X�V����
	//StageManager::Instance().Update(elapsedTime);
	//�G�t�F�N�g�X�V����
	EffectManager::Instance().Update(elapsedTime);

	Graphics& graphics = Graphics::Instance();


	//particle_->integrate(elapsedTime, { player->GetLocalPosition().x,player->GetLocalPosition().y,player->GetLocalPosition().z,0 }, camera->GetView(), camera->GetProjection());
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
	ShaderParameter3D& rc = graphics.shaderParameter3D_;
	rc.lightDirection = graphics.shaderParameter3D_.lightDirection;	// ���C�g����

	//�J�����p�����[�^�[�ݒ�
	rc.view = mainCamera_->GetView();
	rc.projection = mainCamera_->GetProjection();
	DirectX::XMFLOAT3 cameraPos = mainCamera_->GetGameObject()->transform_->GetWorldPosition();
	rc.viewPosition = { cameraPos.x,cameraPos.y,cameraPos.z,1 };


	//�o�b�t�@���
	Graphics::Instance().CacheRenderTargets();

	//�|�X�g�G�t�F�N�g�p�؂�ւ�
	PostRenderTarget* ps = Graphics::Instance().GetPostEffectModelRenderTarget().get();
	PostDepthStencil* ds = Graphics::Instance().GetPostEffectModelDepthStencilView().get();

	// ��ʃN���A�������_�[�^�[�Q�b�g�ݒ�
	rtv = {};
	dsv = {};
	rtv = ps->renderTargetView.Get();
	dsv = ds->depthStencilView.Get();
	dc->ClearRenderTargetView(rtv, color);
	dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	dc->OMSetRenderTargets(1, &rtv, dsv);

	// �r���[�|�[�g�̐ݒ�
	D3D11_VIEWPORT	vp = {};
	vp.Width = static_cast<float>(ps->width);
	vp.Height = static_cast<float>(ps->height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	dc->RSSetViewports(1, &vp);


	GameObjectManager::Instance().UpdateTransform();
	GameObjectManager::Instance().Render(mainCamera_->GetView(), mainCamera_->GetProjection());

	// �f�o�b�O�����_���`����s
	graphics.GetDebugRenderer()->Render(dc, mainCamera_->GetView(), mainCamera_->GetProjection());



	//// 3D���f���`��
	//{
	//	Shader* shader = graphics.GetShader(SHADER_ID::Phong);
	//	shader->Begin(dc, rc);	//�V�F�[�_�[�ɃJ�����̏���n��

	//	////�X�e�[�W�`��
	//	//StageManager::Instance().Render(dc, shader);
	//	////�v���C���[�`��
	//	//player->Render(dc, shader_);

	//	shader->End(dc);
	//}

	//�o�b�t�@�߂�
	Graphics::Instance().RestoreRenderTargets();

	postEff->Render();
	postEff->ImGuiRender();

	////�p�[�e�B�N��
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

	//	particle_->Render(shaderParameter3D_);

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

	//3D�G�t�F�N�g�`��
	{
		EffectManager::Instance().Render(mainCamera_->GetView(), mainCamera_->GetProjection());
	}

	// 3D�f�o�b�O�`��
	{
		// ���C�������_���`����s
		graphics.GetLineRenderer()->Render(dc, mainCamera_->GetView(), mainCamera_->GetProjection());

		ImGui::SetNextWindowPos(ImVec2(950, 50), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		ImGui::Begin("WorldSetting", nullptr, ImGuiWindowFlags_None);

		//���E�̃X�s�[�h
		float worldSpeed = graphics.GetWorldSpeed();
		ImGui::DragFloat("worldSpeed", &worldSpeed, 0.01f, 0, 2);
		graphics.SetWorldSpeed(worldSpeed);

		ImGui::End();

	}

	// 2D�X�v���C�g�`��
	{
	}


}

