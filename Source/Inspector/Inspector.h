#pragma once

#include <windows.h>
#include "HighResolutionTimer.h"

#include "Graphics/Graphics.h"
#include "Camera.h"
#include "CameraController.h"

//�T�u�E�B���h�E�̐�
#define WINDOW_NUM 2

class Inspector
{
public:
	Inspector(HWND hWnd, int ID);
	~Inspector();

private:
	void Update(float elapsedTime/*Elapsed seconds from last frame*/);
	void Render(float elapsedTime/*Elapsed seconds from last frame*/);

public:
	int Run(float elapsedTime);

	void SetSyncInterval(int syncInterval) { this->syncInterval = syncInterval; }
private:
	const HWND				hWnd;
	HighResolutionTimer		timer;
	int syncInterval;

	int windowID;	//�E�B���h�E���ʔԍ�

	Model* p = nullptr;
	CameraController* cameraController = nullptr;
};

