#pragma once

class EnemyCom;

// ���s����
class JudgmentBase
{
public:
	JudgmentBase(EnemyCom* enemy) :owner_(enemy) {}
	virtual bool Judgment() = 0;
protected:
	EnemyCom* owner_;
};