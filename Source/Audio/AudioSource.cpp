#include "Misc.h"
#include "Audio/AudioSource.h"

// コンストラクタ
AudioSource::AudioSource(IXAudio2* xaudio, std::shared_ptr<AudioResource>& resource)
	: resource_(resource)
{
	HRESULT hr;

	// ソースボイスを生成
	hr = xaudio->CreateSourceVoice(&sourceVoice_, &resource->GetWaveFormat());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

// デストラクタ
AudioSource::~AudioSource()
{
	//if (sourceVoice_ != nullptr)
	//{
	//	sourceVoice_->DestroyVoice();
	//	sourceVoice_ = nullptr;
	//}
}

// 再生
void AudioSource::Play(bool loop, float volume)
{
	Stop();

	// ソースボイスにデータを送信
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes = resource_->GetAudioBytes();
	buffer.pAudioData = resource_->GetAudioData();
	buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	
	sourceVoice_->SubmitSourceBuffer(&buffer);

	HRESULT hr = sourceVoice_->Start();
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	sourceVoice_->SetVolume(volume * 0.1f);
}

// 停止
void AudioSource::Stop()
{
	sourceVoice_->FlushSourceBuffers();
	sourceVoice_->Stop();
}

void AudioSource::AudioRelease()
{
	if (sourceVoice_ != nullptr)
	{
		sourceVoice_->DestroyVoice();
		sourceVoice_ = nullptr;
	}
}
