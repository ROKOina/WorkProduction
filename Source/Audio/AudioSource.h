#pragma once

#include <memory>
#include <xaudio2.h>
#include "Audio/AudioResource.h"

// �I�[�f�B�I�\�[�X
class AudioSource
{
public:
	AudioSource(IXAudio2* xaudio, std::shared_ptr<AudioResource>& resource);
	~AudioSource();

	// �Đ�
	void Play(bool loop, float volume = 1.0f);

	// ��~
	void Stop();

	void AudioRelease();

private:
	IXAudio2SourceVoice* sourceVoice_;
	std::shared_ptr<AudioResource>	resource_;
};
