#include "Graphics/Graphics.h"
#include "Camera.h"
#include "EnemyManager.h"
#include "EnemySlime.h"
#include "EffectManager.h"
#include "Input\Input.h"

#include "StageManager.h"
#include "StageMain.h"
#include "StageMoveFloor.h"
#include "SceneGame.h"

// 初期化
void SceneGame::Initialize()
{

	//ステージ初期化
	StageManager& stageManager = StageManager::Instance();
	StageMain* stageMain = new StageMain();	//メイン（マップ）
	stageManager.Register(stageMain);
	StageMoveFloor* stageMoveFloor = new StageMoveFloor();	//オブジェクト（床）
	stageMoveFloor->SetStartPoint(DirectX::XMFLOAT3(0, 1, 3));
	stageMoveFloor->SetGoalPoint(DirectX::XMFLOAT3(10, 2, 3));
	stageMoveFloor->SetTorque(DirectX::XMFLOAT3(0, 1.0f, 0));
	stageManager.Register(stageMoveFloor);

	//プレイヤー初期化
	player = new Player();
	//カメラコントローラー初期化
	cameraController = new CameraController();

	//エネミー初期化
	//EnemyManager::Instance().Register(new EnemySlime());

	EnemyManager& enemyManager = EnemyManager::Instance();
	//EnemySlime* slime = new EnemySlime();
	//slime->SetPosition(DirectX::XMFLOAT3(0, 0, 5));
	//enemyManager.Register(slime);

#if 1

		EnemySlime* slime = new EnemySlime();
		slime->SetPosition(DirectX::XMFLOAT3(0, 0, 5));
		enemyManager.Register(slime);
#else
	//仮でシーンに敵を大量配置してみる
	for (int i = 0; i < 50; ++i)
	{
		EnemySlime* slime = new EnemySlime();
		slime->SetPosition(DirectX::XMFLOAT3(i * 2.0f, 0, 5));
		enemyManager.Register(slime);
	}
#endif

	//カメラ初期設定
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

	//ゲージスプライト
	gauge = new Sprite();
}

// 終了化
void SceneGame::Finalize()
{
	//エネミー終了化
	EnemyManager::Instance().Clear();
	//ステージ終了化
	//if (stage != nullptr)
	//{
	//	delete stage;
	//	stage = nullptr;
	//}
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
	//ゲージスプライト終了化
	if (gauge != nullptr)
	{
		delete gauge;
		gauge = nullptr;
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
	//stage->Update(elapsedTime);
	StageManager::Instance().Update(elapsedTime);
	//プレイヤー更新処理
	player->Update(elapsedTime);
	//エネミー更新処理
	EnemyManager::Instance().Upadate(elapsedTime);
	//エフェクト更新処理
	EffectManager::Instance().Update(elapsedTime);
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
	Camera& camera = Camera::Instance();
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();

	//// ビュー行列
	//{
	//	DirectX::XMFLOAT3 eye = { 0, 10, -10 };	// カメラの視点（位置）
	//	DirectX::XMFLOAT3 focus = { 0, 0, 0 };	// カメラの注視点（ターゲット）
	//	DirectX::XMFLOAT3 up = { 0, 1, 0 };		// カメラの上方向

	//	DirectX::XMVECTOR Eye = DirectX::XMLoadFloat3(&eye);
	//	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
	//	DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);
	//	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
	//	DirectX::XMStoreFloat4x4(&rc.view, View);
	//}
	//// プロジェクション行列
	//{
	//	float fovY = DirectX::XMConvertToRadians(45);	// 視野角
	//	float aspectRatio = graphics.GetScreenWidth() / graphics.GetScreenHeight();	// 画面縦横比率
	//	float nearZ = 0.1f;	// カメラが映し出すの最近距離
	//	float farZ = 1000.0f;	// カメラが映し出すの最遠距離
	//	DirectX::XMMATRIX Projection = DirectX::XMMatrixPerspectiveFovLH(fovY, aspectRatio, nearZ, farZ);	//プロジェクション行列作成
	//	DirectX::XMStoreFloat4x4(&rc.projection, Projection);	//rcに渡す
	//}

	// 3Dモデル描画
	{
		Shader* shader = graphics.GetShader();
		shader->Begin(dc, rc);	//シェーダーにカメラの情報を渡す

		//ステージ描画
		//stage->Render(dc, shader);
		StageManager::Instance().Render(dc, shader);
		//プレイヤー描画
		player->Render(dc, shader);
		//エネミー描画
		EnemyManager::Instance().Render(dc, shader);

		shader->End(dc);
	}

	//3Dエフェクト描画
	{
		EffectManager::Instance().Render(rc.view, rc.projection);
	}

	// 3Dデバッグ描画
	{
		//プレイヤーデバッグプリミティブ描画
		player->DrawDebugPrimitive();


		//エネミーデバッグプリミティブ描画
		EnemyManager::Instance().DrawDebugPrimitive();

		// ラインレンダラ描画実行
		graphics.GetLineRenderer()->Render(dc, rc.view, rc.projection);

		// デバッグレンダラ描画実行
		graphics.GetDebugRenderer()->Render(dc, rc.view, rc.projection);

	}

	// 2Dスプライト描画
	{
		RenderEnemyGauge(dc, rc.view, rc.projection);
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
				DirectX::XMFLOAT3 eye = camera.Instance().GetEye();
				DirectX::XMFLOAT3 focus = camera.Instance().GetFocus();
				ImGui::InputFloat3("Eye", &eye.x);
				ImGui::InputFloat3("Focus", &focus.x);
			}

			//カメラコントローラー
			cameraController->DrawDebugGUI();

			//エネミーマネージャー
			EnemyManager& enemyManager = EnemyManager::Instance();
			enemyManager.Instance().DrawDebugGUI();

		}
		ImGui::End();

	}

	////3Dモデル描画
	//{
	//	Shader* shader = graphics.GetShader();
	//	shader->Begin(dc, rc);

	//	//ステージ描画
	//	stage->Render(dc, shader);
	//	//プレイヤー描画
	//	player->Render(dc, shader);

	//	shader->End(dc);
	//}
}

