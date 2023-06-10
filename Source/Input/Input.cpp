#include "Input/Input.h"

Input* Input::instance_ = nullptr;

// コンストラクタ
Input::Input(HWND hWnd)
	: mouse_(hWnd)
{
	instance_ = this;
}

// 更新処理
void Input::Update()
{
	gamePad_.Update();
	mouse_.Update();
}
