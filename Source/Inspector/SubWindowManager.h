#pragma once

#include <windows.h>
#include "HighResolutionTimer.h"

#include "Graphics/Graphics.h"

class SubWindow;

class SubWindowManager
{
public:
    SubWindowManager(){}
    ~SubWindowManager(){}

    // �C���X�^���X�擾
    static SubWindowManager& Instance() {
        static SubWindowManager instance;
        return instance;
    }


public:
    int Run(float elapsedTime);

    void SetSyncInterval(int s) { syncInterval = s; }
    const int GetSyncInterval() const { return syncInterval; }

    void AddSubWindow(SubWindow* s);

    void Clear();

private:
    int syncInterval;

    std::vector<SubWindow*> subWindows;
};

//�E�B���h�E�����Ƃ��Ɍp������
class SubWindow
{
public:
    SubWindow(HWND hWnd, int ID);
    virtual ~SubWindow();

public: //�������z�֐��Ŏ��̉����Ȃ��悤��
    virtual void Update(float elapsedTime/*Elapsed seconds from last frame*/) = 0;
    virtual void Render(float elapsedTime/*Elapsed seconds from last frame*/);

    int Run(float elapsedTime);

protected:
    const HWND				hWnd;
    HighResolutionTimer		timer;

    int windowID;	//�E�B���h�E���ʔԍ�
};