//エネミーHPゲージ描画
void SceneGame::RenderEnemyGauge(
	ID3D11DeviceContext* dc,
	const DirectX::XMFLOAT4X4& view,
	const DirectX::XMFLOAT4X4& projection)
{
	//ビューポート
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);

	//変換行列
	DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&view);
	DirectX::XMMATRIX Projection = DirectX::XMLoadFloat4x4(&projection);
	DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
	
	//全ての敵の頭上にHPゲージを表示
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemuCount = enemyManager.GetEnemyCount();

	for (int i = 0; i < enemuCount; ++i)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);

		DirectX::XMFLOAT3 enemyPos = enemy->GetPosition();
		enemyPos.y += 1;	//エネミーの頭上をさす
		DirectX::XMVECTOR EnemyPos = DirectX::XMLoadFloat3(&enemyPos);

		//ワールド座標からスクリーン座標に変換
		EnemyPos = DirectX::XMVector3Project(
			EnemyPos,
			viewport.TopLeftX, viewport.TopLeftY,
			viewport.Width, viewport.Height,
			viewport.MinDepth, viewport.MaxDepth,
			Projection, View, World
		);
		DirectX::XMStoreFloat3(&enemyPos, EnemyPos);

		//2dスプライト描画
		gauge->Render(dc, enemyPos.x-20, enemyPos.y, 50/enemy->GetMaxHealth()*enemy->GetHealth(), 10,
			0, 0, 0, 0,
			0,
			1, 0, 0, 1);

		EnemyPos = DirectX::XMLoadFloat3(&enemyPos);

		//スクリーン座標からワールド座標に変換
		EnemyPos = DirectX::XMVector3Unproject(
			EnemyPos,
			viewport.TopLeftX, viewport.TopLeftY,
			viewport.Width, viewport.Height,
			viewport.MinDepth, viewport.MaxDepth,
			Projection, View, World
		);

	}


	//エネミー配置処理
	Mouse& mouse = Input::Instance().GetMouse();
	if (mouse.GetButtonDown() & Mouse::BTN_LEFT)
	{
		//マウスカーソル座標を取得
		DirectX::XMFLOAT3 screenPosition;
		screenPosition.x = static_cast<float>(mouse.GetPositionX());
		screenPosition.y = static_cast<float>(mouse.GetPositionY());

		//レイスタート
		screenPosition.z = viewport.MinDepth;
		DirectX::XMVECTOR ScreenPosS = DirectX::XMLoadFloat3(&screenPosition);
		//レイエンド
		screenPosition.z = viewport.MaxDepth;
		DirectX::XMVECTOR ScreenPosE = DirectX::XMLoadFloat3(&screenPosition);
		//スクリーン座標からワールド座標に変換
		ScreenPosS = DirectX::XMVector3Unproject(
			ScreenPosS,
			viewport.TopLeftX, viewport.TopLeftY,
			viewport.Width, viewport.Height,
			viewport.MinDepth, viewport.MaxDepth,
			Projection, View, World
		);
		ScreenPosE = DirectX::XMVector3Unproject(
			ScreenPosE,
			viewport.TopLeftX, viewport.TopLeftY,
			viewport.Width, viewport.Height,
			viewport.MinDepth, viewport.MaxDepth,
			Projection, View, World
		);
		DirectX::XMFLOAT3 screenPositionS;
		DirectX::XMStoreFloat3(&screenPositionS, ScreenPosS);
		DirectX::XMFLOAT3 screenPositionE;
		DirectX::XMStoreFloat3(&screenPositionE, ScreenPosE);

		//地形との当たり
		HitResult hit;
		if (StageManager::Instance().RayCast(screenPositionS, screenPositionE, hit))
		{
			if (EnemyManager::Instance().enemyLeftClickSet) {
				EnemySlime* enemy = new EnemySlime();
				enemy->SetPosition(hit.position);
				EnemyManager::Instance().Register(enemy);
			}
		}

	}

}
