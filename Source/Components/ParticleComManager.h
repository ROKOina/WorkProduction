#pragma once

#include <memory>

class ParticleSystemCom;

class ParticleComManager
{
private:
    ParticleComManager() {}
    ~ParticleComManager() {}

public:
	// �B��̃C���X�^���X�擾
	static ParticleComManager& Instance()
	{
		static ParticleComManager instance;
		return instance;
	}

	void OnGui();

private:
    std::weak_ptr<ParticleSystemCom> particles;
};