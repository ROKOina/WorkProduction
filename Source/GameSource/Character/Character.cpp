#include "Character.h"
#include "GameSource\Stage\Stage.h"
#include <imgui.h>
#include "Logger.h"
#include "GameSource\Math\Mathf.h"
#include "GameSource\Stage\StageManager.h"

#include <cmath>

//�ړ�����
void Character::IsMove(float vx, float vz, float speed)
{
    //�ړ������x�N�g����ݒ�
    moveVecX_ = vx;
    moveVecZ_ = vz;

    //�ő呬�x�ݒ�
    maxMoveSpeed_ = speed;
}

//�W�����v����
void Character::Jump(float speed)
{
    //������̗͂�ݒ�
    velocity_.y = speed;
}

//���͏����X�V
void Character::UpdateVelocity(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3& up)
{
    //�o�߃t���[��
    float elapsedFrame = 120.0f * elapsedTime;

    //�������͍X�V����
    UpdateVertialVelocity(elapsedFrame);

    //�������͍X�V����
    UpdateHorizontalVelocity(elapsedFrame);

    //�����ړ��X�V����
    UpdateVertialMove(elapsedTime,position,angle,up);

    //�����ړ��X�V����
    UpdateHorizontalMove(elapsedTime, position, angle,up);
}

//�������͍X�V����
void Character::UpdateVertialVelocity(float elapsedFrame)
{
    //�d�͏���
    velocity_.y += gravity_ * elapsedFrame;
}

//�������͍X�V����
void Character::UpdateHorizontalVelocity(float elapsedFrame)
{
    //XZ���ʂ̑��͂���������
    float length = sqrtf(velocity_.x*velocity_.x+velocity_.z*velocity_.z);
    if (length > 0.0f)
    {
        //���C��
        float Friction = this->friction_ * elapsedFrame;

       ////�󒆂ɂ���Ƃ��͖��C�͂����炷
       // if (!isGround)Friction -= airControl;

        //���C�ɂ�鉡�����̌�������
        if (length > friction_)
        {
            //�P�ʃx�N�g��
            float vx = velocity_.x / length;
            float vz = velocity_.z / length;

            velocity_.x -= vx * friction_;
            velocity_.z -= vz * friction_;
        }
        //�������̑��͂����C�͈ȉ��ɂȂ����̂ŁA���͂𖳌���
        else
        {
            velocity_.x = 0;
            velocity_.z = 0;
        }

    }

    //XZ���ʂ̑��͂���������
    if (length <= maxMoveSpeed_)
    {
        //�ړ��x�N�g�����[���x�N�g���łȂ��Ȃ��������
        float moveVecLength = fabsf(moveVecX_) + fabsf(moveVecZ_);
        if (moveVecLength > 0.0f)
        {
            //������
            float acceleration = this->acceleration_ * elapsedFrame;
           
            ////�󒆂ɂ���Ƃ��͉����͂����炷
            //if (!isGround)acceleration -= airControl;
            
            //�ړ��x�N�g���ɂ���������
            velocity_.x += moveVecX_*acceleration;
            velocity_.z += moveVecZ_*acceleration;
            
            //�ő呬�x����
            DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&velocity_);
            Velocity = DirectX::XMVectorSetY(Velocity, 0);   //y����0��
            DirectX::XMVECTOR Length = DirectX::XMVector3Length(Velocity);

            float length = DirectX::XMVectorGetX(Length);
            if (length > maxMoveSpeed_)
            {
               Velocity = DirectX::XMVector3Normalize(Velocity);    //���K������
               Velocity = DirectX::XMVectorScale(Velocity, maxMoveSpeed_);//�ő呬�x�ɐݒ�
               
               velocity_.x = DirectX::XMVectorGetX(Velocity);
               velocity_.z = DirectX::XMVectorGetZ(Velocity);
            }

            ////�����ŃK�^�K�^���Ȃ��悤�ɂ���
            //if (isGround && slopeRate > 0.0f)
            //{
            //    velocity.y -= length * slopeRate * elapsedFrame;
            //}
        }
    }

    //�ړ��x�N�g�������Z�b�g
    moveVecX_ = 0;
    moveVecZ_ = 0;
}

