#include "SwordTrailCom.h"
#include "Graphics\Graphics.h"
#include <WICTextureLoader.h>

#include "Components\TransformCom.h"
#include "Components\RendererCom.h"

// 開始処理
void SwordTrailCom::Start()
{
    Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();
	ID3D11Device* device = Graphics::Instance().GetDevice();

	//頂点シェーダー
	{
		// 入力レイアウト
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		dx11State->createVsFromCso(device, "Shader\\SwordTrailVS.cso", vertexShader_.GetAddressOf(),
			layout_.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
	}

	// ピクセルシェーダー
	{
		dx11State->createPsFromCso(device, "Shader\\SwordTrailPS.cso", pixelShader_.GetAddressOf());
	}

	// 頂点バッファ
	{
		D3D11_BUFFER_DESC VB;
		VB.ByteWidth = sizeof(SwordTrailVertex) * (index_ + (index_ - 1) * (split_ - 1)) * 2;
		VB.Usage = D3D11_USAGE_DEFAULT;
		VB.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		VB.CPUAccessFlags = 0;
		VB.MiscFlags = 0;
		VB.StructureByteStride = 0;

		SwordTrailVertex v{};
		D3D11_SUBRESOURCE_DATA subResourceData;
		subResourceData.pSysMem = &v;
		subResourceData.SysMemPitch = 0;
		subResourceData.SysMemSlicePitch = 0;

		HRESULT hr = device->CreateBuffer(&VB, &subResourceData, vBuffer_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// 定数バッファ
	{
		dx11State->createConstantBuffer(device, sizeof(TrailBuffer), VPBuffer_.GetAddressOf());
	}

	//テクスチャバインド
	{
		HRESULT hr = DirectX::CreateWICTextureFromFile(device,
			L"Data\\Sprite\\trail.png",
			nullptr, texture_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	posArray_.resize(index_);
}

// 更新処理
void SwordTrailCom::Update(float elapsedTime)
{
	std::shared_ptr<RendererCom> rendererCom = GetGameObject()->GetComponent<RendererCom>();
	assert(rendererCom);
	assert(nodeName_[0].size() > 0 && nodeName_[1].size() > 0);

	//トレイル終了処理
	if (endTrail_)
	{
		//データを更新
		for (size_t i = posArray_.size() - 1; i > 0; --i)
		{
			posArray_[i] = posArray_[i - 1];
		}
		posArray_.front() = posArray_[0];

		DirectX::XMVECTOR HeadStart = DirectX::XMLoadFloat3(&posArray_[0].head);
		DirectX::XMVECTOR HeadEnd = DirectX::XMLoadFloat3(&posArray_[posArray_.size() - 1].head);
		float length = DirectX::XMVector3Length(DirectX::XMVectorSubtract(HeadStart, HeadEnd)).m128_f32[0];

		if (length < 0.1f)
		{
			SetEnabled(false);
		}
	}
	else
	{
		//データを更新
		for (size_t i = posArray_.size() - 1; i > 0; --i)
		{
			posArray_[i] = posArray_[i - 1];
		}
		//トレイルのポジションを登録
		PosBuffer tempPos;

		//末端のポジションを見つける
		Model::Node* node = rendererCom->GetModel()->FindNode(nodeName_[0].c_str());
		tempPos.tail = { node->worldTransform._41,node->worldTransform._42,node->worldTransform._43 };
		//先端のポジションを見つける
		node = rendererCom->GetModel()->FindNode(nodeName_[1].c_str());
		tempPos.head = { node->worldTransform._41,node->worldTransform._42,node->worldTransform._43 };

		posArray_.front() = tempPos;
	}

	//曲線を作る
	std::vector<PosBuffer> usedPosArray = posArray_;
	if (usedPosArray.empty())return;
	CreateCurveVertex(usedPosArray, split_);

	//頂点データを更新する
	float amount = 1.0f / (usedPosArray.size() - 1);
	float v = 0;
	vertex_.clear();
	vertex_.resize(usedPosArray.size() * 2);
	for (size_t i = 0, j = 0; i < vertex_.size() && j < usedPosArray.size(); i += 2, ++j)
	{
		vertex_[i].pos = usedPosArray[j].head;
		vertex_[i].uv = DirectX::XMFLOAT2(1.0f, v);
		vertex_[i + 1].pos = usedPosArray[j].tail;
		vertex_[i + 1].uv = DirectX::XMFLOAT2(0.0f, v);
		v += amount;
	}
}

// GUI描画
void SwordTrailCom::OnGUI()
{

}

//描画
void SwordTrailCom::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();

	//初期化
	dc->VSSetShader(vertexShader_.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader_.Get(), nullptr, 0);
	dc->IASetInputLayout(layout_.Get());

	const float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	dc->OMSetBlendState(dx11State->GetBlendState(Dx11StateLib::BLEND_STATE_TYPE::ALPHA).Get(), blend_factor, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(dx11State->GetDepthStencilState(Dx11StateLib::DEPTHSTENCIL_STATE_TYPE::DEPTH_ON_3D).Get(), 0);
	dc->RSSetState(dx11State->GetRasterizerState(Dx11StateLib::RASTERIZER_TYPE::FRONTCOUNTER_TRUE_CULLNONE).Get());

	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D11SamplerState* samplerStates[] =
	{
		 dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_TRAIL).Get(),
	};
	dc->PSSetSamplers(0, ARRAYSIZE(samplerStates), samplerStates);

	dc->PSSetShaderResources(0, 1, texture_.GetAddressOf());

	//コンスタントバッファ更新
	TrailBuffer t;
	t.trailColor = { 1,1,0,1 };
	DirectX::XMStoreFloat4x4(&t.viewProjection,
		DirectX::XMLoadFloat4x4(&graphics.shaderParameter3D_.view) *
		DirectX::XMLoadFloat4x4(&graphics.shaderParameter3D_.projection));

	dc->UpdateSubresource(VPBuffer_.Get(), 0, NULL, &t, 0, 0);
	ID3D11Buffer* constantBuffers[] =
	{
		VPBuffer_.Get()
	};
	dc->VSSetConstantBuffers(0, ARRAYSIZE(constantBuffers), constantBuffers);
	dc->PSSetConstantBuffers(0, ARRAYSIZE(constantBuffers), constantBuffers);



	//動的から静的に(頂点バッファ更新)
	SwordTrailVertex* vv = new SwordTrailVertex[vertex_.size()];

	for (int i = 0; i < vertex_.size(); ++i) {
		vv[i].pos = vertex_.at(i).pos;
		vv[i].uv = vertex_.at(i).uv;
	}

	dc->UpdateSubresource(vBuffer_.Get(), 0, 0, vv, 0, 0);
	delete vv;

	UINT strides = sizeof(SwordTrailVertex);
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, vBuffer_.GetAddressOf(), &strides, &offset);

	dc->Draw(static_cast<UINT>(vertex_.size()), 0);

	dc->VSSetShader(nullptr, nullptr, 0);
	dc->PSSetShader(nullptr, nullptr, 0);
	dc->IASetInputLayout(nullptr);
}

//曲線の頂点を作る
void SwordTrailCom::CreateCurveVertex(std::vector<PosBuffer>& usedPosArray, int split)
{
	if (usedPosArray.size() < 3 || split < 1) { return; }
	std::vector<PosBuffer> newPosArray;
	newPosArray.reserve(usedPosArray.size() + (usedPosArray.size() - 1) * split);
	const float amount = 1.0f / (split + 1);

	PosBuffer newPos;
	newPosArray.push_back(usedPosArray.front());
	for (size_t i = 0; i < usedPosArray.size() - 1; ++i)
	{
		float ratio = amount;
		//  CatMulに使う4つの点を作る（p0, p3がない時の処理も書く）
		DirectX::XMVECTOR p0Head = i == 0 ? DirectX::XMVectorScale(DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&usedPosArray[1].head)
			, DirectX::XMLoadFloat3(&usedPosArray[2].head)), 0.5f)
			: DirectX::XMLoadFloat3(&usedPosArray[i - 1].head);
		DirectX::XMVECTOR p1Head = DirectX::XMLoadFloat3(&usedPosArray[i].head);
		DirectX::XMVECTOR p2Head = DirectX::XMLoadFloat3(&usedPosArray[i + 1].head);
		DirectX::XMVECTOR p3Head = i == usedPosArray.size() - 2 ? DirectX::XMVectorScale(DirectX::XMVectorAdd(p0Head, p2Head), 0.5f)
			: DirectX::XMLoadFloat3(&usedPosArray[i + 2].head);

		DirectX::XMVECTOR p0Tail = i == 0 ? DirectX::XMVectorScale(DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&usedPosArray[1].tail)
			, DirectX::XMLoadFloat3(&usedPosArray[2].head)), 0.5f)
			: DirectX::XMLoadFloat3(&usedPosArray[i - 1].tail);
		DirectX::XMVECTOR p1Tail = DirectX::XMLoadFloat3(&usedPosArray[i].tail);
		DirectX::XMVECTOR p2Tail = DirectX::XMLoadFloat3(&usedPosArray[i + 1].tail);
		DirectX::XMVECTOR p3Tail = i == usedPosArray.size() - 2 ? DirectX::XMVectorScale(DirectX::XMVectorAdd(p0Tail, p2Tail), 0.5f)
			: DirectX::XMLoadFloat3(&usedPosArray[i + 2].tail);

		for (size_t j = 0; j < static_cast<size_t>(split - 1); ++j)
		{
			newPos = PosBuffer();

			DirectX::XMStoreFloat3(&newPos.head, DirectX::XMVectorCatmullRom(p0Head, p1Head, p2Head, p3Head, ratio));
			DirectX::XMStoreFloat3(&newPos.tail, DirectX::XMVectorCatmullRom(p0Tail, p1Tail, p2Tail, p3Tail, ratio));

			newPosArray.push_back(newPos);
			ratio += amount;
		}
		newPosArray.push_back(usedPosArray[i + 1]);
	}
	usedPosArray = newPosArray;
}

//トレイルの位置をリセットする
void SwordTrailCom::ResetNodePos()
{
	endTrail_ = false;

	std::shared_ptr<RendererCom> rendererCom = GetGameObject()->GetComponent<RendererCom>();
	assert(rendererCom);
	assert(nodeName_[0].size() > 0 && nodeName_[1].size() > 0);

	PosBuffer tempPos;
	//末端のポジションを見つける
	Model::Node* node = rendererCom->GetModel()->FindNode(nodeName_[0].c_str());
	tempPos.tail = { node->worldTransform._41,node->worldTransform._42,node->worldTransform._43 };
	//先端のポジションを見つける
	node = rendererCom->GetModel()->FindNode(nodeName_[1].c_str());
	tempPos.head = { node->worldTransform._41,node->worldTransform._42,node->worldTransform._43 };

	//指定したポジションに入れる
	for (int i = 0; i < posArray_.size(); ++i)
	{
		posArray_[i] = tempPos;
	}

}

void SwordTrailCom::EndTrail()
{
	endTrail_ = true;
}
