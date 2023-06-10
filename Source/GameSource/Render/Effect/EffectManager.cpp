#include "Graphics/Graphics.h"
#include "EffectManager.h"

//������
void EffectManager::Initialize()
{
    Graphics& graphics = Graphics::Instance();

    //Effekseer�����_������
    effekseerRenderer_ = EffekseerRendererDX11::Renderer::
        Create(graphics.GetDevice(), graphics.GetDeviceContext(), 2048);

    //Effekseer�}�l�[�W���[����
    effekseerManager_ = Effekseer::Manager::Create(2048);

    //Effekseer�����_���̊e��ݒ�
    //�i���ʂȃJ�X�^�}�C�Y�����Ȃ��ꍇ�͓T�^�I�Ɉȉ��̐ݒ��OK�j
    effekseerManager_->SetSpriteRenderer(effekseerRenderer_->CreateSpriteRenderer());
    effekseerManager_->SetRibbonRenderer(effekseerRenderer_->CreateRibbonRenderer());
    effekseerManager_->SetRingRenderer(effekseerRenderer_->CreateRingRenderer());
    effekseerManager_->SetTrackRenderer(effekseerRenderer_->CreateTrackRenderer());
    effekseerManager_->SetModelRenderer(effekseerRenderer_->CreateModelRenderer());
    //Effekseer���ł̃��[�_�[�̐ݒ�
        //�i���ʂȃJ�X�^�}�C�Y�����Ȃ��ꍇ�͓T�^�I�Ɉȉ��̐ݒ��OK�j
    effekseerManager_->SetTextureLoader(effekseerRenderer_->CreateTextureLoader());
    effekseerManager_->SetModelLoader(effekseerRenderer_->CreateModelLoader());
    effekseerManager_->SetMaterialLoader(effekseerRenderer_->CreateMaterialLoader());

    //Effekseer��������W�n�Ōv�Z����
    effekseerManager_->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

//�I����
void EffectManager::Finalize()
{
}

//�X�V����
void EffectManager::Update(float elapsedTime)
{
    //�G�t�F�N�g�X�V�����i�����ɂ̓t���[���̌o�ߎ��Ԃ�n���j
    effekseerManager_->Update(elapsedTime * 60.0f);
}

//�`�揈��
void EffectManager::Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
    //�r���[���v���W�F�N�V�����}�b�s���O�s���Effekseer�����_���ɐݒ�
    effekseerRenderer_->SetCameraMatrix
    (*reinterpret_cast<const Effekseer::Matrix44*>(&view));
    effekseerRenderer_->SetProjectionMatrix
    (*reinterpret_cast<const Effekseer::Matrix44*>(&projection));

    //Effekseer�`��J�n
    effekseerRenderer_->BeginRendering();

    //Effekseer�`����s
    effekseerManager_->Draw();

    //Effekseer�`��I��
    effekseerRenderer_->EndRendering();
}
