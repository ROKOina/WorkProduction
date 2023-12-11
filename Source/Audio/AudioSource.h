#pragma once

#include <memory>
#include <xaudio2.h>
#include "Audio/AudioResource.h"

// オーディオソース
class AudioSource
{
public:
	AudioSource(IXAudio2* xaudio, std::shared_ptr<AudioResource>& resource);
	~AudioSource();

	// 再生
	void Play(bool loop, float volume = 1.0f);

	// 停止
	void Stop();

	void AudioRelease();

private:
	IXAudio2SourceVoice* sourceVoice_;
	std::shared_ptr<AudioResource>	resource_;
};
