#include "Inspector\SubWindowManager.h"


// SubWindowManager�N���X //

int SubWindowManager::Run(float elapsedTime)
{
    for (SubWindow* s : subWindows) {
        s->Run(elapsedTime);
    }

    return 0;
}

void SubWindowManager::AddSubWindow(SubWindow* s)
{
    subWindows.emplace_back(s);
}

void SubWindowManager::Clear()
{
    for (SubWindow* s : subWindows) {
        delete s;
    }
    subWindows.clear();
}


// SubWindow�N���X //

SubWindow::SubWindow(HWND hWnd, int ID, int width, int height)
    : hWnd(hWnd),
    windowID(ID)
{
    //�X���b�v�`�F�C������
    //Graphics::Instance().CreateSubWindowSwapChain(hWnd_, width, height);
}

SubWindow::~SubWindow() 
{

}

void SubWindow::Render(float elapsedTime)
{
    int syncInterval = SubWindowManager::Instance().GetSyncInterval();

    // �o�b�N�o�b�t�@�ɕ`�悵�������ʂɕ\������B
    //Graphics::Instance().GetSubWindowSwapChain(windowID)->Present(syncInterval, 0);
}

int SubWindow::Run(float elapsedTime)
{
    Update(elapsedTime);
    Render(elapsedTime);

    return 0;
}
