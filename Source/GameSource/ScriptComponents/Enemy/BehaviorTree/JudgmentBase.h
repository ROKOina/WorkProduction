#pragma once
#include <memory>

class EnemyCom;

// ���s����
class JudgmentBase
{
public:
	JudgmentBase(std::shared_ptr<EnemyCom> enemy) :owner_(enemy) {}
	virtual bool Judgment() = 0;
protected:
	std::weak_ptr<EnemyCom> owner_;
};