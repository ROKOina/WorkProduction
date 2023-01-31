#include "StageMoveFloor.h"

//�R���X�g���N�^
StageMoveFloor::StageMoveFloor()
{
    scale.x = scale.z = 3.0f;
    scale.y = 0.5f;

    //�X�e�[�W���f����ǂݍ���
    model = new Model("Data/Model/Cube/Cube.mdl");
}

//�f�X�g���N�^
StageMoveFloor::~StageMoveFloor()
{
    //�X�e�[�W���f����j��
    delete model;
}

//�X�V����
void StageMoveFloor::Update(float elapsedTime)
{
    //�O��̏���ۑ�
    oldTransform = transform;
    oldAngle = angle;

    //�X�^�[�g����S�[���܂ł̋������Z�o����
    DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
    DirectX::XMVECTOR Goal = DirectX::XMLoadFloat3(&goal);
    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(Goal,Start);
    DirectX::XMVECTOR Length = DirectX::XMVector3Length(Vec);
    float length;
    DirectX::XMStoreFloat(&length, Length);

    //�X�^�[�g����S�[���܂ł̊Ԃ���b�ԂŐi�ފ���(0.0~1.0)���Z�o����
    float speed = moveSpeed * elapsedTime;
    float speedRate = speed / length;
    moveRate += speedRate;

    //�S�[���ɓ��B�A�܂��̓X�^�[�g�ɖ߂����ꍇ�A�ړ������𔽓]������
    if (moveRate <= 0.0f || moveRate >= 1.0f)
    {
        moveSpeed = -moveSpeed;
    }

    //���`�⊮�ňʒu���Z�o����
    DirectX::XMVECTOR Position = DirectX::XMVectorLerp(Start, Goal, moveRate);
    DirectX::XMStoreFloat3(&position, Position);

    //��]
    angle.x += torque.x * elapsedTime;
    angle.y += torque.y * elapsedTime;
    angle.z += torque.z * elapsedTime;

    //�s��X�V
    UpdateTransform();

    //���f���s��X�V
    //���C�L���X�g�p�Ƀ��f����ԍs��ɂ��邽�ߒP�ʍs���n��
    const DirectX::XMFLOAT4X4 transformIdentity= { 1,0,0,0
                                      ,0,1,0,0
                                      ,0,0,1,0
                                      ,0,0,0,1 };
    model->UpdateTransform(transformIdentity);


 }

//�`�揈��
void StageMoveFloor::Render(ID3D11DeviceContext* dc, Shader* shader)
{
    //�\���p�̂��߃��[���h�s��ɍX�V����
    model->UpdateTransform(transform);

    shader->Draw(dc, model);
 }

//���C�L���X�g
bool StageMoveFloor::RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit)
{
    //return Collision::IntersectRayVsModel(start, end, model, hit);

    //�O��̃��[���h�s��Ƌt�s������߂�
    DirectX::XMMATRIX OldLocalTransform = DirectX::XMLoadFloat4x4(&oldTransform);
    DirectX::XMMATRIX InverseOldWorldTransform=DirectX::XMMatrixInverse(nullptr, OldLocalTransform);   

    //�O��̃��[�J����Ԃł̃��C�ɕϊ�
    DirectX::XMVECTOR WorldStart = DirectX::XMLoadFloat3(&start);
    DirectX::XMVECTOR WorldEnd = DirectX::XMLoadFloat3(&end);

    DirectX::XMVECTOR S = DirectX::XMVector3TransformCoord(WorldStart, InverseOldWorldTransform);
    DirectX::XMVECTOR E = DirectX::XMVector3TransformCoord(WorldEnd, InverseOldWorldTransform);
    //DirectX::XMVECTOR SE = DirectX::XMVectorSubtract(E, S);
    //DirectX::XMVECTOR V = DirectX::XMVector3Normalize(SE);
    //DirectX::XMVECTOR Length = DirectX::XMVector3Length(SE);
    
    //���[�J����Ԃł̃��C�Ƃ̌�_�����߂�
    DirectX::XMFLOAT3 localStart, localEnd;
    DirectX::XMStoreFloat3(&localStart, S);
    DirectX::XMStoreFloat3(&localEnd, E);

    HitResult localHit;
    if (Collision::IntersectRayVsModel(localStart, localEnd, model, localHit))
    {
        //�O��̃��[�J����Ԃ��獡��̃��[���h��Ԃ֕ϊ�
        //�O�񂩂獡��ɂ����ĕύX���ꂽ���e������Ă���I�u�W�F�N�g�ɔ��f�����

        DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&transform); //�s����X�V

        DirectX::XMVECTOR HitPosition = DirectX::XMLoadFloat3(&localHit.position);  //��_��ϊ�

        //�ړ���̍s����Z�o
        DirectX::XMVECTOR WorldPosition =
            DirectX::XMVector3TransformCoord(HitPosition, WorldTransform);

        DirectX::XMStoreFloat3(&hit.position, WorldPosition);   //�q�b�g�|�W�V���������݂̃��[���h��Ԃɕϊ�

        //��]�������Z�o
        hit.rotation.x = angle.x - oldAngle.x;
        hit.rotation.y = angle.y - oldAngle.y;
        hit.rotation.z = angle.z - oldAngle.z;

        return true;
    }

    return false;

}

//�s��X�V����
void StageMoveFloor::UpdateTransform()
{
    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
    DirectX::XMMATRIX W = S * R * T;
    DirectX::XMStoreFloat4x4(&transform, W);

}
