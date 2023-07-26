#include <stdio.h>
#include <memory>
#include "Misc.h"
#include "DebugRenderer.h"
#include "Graphics/Graphics.h"

DebugRenderer::DebugRenderer(ID3D11Device* device)
{
	Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();

	// 頂点シェーダー
	{
		// 入力レイアウト
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		dx11State->createVsFromCso(device, "Shader\\DebugVS.cso", vertexShader_.GetAddressOf(),
			inputLayout_.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
	}

	// ピクセルシェーダー
	{
		dx11State->createPsFromCso(device, "Shader\\DebugPS.cso", pixelShader_.GetAddressOf());
	}

	// 定数バッファ
	{
		dx11State->createConstantBuffer(device, sizeof(CbMesh), constantBuffer_.GetAddressOf());
	}

	// 球メッシュ作成
	CreateSphereMesh(device, 1.0f, 16, 16);

	// 箱メッシュ作成
	CreateBoxMesh(device, { 1,1,1 });

	// 円柱メッシュ作成
	CreateCylinderMesh(device, 1.0f, 1.0f, 0.0f, 1.0f, 16, 1);
}

// 描画開始
void DebugRenderer::Render(ID3D11DeviceContext* context, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();

	// シェーダー設定
	context->VSSetShader(vertexShader_.Get(), nullptr, 0);
	context->PSSetShader(pixelShader_.Get(), nullptr, 0);
	context->IASetInputLayout(inputLayout_.Get());

	// 定数バッファ設定
	context->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
	//context->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());

	// レンダーステート設定
	const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	context->OMSetBlendState(dx11State->GetBlendState(Dx11StateLib::BLEND_STATE_TYPE::ALPHA).Get()
		, blendFactor, 0xFFFFFFFF);
	context->OMSetDepthStencilState(dx11State->GetDepthStencilState(Dx11StateLib::DEPTHSTENCIL_STATE_TYPE::DEPTH_ON_3D).Get()
		, 0);
	context->RSSetState(dx11State->GetRasterizerState(Dx11StateLib::RASTERIZER_TYPE::FRONTCOUNTER_FALSE_CULLNONE_WIREFRAME).Get());

	// ビュープロジェクション行列作成
	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&view);
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&projection);
	DirectX::XMMATRIX VP = V * P;

	// プリミティブ設定
	UINT stride = sizeof(DirectX::XMFLOAT3);
	UINT offset = 0;
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// 球描画
	context->IASetVertexBuffers(0, 1, sphereVertexBuffer_.GetAddressOf(), &stride, &offset);
	for (const Sphere& sphere : spheres_)
	{
		// ワールドビュープロジェクション行列作成
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(sphere.radius, sphere.radius, sphere.radius);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(sphere.center.x, sphere.center.y, sphere.center.z);
		DirectX::XMMATRIX W = S * T;
		DirectX::XMMATRIX WVP = W * VP;

		// 定数バッファ更新
		CbMesh cbMesh;
		cbMesh.color = sphere.color;
		DirectX::XMStoreFloat4x4(&cbMesh.wvp, WVP);

		context->UpdateSubresource(constantBuffer_.Get(), 0, 0, &cbMesh, 0, 0);
		context->Draw(sphereVertexCount_, 0);
	}
	spheres_.clear();

	//箱描画
	stride = sizeof(vertex);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers(0, 1, boxVertexBuffer_.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(boxIndexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
	for (const Box& box : boxes_)
	{
		// ワールドビュープロジェクション行列作成
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(box.scale.x, box.scale.y, box.scale.z);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(box.center.x, box.center.y, box.center.z);
		DirectX::XMMATRIX W = S * T;
		DirectX::XMMATRIX WVP = W * VP;

		// 定数バッファ更新
		CbMesh cbMesh;
		cbMesh.color = box.color;
		DirectX::XMStoreFloat4x4(&cbMesh.wvp, WVP);

		context->UpdateSubresource(constantBuffer_.Get(), 0, 0, &cbMesh, 0, 0);
		D3D11_BUFFER_DESC buffer_desc{};
		boxIndexBuffer_->GetDesc(&buffer_desc);
		context->DrawIndexed(buffer_desc.ByteWidth / sizeof(uint32_t), 0, 0);
	}
	boxes_.clear();

	// 円柱描画
	stride= sizeof(DirectX::XMFLOAT3);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	context->IASetVertexBuffers(0, 1, cylinderVertexBuffer_.GetAddressOf(), &stride, &offset);
	for (const Cylinder& cylinder : cylinders_)
	{
		// ワールドビュープロジェクション行列作成
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(cylinder.radius, cylinder.height, cylinder.radius);

		QuaternionStruct q;
		//向き
		DirectX::XMVECTOR P1 = DirectX::XMLoadFloat3(&cylinder.position1);
		DirectX::XMVECTOR P2 = DirectX::XMLoadFloat3(&cylinder.position2);
		DirectX::XMVECTOR P12 = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(P2, P1));
		DirectX::XMFLOAT3 dir;
		DirectX::XMStoreFloat3(&dir, P12);
		//上
		DirectX::XMVECTOR Right = { 1,0,0 };
		if (DirectX::XMVectorGetX(P12) * DirectX::XMVectorGetX(P12) - DirectX::XMVectorGetX(Right) * DirectX::XMVectorGetX(Right) < 0.001)
		{
			Right = { 0.9f,0.1f,0 };
		}
		DirectX::XMVECTOR Up = DirectX::XMVector3Cross(Right, P12);
		DirectX::XMFLOAT3 up;
		DirectX::XMStoreFloat3(&up, Up);		
		//上方向に伸びるのでupを前に
		q = q.LookRotation(up, dir);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&q.dxFloat4));

		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(cylinder.position1.x, cylinder.position1.y, cylinder.position1.z);
		DirectX::XMMATRIX W = S * R * T;
		DirectX::XMMATRIX WVP = W * VP;

		// 定数バッファ更新
		CbMesh cbMesh;
		cbMesh.color = cylinder.color;
		DirectX::XMStoreFloat4x4(&cbMesh.wvp, WVP);

		context->UpdateSubresource(constantBuffer_.Get(), 0, 0, &cbMesh, 0, 0);
		context->Draw(cylinderVertexCount_, 0);
	}
	cylinders_.clear();
}

