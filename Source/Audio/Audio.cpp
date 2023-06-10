#include "Misc.h"
#include "Audio/Audio.h"

Audio* Audio::instance = nullptr;

// �R���X�g���N�^
Audio::Audio()
{
	// �C���X�^���X�ݒ�
	_ASSERT_EXPR(instance == nullptr, "already instantiated");
	instance = this;

	HRESULT hr;

	// COM�̏�����
	hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	UINT32 createFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	//createFlags |= XAUDIO2_DEBUG_ENGINE;
#endif

	// XAudio������
	hr = XAudio2Create(&xaudio_, createFlags);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	// �}�X�^�����O�{�C�X����
	hr = xaudio_->CreateMasteringVoice(&masteringVoice_);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

// �f�X�g���N�^
Audio::~Audio()
{
	// �}�X�^�����O�{�C�X�j��
	if (masteringVoice_ != nullptr)
	{
		masteringVoice_->DestroyVoice();
		masteringVoice_ = nullptr;
	}

	// XAudio�I����
	if (xaudio_ != nullptr)
	{
		xaudio_->Release();
		xaudio_ = nullptr;
	}

	// COM�I����
	CoUninitialize();
}

// �I�[�f�B�I�\�[�X�ǂݍ���
std::unique_ptr<AudioSource> Audio::LoadAudioSource(const char* filename)
{
	std::shared_ptr<AudioResource> resource = std::make_shared<AudioResource>(filename);
	return std::make_unique<AudioSource>(xaudio_, resource);
}
