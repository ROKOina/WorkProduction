#pragma once

#include "Inspector\SubWindowManager.h"

#include "GameSource\Camera\CameraController.h"

class Inspector : public SubWindow
{
public:
	Inspector(HWND hWnd, int ID, int width, int height);
	~Inspector()override;

private:
	void Update(float elapsedTime/*Elapsed seconds from last frame*/)override;
	void Render(float elapsedTime/*Elapsed seconds from last frame*/)override;

private:

	std::unique_ptr<Model> p;
	std::unique_ptr<CameraController> cameraController;
};

