#pragma once

#include <windows.h>
#include "HighResolutionTimer.h"
#include "Audio/Audio.h"
#include "Graphics/Graphics.h"
#include "Input/input.h"

//�T�u�E�B���h�E
#include "Inspector\Inspector.h"
#include "Inspector\SubWindowManager.h"

#include "dxgidebug.h"

class Framework
{
public:
	Framework(HWND hWnd);
	~Framework();

private:
	void Update(float elapsedTime/*Elapsed seconds from last frame*/);
	void Render(float elapsedTime/*Elapsed seconds from last frame*/);

	void CalculateFrameStats();

public:
	int Run();
	LRESULT CALLBACK HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	const HWND				hWnd_;
	HighResolutionTimer		timer_;
	Audio					audio_;
	Graphics				graphics_;
	Input					input_;


	Microsoft::WRL::ComPtr<IDXGIDebug>				debugGI_;
	Microsoft::WRL::ComPtr<ID3D11Debug>				debugID_;

private:	//�T�u�E�B���h�E
	void AddSubWindow(int width = 100, int height = 100);
	int countSubWindow_;
};

