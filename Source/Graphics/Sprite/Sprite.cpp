#include <stdio.h> 
#include <WICTextureLoader.h>
#include "Sprite.h"
#include "Misc.h"
#include "Graphics/Graphics.h"
#include <imgui.h>

// コンストラクタ
Sprite::Sprite()
	: Sprite(nullptr)
{
}

// コンストラクタ
Sprite::Sprite(const char* filename)
{
	ID3D11Device* device = Graphics::Instance().GetDevice();

	HRESULT hr = S_OK;

	// 頂点データの定義
	// 0           1
	// +-----------+
	// |           |
	// |           |
	// +-----------+
	// 2           3
	Vertex vertices[] = {
		{ DirectX::XMFLOAT3(-0.5, +0.5, 0), DirectX::XMFLOAT4(1, 1, 1, 1) },
		{ DirectX::XMFLOAT3(+0.5, +0.5, 0), DirectX::XMFLOAT4(1, 0, 0, 1) },
		{ DirectX::XMFLOAT3(-0.5, -0.5, 0), DirectX::XMFLOAT4(0, 1, 0, 1) },
		{ DirectX::XMFLOAT3(+0.5, -0.5, 0), DirectX::XMFLOAT4(0, 0, 1, 1) },
	};

	// ポリゴンを描画するにはGPUに頂点データやシェーダーなどのデータを渡す必要がある。
	// GPUにデータを渡すにはID3D11***のオブジェクトを介してデータを渡します。

	// 頂点バッファの生成
	{
		// 頂点バッファを作成するための設定オプション
		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.ByteWidth = sizeof(vertices);	// バッファ（データを格納する入れ物）のサイズ
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;	// UNIT.03
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// 頂点バッファとしてバッファを作成する。
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// UNIT.03
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		// 頂点バッファに頂点データを入れるための設定
		D3D11_SUBRESOURCE_DATA subresource_data = {};
		subresource_data.pSysMem = vertices;	// ここに格納したい頂点データのアドレスを渡すことでCreateBuffer()時にデータを入れることができる。
		subresource_data.SysMemPitch = 0; //Not use for vertex buffers.
		subresource_data.SysMemSlicePitch = 0; //Not use for vertex buffers.
		// 頂点バッファオブジェクトの生成
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, &vertexBuffer_);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();

	// 頂点シェーダー
	{
		// 入力レイアウト
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		
		dx11State->createVsFromCso(device, "Shader\\SpriteVS.cso", vertexShader_.GetAddressOf(),
			inputLayout_.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
	}

	// ピクセルシェーダー
	{
		dx11State->createPsFromCso(device, "Shader\\SpritePS.cso", pixelShader_.GetAddressOf());
	}

	// テクスチャの生成		
	D3D11_TEXTURE2D_DESC desc;
	dx11State->load_texture_from_file(device, filename, shaderResourceView_.GetAddressOf(), &desc);
	textureWidth_ = desc.Width;
	textureHeight_ = desc.Height;



	//効果初期化
	{
		dx11State->createConstantBuffer(device, sizeof(Effect2DConstans), effectBuffer_.GetAddressOf());
	}
}

// 描画実行
void Sprite::Render(ID3D11DeviceContext *immediate_context,
	float dx, float dy,
	float dw, float dh,
	float sx, float sy,
	float sw, float sh,
	float angle,
	float r, float g, float b, float a)
{
	{
		// 現在設定されているビューポートからスクリーンサイズを取得する。
		D3D11_VIEWPORT viewport;
		UINT numViewports = 1;
		immediate_context->RSGetViewports(&numViewports, &viewport);
		float screen_width = viewport.Width;
		float screen_height = viewport.Height;

		// スプライトを構成する４頂点のスクリーン座標を計算する
		DirectX::XMFLOAT2 positions[] = {
			DirectX::XMFLOAT2(dx,      dy),			// 左上
			DirectX::XMFLOAT2(dx + dw, dy),			// 右上
			DirectX::XMFLOAT2(dx,      dy + dh),	// 左下
			DirectX::XMFLOAT2(dx + dw, dy + dh),	// 右下
		};

		// スプライトを構成する４頂点のテクスチャ座標を計算する
		DirectX::XMFLOAT2 texcoords[] = {
			DirectX::XMFLOAT2(sx,      sy),			// 左上
			DirectX::XMFLOAT2(sx + sw, sy),			// 右上
			DirectX::XMFLOAT2(sx,      sy + sh),	// 左下
			DirectX::XMFLOAT2(sx + sw, sy + sh),	// 右下
		};

		// スプライトの中心で回転させるために４頂点の中心位置が
		// 原点(0, 0)になるように一旦頂点を移動させる。
		float mx = dx;
		float my = dy;
		for (auto& p : positions)
		{
			p.x -= mx;
			p.y -= my;
		}

		// 頂点を回転させる
		const float PI = 3.141592653589793f;
		float theta = angle * (PI / 180.0f);	// 角度をラジアン(θ)に変換
		float c = cosf(theta);
		float s = sinf(theta);
		for (auto& p : positions)
		{
			DirectX::XMFLOAT2 r = p;
			p.x = c * r.x + -s * r.y;
			p.y = s * r.x + c * r.y;
		}

		// 回転のために移動させた頂点を元の位置に戻す
		for (auto& p : positions)
		{
			p.x += mx;
			p.y += my;
		}

		// スクリーン座標系からNDC座標系へ変換する。
		for (auto& p : positions)
		{
			p.x = 2.0f*p.x / screen_width - 1.0f;
			p.y = 1.0f - 2.0f*p.y / screen_height;
		}

		// 頂点バッファの内容の編集を開始する。
		D3D11_MAPPED_SUBRESOURCE mappedBuffer;
		HRESULT hr = immediate_context->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		// pDataを編集することで頂点データの内容を書き換えることができる。
		Vertex* v = static_cast<Vertex*>(mappedBuffer.pData);
		for (int i = 0; i < 4; ++i)
		{
			v[i].position.x = positions[i].x;
			v[i].position.y = positions[i].y;
			v[i].position.z = 0.0f;

			v[i].color.x = r;
			v[i].color.y = g;
			v[i].color.z = b;
			v[i].color.w = a;

			v[i].texcoord.x = texcoords[i].x / textureWidth_;
			v[i].texcoord.y = texcoords[i].y / textureHeight_;
		}

		// 頂点バッファの内容の編集を終了する。
		immediate_context->Unmap(vertexBuffer_.Get(), 0);
	}

	{
		// パイプライン設定	
		Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		immediate_context->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
		immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		immediate_context->IASetInputLayout(inputLayout_.Get());

		immediate_context->VSSetShader(vertexShader_.Get(), nullptr, 0);
		immediate_context->PSSetShader(pixelShader_.Get(), nullptr, 0);

		//効果情報
		ID3D11Buffer* constantBuffers[] =
		{
			effectBuffer_.Get()
		};
		immediate_context->PSSetConstantBuffers(0, ARRAYSIZE(constantBuffers), constantBuffers);
		immediate_context->UpdateSubresource(effectBuffer_.Get(), 0, 0, &effectConstans_, 0, 0);


		immediate_context->PSSetShaderResources(0, 1, shaderResourceView_.GetAddressOf());
		immediate_context->PSSetShaderResources(1, 1, dissolveSRV_.GetAddressOf());

		const float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		
		immediate_context->OMSetBlendState(dx11State->GetBlendState(Dx11StateLib::BLEND_STATE_TYPE::ALPHA).Get()
			, blend_factor, 0xFFFFFFFF);
		immediate_context->OMSetDepthStencilState(dx11State->GetDepthStencilState(Dx11StateLib::DEPTHSTENCIL_STATE_TYPE::DEPTH_ON_2D).Get()
			, 0);
		immediate_context->RSSetState(dx11State->GetRasterizerState(Dx11StateLib::RASTERIZER_TYPE::FRONTCOUNTER_FALSE_CULLNONE).Get());
		immediate_context->PSSetSamplers(0, 1
			, dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_WRAP).GetAddressOf());


		// 描画
		immediate_context->Draw(4, 0);
	}
}

void Sprite::SetShaderResourceView(ID3D11ShaderResourceView* srv, float textureWidth, float textureHeight)
{
	shaderResourceView_.Reset();
	shaderResourceView_ = srv;
	textureWidth_ = static_cast<int>(textureWidth);
	textureHeight_ = static_cast<int>(textureHeight);
}

void Sprite::SetDissolveSRV(const char* filename)
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();

	D3D11_TEXTURE2D_DESC desc;
	dissolveSRV_.Reset();
	dx11State->load_texture_from_file(device, filename, dissolveSRV_.GetAddressOf(), &desc);
}

