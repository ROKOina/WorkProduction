#include "Misc.h"
#include "Audio/AudioSource.h"

// �R���X�g���N�^
AudioSource::AudioSource(IXAudio2* xaudio, std::shared_ptr<AudioResource>& resource)
	: resource_(resource)
{
	HRESULT hr;

	// �\�[�X�{�C�X�𐶐�
	hr = xaudio->CreateSourceVoice(&sourceVoice_, &resource->GetWaveFormat());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

// �f�X�g���N�^
AudioSource::~AudioSource()
{
	if (sourceVoice_ != nullptr)
	{
		sourceVoice_->DestroyVoice();
		sourceVoice_ = nullptr;
	}
}

// �Đ�
void AudioSource::Play(bool loop)
{
	Stop();

	// �\�[�X�{�C�X�Ƀf�[�^�𑗐M
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes = resource_->GetAudioBytes();
	buffer.pAudioData = resource_->GetAudioData();
	buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	
	sourceVoice_->SubmitSourceBuffer(&buffer);

	HRESULT hr = sourceVoice_->Start();
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	sourceVoice_->SetVolume(1.0f);
}

// ��~
void AudioSource::Stop()
{
	sourceVoice_->FlushSourceBuffers();
	sourceVoice_->Stop();
}
