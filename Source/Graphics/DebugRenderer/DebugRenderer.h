#pragma once

#include <vector>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

class DebugRenderer
{
public:
	DebugRenderer(ID3D11Device* device);
	~DebugRenderer() {}

public:
	// �`����s
	void Render(ID3D11DeviceContext* context, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

	// ���`��
	void DrawSphere(const DirectX::XMFLOAT3& center, float radius, const DirectX::XMFLOAT4& color);

	// ���`��
	void DrawBox(const DirectX::XMFLOAT3& center, DirectX::XMFLOAT3	scale, const DirectX::XMFLOAT4& color);

	// �~���`��
	void DrawCylinder(const DirectX::XMFLOAT3& position, float radius, float height, const DirectX::XMFLOAT4& color);

private:
	// �����b�V���쐬
	void CreateSphereMesh(ID3D11Device* device, float radius, int slices, int stacks);

	// �����b�V���쐬
	void CreateBoxMesh(ID3D11Device* device, DirectX::XMFLOAT3 scale);

	// �~�����b�V���쐬
	void CreateCylinderMesh(ID3D11Device* device, float radius1, float radius2, float start, float height, int slices, int stacks);

private:
	struct CbMesh
	{
		DirectX::XMFLOAT4X4	wvp;
		DirectX::XMFLOAT4	color;
	};

	struct Sphere
	{
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT3	center;
		float				radius;
	};

	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
	};
	struct Box
	{
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT3	center;
		DirectX::XMFLOAT3	scale;
	};

	struct Cylinder
	{
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT3	position;
		float				radius;
		float				height;
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer>			sphereVertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			boxVertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			boxIndexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			cylinderVertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			constantBuffer_;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>		vertexShader_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		pixelShader_;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>		inputLayout_;

	std::vector<Sphere>		spheres_;
	std::vector<Box>		boxes_;
	std::vector<Cylinder>	cylinders_;

	UINT	sphereVertexCount_ = 0;
	UINT	boxVertexCount_ = 0;
	UINT	boxIndicesCount_ = 0;
	UINT	cylinderVertexCount_ = 0;
};