//�����ړ��X�V����
void Character::UpdateVertialMove(float elapsedTime,DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3& up)
{
    //���������̈ړ���
    float my = velocity_.y * elapsedTime;

    //�L�����N�^�[��Y�������ƂȂ�@���x�N�g��
    DirectX::XMFLOAT3 normal = { 0,1,0 };

    slopeRate_ = 0.0f;   //�⓹���������p

    //������
    if (my < 0.0f)
    {
        //���C�̊J�n�ʒu�͑�����菭����
        DirectX::XMFLOAT3 start = { position.x,position.y + stepOffset_,position.z };
        //���C�̏I�_�ʒu�͈ړ���̈ʒu
        DirectX::XMFLOAT3 end = { position.x,position.y + my,position.z };

        //���C�L���X�g�ɂ��n�ʔ���
        HitResult hit;
        if (StageManager::Instance().RayCast(start, end, hit))
        {
            //�@���x�N�g���擾
            normal = hit.normal;

            //�n�ʂɐڒn���Ă���
            position = hit.position;

            //�X�Η��̌v�Z
            float normalLenghtXZ = sqrtf(
                hit.normal.x * hit.normal.x + hit.normal.z * hit.normal.z);
            slopeRate_ = 1.0f - 
                (hit.normal.y / (normalLenghtXZ + hit.normal.y));

            ////���n����
            //if (!isGround)
            //{
            //    OnLanding();
            //}
            //isGround = true;
            velocity_.y = 0.0f;
        }
        else
        {
            //�󒆂ɕ����Ă���
            position.y += my;
            //isGround = false;
        }
    }

    //�㏸��
    else if (my > 0.0f)
    {
        position.y += my;
        //isGround = false;
    }
    //�n�ʂ̌����ɉ����悤��XZ����]
    {
        //Y�����@���x�N�g�������Ɍ����I�C���[�p��]���Z�o����
        float angleX, angleZ;
        angleX = atan2f( normal.z, normal.y);
        angleZ = atan2f(-normal.x, normal.y);

        //���`�⊮�Ŋ��炩�ɉ�]����
        angle.x = Mathf::Lerp(angle.x, angleX, 0.1f);
        angle.z = Mathf::Lerp(angle.z, angleZ, 0.1f);

        up = normal;
    }
}

//�����ړ��X�V����
void Character::UpdateHorizontalMove(float elapsedTime, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3& up)
{
    //�������͗ʌv�Z
    float velocityLengthXZ = fabsf(velocity_.x) + fabsf(velocity_.z);
    if (velocityLengthXZ > 0.0f)
    {
        //�����ړ���
        float mx = velocity_.x * elapsedTime;
        float mz = velocity_.z * elapsedTime;

        //���C�̊J�n�ʒu�ƏI�_�ʒu
        DirectX::XMFLOAT3 start = { position.x ,position.y + stepOffset_ ,position.z };
        
        DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);

        DirectX::XMFLOAT3 end = { start.x + mx ,start.y ,start.z + mz };

        //���C�L���X�g�ɂ��ǔ���
        HitResult hit;
        if (StageManager::Instance().RayCast(start, end, hit))
        {
            //�ǂ܂ł̃x�N�g��
            DirectX::XMVECTOR StartHit = DirectX::XMLoadFloat3(&hit.position);
            DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
            DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(StartHit, End);

            //�ǂ̖@��
            DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&hit.normal);
            DirectX::XMFLOAT3 normal;
            DirectX::XMStoreFloat3(&normal, Normal);

            //Logger::Print("x:%.2f y:%.2f z:%.2f\n", normal.x, normal.y, normal.z);

            //���˃x�N�g����@���Ɏˉe
            DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(Normal, Vec);

            //�␳��̌v�Z
            DirectX::XMVECTOR ND = DirectX::XMVectorScale(Normal, DirectX::XMVectorGetX(Dot));
            DirectX::XMVECTOR EndWall = DirectX::XMVectorScale(Normal, DirectX::XMVectorGetX(Dot)*1.2f);

            //�␳��ɕ��̂��������ꍇ�~�܂�
            DirectX::XMStoreFloat3(&start, Start);
            DirectX::XMStoreFloat3(&end, EndWall);
            start.y += stepOffset_ * 5;
            end.y += stepOffset_ * 5;
            if (StageManager::Instance().RayCast(start, end, hit))return;  //���̂��������ꍇreturn

            DirectX::XMVECTOR VecRay = DirectX::XMVectorSubtract(End, Start);
            DirectX::XMVECTOR R = DirectX::XMVectorAdd(VecRay, ND); //�ǂ���ׂ��Ƃ�擾



            //�ǂ���ړ�
            position.x += DirectX::XMVectorGetX(R);
            position.z += DirectX::XMVectorGetZ(R);

            //�R���p�C�����萔(https://www.slideshare.net/GenyaMurakami/constexpr-10458089
            constexpr float epsilon = 0.001;

            position.x += normal.x * epsilon;
            position.z += normal.z * epsilon;

        }
        else
        {
            //�ړ�
            position.x += mx;
            position.z += mz;
        }
    }
}

