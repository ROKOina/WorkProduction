#pragma once

#include <memory>

class ParticleSystemCom;

class ParticleComManager
{
private:
    ParticleComManager() {}
    ~ParticleComManager() {}

public:
	// 唯一のインスタンス取得
	static ParticleComManager& Instance()
	{
		static ParticleComManager instance;
		return instance;
	}

	void OnGui();

private:
    std::weak_ptr<ParticleSystemCom> particles;
};