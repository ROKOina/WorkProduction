#pragma once

#include "Input/GamePad.h"
#include "Input/Mouse.h"

// インプット
class Input
{
public:
	Input(HWND hWnd);
	~Input() {}

public:
	// インスタンス取得
	static Input& Instance() { return *instance_; }

	// 更新処理
	void Update();

	// ゲームパッド取得
	GamePad& GetGamePad() { return gamePad_; }

	// マウス取得
	Mouse& GetMouse() { return mouse_; }

private:
	static Input*		instance_;
	GamePad				gamePad_;
	Mouse				mouse_;
};
