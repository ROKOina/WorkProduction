#include "ParticleSystemCom.h"

#include "TransformCom.h"
#include "Graphics/Graphics.h"
#include <imgui.h>

UINT align(UINT num, UINT alignment)
{
	return (num + (alignment - 1)) & ~(alignment - 1);
}

// 開始処理
void ParticleSystemCom::Start()
{
	particleData_.emitterPosition = GetGameObject()->transform_->GetWorldPosition();

	Graphics& graphics = Graphics::Instance();
	ID3D11Device* device = graphics.GetDevice();

	HRESULT hr{ S_OK };
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Particle) * maxParticleCount_);
	buffer_desc.StructureByteStride = sizeof(Particle);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	hr = device->CreateBuffer(&buffer_desc, NULL, particleBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc;
	shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shader_resource_view_desc.Buffer.ElementOffset = 0;
	shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(maxParticleCount_);
	hr = device->CreateShaderResourceView(particleBuffer_.Get(), &shader_resource_view_desc, particleSRV_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_UNORDERED_ACCESS_VIEW_DESC unordered_access_view_desc;
	unordered_access_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	unordered_access_view_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	unordered_access_view_desc.Buffer.FirstElement = 0;
	unordered_access_view_desc.Buffer.NumElements = static_cast<UINT>(maxParticleCount_);
	unordered_access_view_desc.Buffer.Flags = 0;
	hr = device->CreateUnorderedAccessView(particleBuffer_.Get(), &unordered_access_view_desc, particleUAV_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	Dx11StateLib* dx11State = graphics.GetDx11State().get();

	//画像読み込み
	D3D11_TEXTURE2D_DESC texture2d_desc{};
	dx11State->load_texture_from_file(device, L"Data/Sprite/color.png", particleSprite_.GetAddressOf(), &texture2d_desc);

	//定数バッファ
	dx11State->createConstantBuffer(device, sizeof(ParticleConstants), constantBuffer_.GetAddressOf());
	dx11State->createConstantBuffer(device, sizeof(ParticleScene), sceneBuffer_.GetAddressOf());

	//シェーダー初期化
	dx11State->createVsFromCso(device, "Shader\\ParticleVS.cso", particleVertex_.ReleaseAndGetAddressOf(), NULL, NULL, 0);
	dx11State->createPsFromCso(device, "Shader\\ParticlePS.cso", particlePixel_.ReleaseAndGetAddressOf());
	dx11State->createGsFromCso(device, "Shader\\ParticleGS.cso", particleGeometry_.ReleaseAndGetAddressOf());
	dx11State->createCsFromCso(device, "Shader\\ParticleCS.cso", particleCompute_.ReleaseAndGetAddressOf());
	dx11State->createCsFromCso(device, "Shader\\ParticleInitializeCS.cso", particleInitializerCompute_.ReleaseAndGetAddressOf());

	//パーティクル初期化
	Initialize();
}

// 更新処理
void ParticleSystemCom::Update(float elapsedTime)
{
	particleData_.emitterPosition = GetGameObject()->transform_->GetWorldPosition();

	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	dc->CSSetUnorderedAccessViews(0, 1, particleUAV_.GetAddressOf(), NULL);

	particleData_.time += elapsedTime;
	particleData_.elapsedTime = elapsedTime;
	dc->UpdateSubresource(constantBuffer_.Get(), 0, 0, &particleData_, 0, 0);
	dc->CSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());

	dc->CSSetShader(particleCompute_.Get(), NULL, 0);

	const UINT thread_group_count_x = align(static_cast<UINT>(maxParticleCount_), NUMTHREADS_X) / NUMTHREADS_X;
	dc->Dispatch(thread_group_count_x, 1, 1);

	ID3D11UnorderedAccessView* null_unordered_access_view{};
	dc->CSSetUnorderedAccessViews(0, 1, &null_unordered_access_view, NULL);
}

// GUI描画
void ParticleSystemCom::OnGUI()
{
	ImGui::DragFloat2("texSize", &particleData_.texSize.x, 0.01f);
	ImGui::DragFloat2("texPos", &particleData_.texPos.x, 0.01f);
}

void ParticleSystemCom::Initialize()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	dc->CSSetUnorderedAccessViews(0, 1, particleUAV_.GetAddressOf(), NULL);

	dc->UpdateSubresource(constantBuffer_.Get(), 0, 0, &particleData_, 0, 0);
	dc->CSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());

	dc->CSSetShader(particleInitializerCompute_.Get(), NULL, 0);

	const UINT thread_group_count_x = align(static_cast<UINT>(maxParticleCount_), NUMTHREADS_X) / NUMTHREADS_X;
	dc->Dispatch(thread_group_count_x, 1, 1);

	ID3D11UnorderedAccessView* null_unordered_access_view{};
	dc->CSSetUnorderedAccessViews(0, 1, &null_unordered_access_view, NULL);
}

void ParticleSystemCom::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	Dx11StateLib* dx11State = graphics.GetDx11State().get();

	ID3D11SamplerState* samplerState = dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_WRAP).Get();
	dc->PSSetSamplers(0, 1, &samplerState);

	dc->PSSetShaderResources(0, 1, particleSprite_.GetAddressOf());

	dc->VSSetShader(particleVertex_.Get(), NULL, 0);
	dc->PSSetShader(particlePixel_.Get(), NULL, 0);
	dc->GSSetShader(particleGeometry_.Get(), NULL, 0);
	dc->GSSetShaderResources(9, 1, particleSRV_.GetAddressOf());

	
	ParticleScene scene;

	scene.cameraPos = graphics.shaderParameter3D_.viewPosition;

	DirectX::XMStoreFloat4x4(&scene.viweProj, 
		DirectX::XMLoadFloat4x4(&graphics.shaderParameter3D_.view) 
		* DirectX::XMLoadFloat4x4(&graphics.shaderParameter3D_.projection));
	scene.lightDir = graphics.shaderParameter3D_.lightDirection;

	dc->UpdateSubresource(sceneBuffer_.Get(), 0, 0, &scene, 0, 0);
	dc->GSSetConstantBuffers(1, 1, sceneBuffer_.GetAddressOf());

	dc->UpdateSubresource(constantBuffer_.Get(), 0, 0, &particleData_, 0, 0);
	dc->VSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());
	dc->PSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());
	dc->GSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());

	dc->IASetInputLayout(NULL);
	dc->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	dc->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	dc->Draw(static_cast<UINT>(maxParticleCount_), 0);

	ID3D11ShaderResourceView* null_shader_resource_view{};
	dc->GSSetShaderResources(9, 1, &null_shader_resource_view);
	dc->VSSetShader(NULL, NULL, 0);
	dc->PSSetShader(NULL, NULL, 0);
	dc->GSSetShader(NULL, NULL, 0);
}