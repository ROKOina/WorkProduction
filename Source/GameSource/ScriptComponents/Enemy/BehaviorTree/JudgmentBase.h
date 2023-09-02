#pragma once

class EnemyCom;

// é¿çsîªíË
class JudgmentBase
{
public:
	JudgmentBase(EnemyCom* enemy) :owner_(enemy) {}
	virtual bool Judgment() = 0;
protected:
	EnemyCom* owner_;
};