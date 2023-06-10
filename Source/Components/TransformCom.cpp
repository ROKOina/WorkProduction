#include <imgui.h>
#include "TransformCom.h"
#include <cmath>

// �J�n����
void TransformCom::Start()
{

}

// �X�V����
void TransformCom::Update(float elapsedTime)
{

}

// GUI�`��
void TransformCom::OnGUI()
{
	// �g�����X�t�H�[��
	ImGui::DragFloat3("Position", &position_.x, 0.1f);
	ImGui::DragFloat3("WorldPosition", &worldPosition_.x, 0.1f);

	ImGui::DragFloat4("Rotation", &rotation_.x, 0.1f);

	{	//�I�C���[�p
		DirectX::XMFLOAT3 euler = eulerRotation_;
		if (ImGui::DragFloat3("EulerRotato", &euler.x)) {
			SetEulerRotation(euler);
		}
	}

	ImGui::DragFloat3("Scale", &scale_.x, 0.1f);
}

//�s��X�V
void TransformCom::UpdateTransform()
{
	// ���[���h�s��̍X�V
	DirectX::XMVECTOR Q = DirectX::XMLoadFloat4(&rotation_);
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale_.x, scale_.y, scale_.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionNormalize(Q));
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);

	DirectX::XMMATRIX L = DirectX::XMLoadFloat4x4(&parentTransform_);
	DirectX::XMMATRIX W = S * R * T * L;

	DirectX::XMStoreFloat4x4(&transform_, W);

	worldPosition_ = { transform_._41,transform_._42,transform_._43 };


}

//�w�����������
void TransformCom::LookAtTransform(const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up)
{
	UpdateTransform();

	//�ʒu�A�����_�A���������r���[�s����쐬
	DirectX::XMVECTOR Eye = DirectX::XMLoadFloat3(&worldPosition_);
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
	DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);

	//�������W�̏ꍇ�A�������炷
	DirectX::XMFLOAT3 wp = worldPosition_;
	if (focus.x == wp.x && focus.y == wp.y && focus.z == wp.z)
	{
		wp.y += 0.0001f;
		Eye = DirectX::XMLoadFloat3(&wp);
	}

	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);

	//�r���[���t�s�񉻂��A���[���h���W�ɖ߂�
	DirectX::XMMATRIX World = DirectX::XMMatrixInverse(nullptr, View);
	if (!std::isfinite(World.r->m128_f32[0]))return;

	DirectX::XMStoreFloat4x4(&transform_, World);

	//�N�H�[�^�j�I���ɓK�p
	DirectX::XMStoreFloat4(&rotation_,
		DirectX::XMQuaternionRotationMatrix(DirectX::XMLoadFloat4x4(&transform_)));
}

//�w���Up�ɍ��킹��
void TransformCom::SetUpTransform(const DirectX::XMFLOAT3& up)
{
	DirectX::XMFLOAT3 front = GetFront();
	DirectX::XMFLOAT3 right = GetRight();
	DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);
	DirectX::XMVECTOR Front = DirectX::XMLoadFloat3(&front);
	DirectX::XMVECTOR Right = DirectX::XMLoadFloat3(&right);

	//up�����
	Right = DirectX::XMVector3Cross(Up, Front);
	Front = DirectX::XMVector3Cross(Right, Up);
	
	DirectX::XMFLOAT3 up2;
	DirectX::XMStoreFloat3(&up2, Up);
	DirectX::XMStoreFloat3(&front, Front);
	DirectX::XMStoreFloat3(&right, Right);

	DirectX::XMFLOAT4X4 rt = {
		right.x, right.y, right.z, 0,
		up.x, up.y, up.z, 0,
		front.x, front.y, front.z, 0,
		0, 0, 0, 1 };
	DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&rt);

	DirectX::XMStoreFloat4(&rotation_, DirectX::XMQuaternionRotationMatrix(M));
}