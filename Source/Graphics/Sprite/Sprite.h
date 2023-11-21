#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

// スプライト
class Sprite
{
public:
	Sprite();
	Sprite(const char* filename);
	~Sprite() {}

	struct Vertex
	{
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT2	texcoord;
	};

	// 描画実行
	void Render(ID3D11DeviceContext *dc,
		float dx, float dy,
		float dw, float dh,
		float sx, float sy,
		float sw, float sh,
		float angle,
		float r, float g, float b, float a);

	// テクスチャ幅取得
	int GetTextureWidth() const { return textureWidth_; }

	// テクスチャ高さ取得
	int GetTextureHeight() const { return textureHeight_; }

	//テクスチャ変更
	void SetShaderResourceView(ID3D11ShaderResourceView* srv,float textureWidth,float textureHeight);

	//ディゾルブ用画像登録
	void SetDissolveSRV(const char* filename);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexShader_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelShader_;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			inputLayout_;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBuffer_;


	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView_;

public:
	//効果一覧
	struct DissolveConstans
	{
		float isDissolve = false;

		float dissolveThreshold= 0.5f;	// ディゾルブ
		float edgeThreshold=0.5f; 		// ふちの閾値
		float dummy;
		DirectX::XMFLOAT4	edgeColor{1,0,0,1};			// ふちの色
	};
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	dissolveSRV_;

	//効果まとめる
	struct Effect2DConstans
	{
		DissolveConstans dissolveConstant;
	};
	Effect2DConstans effectConstans_;

	Effect2DConstans& GetEffectSpriteData() { return effectConstans_; }

private:
	//効果を掛ける時に使用するバッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer>				effectBuffer_;

	int textureWidth_ = 0;
	int textureHeight_ = 0;
};