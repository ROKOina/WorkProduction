#include <stdio.h> 
#include <WICTextureLoader.h>
#include "Sprite.h"
#include "Misc.h"
#include "Graphics/Graphics.h"

// �R���X�g���N�^
Sprite::Sprite()
	: Sprite(nullptr)
{
}

// �R���X�g���N�^
Sprite::Sprite(const char* filename)
{
	ID3D11Device* device = Graphics::Instance().GetDevice();

	HRESULT hr = S_OK;

	// ���_�f�[�^�̒�`
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

	// �|���S����`�悷��ɂ�GPU�ɒ��_�f�[�^��V�F�[�_�[�Ȃǂ̃f�[�^��n���K�v������B
	// GPU�Ƀf�[�^��n���ɂ�ID3D11***�̃I�u�W�F�N�g����ăf�[�^��n���܂��B

	// ���_�o�b�t�@�̐���
	{
		// ���_�o�b�t�@���쐬���邽�߂̐ݒ�I�v�V����
		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.ByteWidth = sizeof(vertices);	// �o�b�t�@�i�f�[�^���i�[������ꕨ�j�̃T�C�Y
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;	// UNIT.03
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// ���_�o�b�t�@�Ƃ��ăo�b�t�@���쐬����B
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// UNIT.03
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		// ���_�o�b�t�@�ɒ��_�f�[�^�����邽�߂̐ݒ�
		D3D11_SUBRESOURCE_DATA subresource_data = {};
		subresource_data.pSysMem = vertices;	// �����Ɋi�[���������_�f�[�^�̃A�h���X��n�����Ƃ�CreateBuffer()���Ƀf�[�^�����邱�Ƃ��ł���B
		subresource_data.SysMemPitch = 0; //Not use for vertex buffers.
		subresource_data.SysMemSlicePitch = 0; //Not use for vertex buffers.
		// ���_�o�b�t�@�I�u�W�F�N�g�̐���
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, &vertexBuffer_);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();

	// ���_�V�F�[�_�[
	{
		// ���̓��C�A�E�g
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		
		dx11State->createVsFromCso(device, "Shader\\SpriteVS.cso", vertexShader_.GetAddressOf(),
			inputLayout_.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
	}

	// �s�N�Z���V�F�[�_�[
	{
		dx11State->createPsFromCso(device, "Shader\\SpritePS.cso", pixelShader_.GetAddressOf());
	}

	// �e�N�X�`���̐���
	if (filename != nullptr)
	{
		// �}���`�o�C�g�������烏�C�h�����֕ϊ�
		wchar_t wfilename[256];
		::MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, 256);

		// �e�N�X�`���t�@�C���ǂݍ���
		// �e�N�X�`���ǂݍ���
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		HRESULT hr = DirectX::CreateWICTextureFromFile(device, wfilename, resource.GetAddressOf(), shaderResourceView_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		// �e�N�X�`�����̎擾
		D3D11_TEXTURE2D_DESC desc;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
		hr = resource->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		texture2d->GetDesc(&desc);

		textureWidth_ = desc.Width;
		textureHeight_ = desc.Height;
	}
	else
	{
		const int width = 8;
		const int height = 8;
		UINT pixels[width * height];
		::memset(pixels, 0xFF, sizeof(pixels));

		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA data;
		::memset(&data, 0, sizeof(data));
		data.pSysMem = pixels;
		data.SysMemPitch = width;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>	texture;
		HRESULT hr = device->CreateTexture2D(&desc, &data, texture.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		hr = device->CreateShaderResourceView(texture.Get(), nullptr, shaderResourceView_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		textureWidth_ = desc.Width;
		textureHeight_ = desc.Height;
	}
}

// �`����s
void Sprite::Render(ID3D11DeviceContext *immediate_context,
	float dx, float dy,
	float dw, float dh,
	float sx, float sy,
	float sw, float sh,
	float angle,
	float r, float g, float b, float a) const
{
	{
		// ���ݐݒ肳��Ă���r���[�|�[�g����X�N���[���T�C�Y���擾����B
		D3D11_VIEWPORT viewport;
		UINT numViewports = 1;
		immediate_context->RSGetViewports(&numViewports, &viewport);
		float screen_width = viewport.Width;
		float screen_height = viewport.Height;

		// �X�v���C�g���\������S���_�̃X�N���[�����W���v�Z����
		DirectX::XMFLOAT2 positions[] = {
			DirectX::XMFLOAT2(dx,      dy),			// ����
			DirectX::XMFLOAT2(dx + dw, dy),			// �E��
			DirectX::XMFLOAT2(dx,      dy + dh),	// ����
			DirectX::XMFLOAT2(dx + dw, dy + dh),	// �E��
		};

		// �X�v���C�g���\������S���_�̃e�N�X�`�����W���v�Z����
		DirectX::XMFLOAT2 texcoords[] = {
			DirectX::XMFLOAT2(sx,      sy),			// ����
			DirectX::XMFLOAT2(sx + sw, sy),			// �E��
			DirectX::XMFLOAT2(sx,      sy + sh),	// ����
			DirectX::XMFLOAT2(sx + sw, sy + sh),	// �E��
		};

		// �X�v���C�g�̒��S�ŉ�]�����邽�߂ɂS���_�̒��S�ʒu��
		// ���_(0, 0)�ɂȂ�悤�Ɉ�U���_���ړ�������B
		float mx = dx + dw * 0.5f;
		float my = dy + dh * 0.5f;
		for (auto& p : positions)
		{
			p.x -= mx;
			p.y -= my;
		}

		// ���_����]������
		const float PI = 3.141592653589793f;
		float theta = angle * (PI / 180.0f);	// �p�x�����W�A��(��)�ɕϊ�
		float c = cosf(theta);
		float s = sinf(theta);
		for (auto& p : positions)
		{
			DirectX::XMFLOAT2 r = p;
			p.x = c * r.x + -s * r.y;
			p.y = s * r.x + c * r.y;
		}

		// ��]�̂��߂Ɉړ����������_�����̈ʒu�ɖ߂�
		for (auto& p : positions)
		{
			p.x += mx;
			p.y += my;
		}

		// �X�N���[�����W�n����NDC���W�n�֕ϊ�����B
		for (auto& p : positions)
		{
			p.x = 2.0f*p.x / screen_width - 1.0f;
			p.y = 1.0f - 2.0f*p.y / screen_height;
		}

		// ���_�o�b�t�@�̓��e�̕ҏW���J�n����B
		D3D11_MAPPED_SUBRESOURCE mappedBuffer;
		HRESULT hr = immediate_context->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		// pData��ҏW���邱�ƂŒ��_�f�[�^�̓��e�����������邱�Ƃ��ł���B
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

		// ���_�o�b�t�@�̓��e�̕ҏW���I������B
		immediate_context->Unmap(vertexBuffer_.Get(), 0);
	}

	{
		// �p�C�v���C���ݒ�	
		Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		immediate_context->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
		immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		immediate_context->IASetInputLayout(inputLayout_.Get());

		immediate_context->VSSetShader(vertexShader_.Get(), nullptr, 0);
		immediate_context->PSSetShader(pixelShader_.Get(), nullptr, 0);

		immediate_context->PSSetShaderResources(0, 1, shaderResourceView_.GetAddressOf());

		const float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		
		immediate_context->OMSetBlendState(dx11State->GetBlendState(Dx11StateLib::BLEND_STATE_TYPE::ALPHA).Get()
			, blend_factor, 0xFFFFFFFF);
		immediate_context->OMSetDepthStencilState(dx11State->GetDepthStencilState(Dx11StateLib::DEPTHSTENCIL_STATE_TYPE::DEPTH_ON_2D).Get()
			, 0);
		immediate_context->RSSetState(dx11State->GetRasterizerState(Dx11StateLib::RASTERIZER_TYPE::FRONTCOUNTER_FALSE_CULLNONE).Get());
		immediate_context->PSSetSamplers(0, 1
			, dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_WRAP).GetAddressOf());


		// �`��
		immediate_context->Draw(4, 0);
	}
}