//���񏈗�
void Character::Turn(float elapsedTime, float vx, float vz, float speed, DirectX::XMFLOAT4& angle, DirectX::XMFLOAT3 up)
{
    if (!std::isfinite(vx))return;

    speed *= elapsedTime;




    //�i�s�x�N�g����P�ʃx�N�g����
    float moveVecLength = sqrtf(vx * vx + vz * vz);
    if (moveVecLength <= 0)return;  //�i�s�x�N�g�����[���x�N�g���̏ꍇ�͏�������K�v�Ȃ�
    if (moveVecLength > 0.0f)
    {
        //�P�ʃx�N�g����
        vx /= moveVecLength;
        vz /= moveVecLength;
    }

    //���g�̉�]�l����O���������߂�
    float frontX = sinf(angle.y);
    float frontZ = cosf(angle.y);

    //��]�p�����߂邽�߁A�Q�̒P�ʃx�N�g���̓��ς��v�Z����
    float dot = (vx * frontX) + (vz * frontZ);

    //���ϒl��-1.0~1.0�ŕ\������Ă���A�Q�̒P�ʃx�N�g���̊p�x��
    //�������قǂ�1.0�ɋ߂Â��Ƃ��������𗘗p���ĉ�]���x�𒲐�����
    float rot = 1.0f - dot; //�␳��
    if (rot > speed)rot = speed;

    //���E������s�����߂�2�̒P�ʃx�N�g���̊O�ς��v�Z����
    float cross = (vx * frontZ) - (vz * frontX);

    //2D�̊O�ϒl�����̏ꍇ�����̏ꍇ�ɂ���č��E���肪�s����
    //���E������s�����Ƃɂ���č��E��]��I������
    if (cross < 0.0f)   //�E
    {
        DirectX::XMVECTOR Angle = DirectX::XMLoadFloat4(&angle);
        DirectX::XMVECTOR AngleAdd = DirectX::XMQuaternionMultiply(DirectX::XMQuaternionRotationAxis({ up.x,up.y,up.z }, DirectX::XMConvertToRadians(-rot)), Angle);
        DirectX::XMStoreFloat4(&angle, AngleAdd);
    }
    else
    {
        DirectX::XMVECTOR Angle = DirectX::XMLoadFloat4(&angle);
        DirectX::XMVECTOR AngleAdd = DirectX::XMQuaternionMultiply(DirectX::XMQuaternionRotationAxis({ up.x,up.y,up.z }, DirectX::XMConvertToRadians(rot)), Angle);
        DirectX::XMStoreFloat4(&angle, AngleAdd);
    }

    //DirectX::XMVECTOR Angle = DirectX::XMLoadFloat4(&angle_);
    //DirectX::XMVECTOR AngleAdd = DirectX::XMQuaternionMultiply(DirectX::XMQuaternionRotationAxis({ up_.x,up_.y,up_.z }, 0.1f), Angle);
    //DirectX::XMStoreFloat4(&angle_, AngleAdd);

}

