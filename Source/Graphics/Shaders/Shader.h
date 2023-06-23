#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "3D\ShaderParameter3D.h"
#include "Graphics/Model/Model.h"


class Shader
{
public:
	Shader() {}
	virtual ~Shader() {}

	// �`��J�n
	virtual void Begin(ID3D11DeviceContext* dc, const ShaderParameter3D& rc) = 0;

	// �`��
	virtual void Draw(ID3D11DeviceContext* dc, const Model* model) = 0;

	// �`��I��
	virtual void End(ID3D11DeviceContext* context) = 0;
};
