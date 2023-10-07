#pragma once

#include "Components\System\Component.h"

#include <wrl.h>
#include <d3d11.h>

class SwordTrailCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    SwordTrailCom() {}
    ~SwordTrailCom() {}

    // ���O�擾
    const char* GetName() const override { return "SwordTrail"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;

    //SwordTrailCom�N���X
public:
    //�`��
    void Render();

    //���[�Ɛ�[�m�[�h�o�^(tail�F���[�@�A�@head�F��[)
    void SetHeadTailNodeName(std::string tail, std::string head)
    {
        nodeName_[0] = tail;
        nodeName_[1] = head;
    }

private:
    //���̈ʒu��ۑ�����\����
    struct PosBuffer
    {
        DirectX::XMFLOAT3 head = { 0,0,0 }; //���̐�[�̈ʒu
        DirectX::XMFLOAT3 tail = { 0,0,0 }; //���̖��[�̈ʒu
    };

    //���_�o�b�t�@
    struct SwordTrailVertex
    {
        DirectX::XMFLOAT3 pos = { 0, 0, 0 };
        DirectX::XMFLOAT2 uv = { 0, 0 };
    };

    //�Ȑ��̒��_�����
    void CreateCurveVertex(std::vector<PosBuffer>& usedPosArray, int split);



private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;   
    Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;

    Microsoft::WRL::ComPtr<ID3D11Buffer> vBuffer_;

    struct TrailBuffer
    {
        DirectX::XMFLOAT4X4	viewProjection;
        DirectX::XMFLOAT4 trailColor;
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> VPBuffer_;

    std::vector<PosBuffer> posArray_;             //���̈ʒu��ۑ�����o�b�t�@
    std::vector<SwordTrailVertex> vertex_;        //���_�o�b�t�@

    //�g���C���̐�[�Ɩ��[�m�[�h�̖��O
    std::string nodeName_[2];   //0:���[�@�A1:��[

};