// 球描画
void DebugRenderer::DrawSphere(const DirectX::XMFLOAT3& center, float radius, const DirectX::XMFLOAT4& color)
{
	Sphere sphere;
	sphere.center = center;
	sphere.radius = radius;
	sphere.color = color;
	spheres_.emplace_back(sphere);
}

// 箱描画
void DebugRenderer::DrawBox(const DirectX::XMFLOAT3& center, DirectX::XMFLOAT3	scale, const DirectX::XMFLOAT4& color)
{
	Box box;
	box.center = center;
	box.scale = scale;
	box.color = color;
	boxes_.emplace_back(box);
}

// 円柱描画
void DebugRenderer::DrawCylinder(const DirectX::XMFLOAT3& position1,const DirectX::XMFLOAT3& position2, float radius, float height, const DirectX::XMFLOAT4& color)
{
	Cylinder cylinder;
	cylinder.position1 = position1;
	cylinder.position2 = position2;
	cylinder.radius = radius;
	cylinder.height = height;
	cylinder.color = color;
	cylinders_.emplace_back(cylinder);
}

// 球メッシュ作成
void DebugRenderer::CreateSphereMesh(ID3D11Device* device, float radius, int slices, int stacks)
{
	sphereVertexCount_ = stacks * slices * 2 + slices * stacks * 2;
	std::unique_ptr<DirectX::XMFLOAT3[]> vertices = std::make_unique<DirectX::XMFLOAT3[]>(sphereVertexCount_);

	float phiStep = DirectX::XM_PI / stacks;
	float thetaStep = DirectX::XM_2PI / slices;

	DirectX::XMFLOAT3* p = vertices.get();
	
	for (int i = 0; i < stacks; ++i)
	{
		float phi = i * phiStep;
		float y = radius * cosf(phi);
		float r = radius * sinf(phi);

		for (int j = 0; j < slices; ++j)
		{
			float theta = j * thetaStep;
			p->x = r * sinf(theta);
			p->y = y;
			p->z = r * cosf(theta);
			p++;

			theta += thetaStep;

			p->x = r * sinf(theta);
			p->y = y;
			p->z = r * cosf(theta);
			p++;
		}
	}

	thetaStep = DirectX::XM_2PI / stacks;
	for (int i = 0; i < slices; ++i)
	{
		DirectX::XMMATRIX M = DirectX::XMMatrixRotationY(i * thetaStep);
		for (int j = 0; j < stacks; ++j)
		{
			float theta = j * thetaStep;
			DirectX::XMVECTOR V1 = DirectX::XMVectorSet(radius * sinf(theta), radius * cosf(theta), 0.0f, 1.0f);
			DirectX::XMVECTOR P1 = DirectX::XMVector3TransformCoord(V1, M);
			DirectX::XMStoreFloat3(p++, P1);

			int n = (j + 1) % stacks;
			theta += thetaStep;

			DirectX::XMVECTOR V2 = DirectX::XMVectorSet(radius * sinf(theta), radius * cosf(theta), 0.0f, 1.0f);
			DirectX::XMVECTOR P2 = DirectX::XMVector3TransformCoord(V2, M);
			DirectX::XMStoreFloat3(p++, P2);
		}
	}

	// 頂点バッファ
	{
		D3D11_BUFFER_DESC desc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		desc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * sphereVertexCount_);
		desc.Usage = D3D11_USAGE_IMMUTABLE;	// D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		subresourceData.pSysMem = vertices.get();
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;

		HRESULT hr = device->CreateBuffer(&desc, &subresourceData, sphereVertexBuffer_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
}

// 箱メッシュ作成
void DebugRenderer::CreateBoxMesh(ID3D11Device* device, DirectX::XMFLOAT3 scale)
{
	boxVertexCount_ = 24;
	boxIndicesCount_ = 36;

	vertex vertices[24]{
		{{-scale.x, -scale.y,  -scale.z} ,{ 0, 0,-1}}, // 0
		{{-scale.x,  scale.y,  -scale.z} ,{ 0, 0,-1}}, // 1
		{{ scale.x,  scale.y,  -scale.z} ,{ 0, 0,-1}}, // 2
		{{ scale.x, -scale.y,  -scale.z} ,{ 0, 0,-1}}, // 3  //手前向き
				    
		{{ scale.x, -scale.y,  scale.z} ,{ 0, 0, 1}}, // 4
		{{ scale.x,  scale.y,  scale.z} ,{ 0, 0, 1}}, // 5
		{{-scale.x,  scale.y,  scale.z} ,{ 0, 0, 1}}, // 6
		{{-scale.x, -scale.y,  scale.z} ,{ 0, 0, 1}}, // 7  //奥向き
				    
		{{ scale.x, -scale.y, -scale.z} ,{ 1, 0, 0}}, // 8
		{{ scale.x,  scale.y, -scale.z} ,{ 1, 0, 0}}, // 9
		{{ scale.x,  scale.y,  scale.z} ,{ 1, 0, 0}}, // 10
		{{ scale.x, -scale.y,  scale.z} ,{ 1, 0, 0}}, // 11  //右向き
				    
		{{-scale.x, -scale.y,  scale.z} ,{-1, 0, 0}}, // 12  
		{{-scale.x,  scale.y,  scale.z} ,{-1, 0, 0}}, // 13
		{{-scale.x,  scale.y, -scale.z} ,{-1, 0, 0}}, // 14
		{{-scale.x, -scale.y, -scale.z} ,{-1, 0, 0}}, // 15  //左向き
				    
		{{-scale.x,  scale.y, -scale.z} ,{ 0, 1, 0}}, // 16  
		{{-scale.x,  scale.y,  scale.z} ,{ 0, 1, 0}}, // 17
		{{ scale.x,  scale.y,  scale.z} ,{ 0, 1, 0}}, // 18
		{{ scale.x,  scale.y, -scale.z} ,{ 0, 1, 0}}, // 19  //上向き
				    
		{{-scale.x, -scale.y,  scale.z} ,{ 0,-1, 0}}, // 20
		{{-scale.x, -scale.y, -scale.z} ,{ 0,-1, 0}}, // 21
		{{ scale.x, -scale.y, -scale.z} ,{ 0,-1, 0}}, // 22
		{{ scale.x, -scale.y,  scale.z} ,{ 0,-1, 0}}, // 23  //下向き
	};
	uint32_t indices[36]{
	0,1,2,3,0,2,
	4,5,6,7,4,6,
	8,9,10,11,8,10,
	12,13,14,15,12,14,
	16,17,18,19,16,18,
	20,21,22,23,20,22,
	};

	// 頂点バッファ&インデックスバッファ
	{
		D3D11_BUFFER_DESC desc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		desc.ByteWidth = static_cast<UINT>(sizeof(vertex) * boxVertexCount_);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		subresourceData.pSysMem = vertices;
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;
		HRESULT hr = device->CreateBuffer(&desc, &subresourceData, boxVertexBuffer_.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * boxIndicesCount_);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		subresourceData.pSysMem = indices;
		hr = device->CreateBuffer(&desc, &subresourceData, boxIndexBuffer_.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
}


// 円柱メッシュ作成
void DebugRenderer::CreateCylinderMesh(ID3D11Device* device, float radius1, float radius2, float start, float height, int slices, int stacks)
{
	cylinderVertexCount_ = 2 * slices * (stacks + 1) + 2 * slices;
	std::unique_ptr<DirectX::XMFLOAT3[]> vertices = std::make_unique<DirectX::XMFLOAT3[]>(cylinderVertexCount_);

	DirectX::XMFLOAT3* p = vertices.get();

	float stackHeight = height / stacks;
	float radiusStep = (radius2 - radius1) / stacks;

	// vertices_ of ring
	float dTheta = DirectX::XM_2PI / slices;

	for (int i = 0; i < slices; ++i)
	{
		int n = (i + 1) % slices;

		float c1 = cosf(i * dTheta);
		float s1 = sinf(i * dTheta);

		float c2 = cosf(n * dTheta);
		float s2 = sinf(n * dTheta);

		for (int j = 0; j <= stacks; ++j)
		{
			float y = start + j * stackHeight;
			float r = radius1 + j * radiusStep;

			p->x = r * c1;
			p->y = y;
			p->z = r * s1;
			p++;

			p->x = r * c2;
			p->y = y;
			p->z = r * s2;
			p++;
		}

		p->x = radius1 * c1;
		p->y = start;
		p->z = radius1 * s1;
		p++;

		p->x = radius2 * c1;
		p->y = start + height;
		p->z = radius2 * s1;
		p++;
	}

	// 頂点バッファ
	{
		D3D11_BUFFER_DESC desc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		desc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * cylinderVertexCount_);
		desc.Usage = D3D11_USAGE_IMMUTABLE;	// D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		subresourceData.pSysMem = vertices.get();
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;

		HRESULT hr = device->CreateBuffer(&desc, &subresourceData, cylinderVertexBuffer_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
}
