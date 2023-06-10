#include "Inspector\SubWindowManager.h"


// SubWindowManagerクラス //

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


// SubWindowクラス //

SubWindow::SubWindow(HWND hWnd, int ID, int width, int height)
    : hWnd(hWnd),
    windowID(ID)
{
    //スワップチェイン生成
    //Graphics::Instance().CreateSubWindowSwapChain(hWnd_, width, height);
}

SubWindow::~SubWindow() 
{

}

void SubWindow::Render(float elapsedTime)
{
    int syncInterval = SubWindowManager::Instance().GetSyncInterval();

    // バックバッファに描画した画を画面に表示する。
    //Graphics::Instance().GetSubWindowSwapChain(windowID)->Present(syncInterval, 0);
}

int SubWindow::Run(float elapsedTime)
{
    Update(elapsedTime);
    Render(elapsedTime);

    return 0;
}
