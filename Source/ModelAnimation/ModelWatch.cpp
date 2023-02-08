#include "ModelWatch.h"

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#define IMAPP_IMPL

#include "ImGuizmo\ImGuizmo.h"
#include "ImGuizmo\ImSequencer.h"
#include "ImGuizmo\ImZoomSlider.h"
#include "ImGuizmo\ImCurveEdit.h"
#include "ImGuizmo\GraphEditor.h"
#include <math.h>
#include <vector>
#include <algorithm>


void ModelWatch::Update(float elapsedTime)
{
}

void ModelWatch::Render(float elapsedTime)
{

    ImGuiIO& io = ImGui::GetIO();
    float viewManipulateRight = io.DisplaySize.x;
    float viewManipulateTop = 0;
    static ImGuiWindowFlags gizmoWindowFlags = 0;
    gizmoWindowFlags |= ImGuiWindowFlags_MenuBar;

    //ウィンドウ作成処理
    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(400, 20), ImGuiCond_Appearing);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(0.35f, 0.3f, 0.3f));
    ImGui::Begin("Gizmo", 0, gizmoWindowFlags);
    //ImGui::Begin("Gizmo", 0);
    //ImGuizmo::SetDrawlist();
    float windowWidth = (float)ImGui::GetWindowWidth();
    float windowHeight = (float)ImGui::GetWindowHeight();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
    viewManipulateRight = ImGui::GetWindowPos().x + windowWidth;
    viewManipulateTop = ImGui::GetWindowPos().y;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    gizmoWindowFlags = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max) ? ImGuiWindowFlags_NoMove : 0;

    // ファイルメニュー
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            bool check = false;
            if (ImGui::MenuItem("Open Model", "", &check))
            {
                //OpenModelFile();
            }
            if (ImGui::MenuItem("Open Model2", "", &check))
            {
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    //ウィンドウ終了処理
    ImGui::End();
    ImGui::PopStyleColor(1);
}
