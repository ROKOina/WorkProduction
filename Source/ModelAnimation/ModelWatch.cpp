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

#include "Graphics\Graphics.h"

float screenWidth = 800;
float screenHeight = 400;

ModelWatch::ModelWatch()
{
    cameraController = std::make_unique<CameraController>();

    std::shared_ptr<CameraCom> camera = cameraController->GetCamera();
    camera->SetLookAt(
        DirectX::XMFLOAT3(0, 0, 0),
        DirectX::XMFLOAT3(0, 1, 0)
    );
    camera->SetPerspectiveFov(
        DirectX::XMConvertToRadians(45),
        screenWidth / screenHeight,
        0.1f, 1000.0f
    );
}


void ModelWatch::Update(float elapsedTime)
{
    std::shared_ptr<CameraCom> camera = cameraController->GetCamera();

    //回転行列を作成
    DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(cameraAngle.y),
        DirectX::XMConvertToRadians(cameraAngle.x),
        0);

    //回転行列から前方向ベクトルを取り出す
    DirectX::XMVECTOR Front = Transform.r[2];
    DirectX::XMFLOAT3 front;
    DirectX::XMStoreFloat3(&front, Front);

    //注視点から後ろベクトル方向に一定距離離れたカメラ視点を求める
    front.x = cameraEye.x + front.x;
    front.y = cameraEye.y + front.y;
    front.z = cameraEye.z + front.z;

    //camera->SetLookAt(cameraEye, front, { 0,1,0 });

    if (cameraAngle.y >= 360) {
        cameraAngle.y = 0;
    }
    if (cameraAngle.y < 0) {
        cameraAngle.y = 360;
    }
    if (cameraAngle.x >= 360) {
        cameraAngle.x = 0;
    }
    if (cameraAngle.x < 0) {
        cameraAngle.x = 360;
    }
}

void ModelWatch::Render(float elapsedTime)
{
    std::shared_ptr<CameraCom> camera = cameraController->GetCamera();

    ImGuiIO& io = ImGui::GetIO();
    float viewManipulateRight = io.DisplaySize.x;
    float viewManipulateTop = 0;
    static ImGuiWindowFlags gizmoWindowFlags = 0;

    //メニューバーの表示
    gizmoWindowFlags |= ImGuiWindowFlags_MenuBar;

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    //ウィンドウ作成処理
    ImGui::SetNextWindowSize(ImVec2(screenWidth, screenHeight), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(400, 20), ImGuiCond_Appearing);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(0.35f, 0.3f, 0.3f));

    ImGui::Begin("Gizmo", 0, gizmoWindowFlags);
    ImGuizmo::SetDrawlist();
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


    static const float identityMatrix[16] =
    { 1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f };

    ImGuizmo::DrawGrid(&camera->GetView()._11, &camera->GetProjection()._11, identityMatrix, 100.0f);
    ImGuizmo::DrawCubes(&camera->GetView()._11, &camera->GetProjection()._11, identityMatrix, 1);

    //カメラ
    if (ImGui::TreeNode("Camera")) {
        ImGui::DragFloat3("Eye", &cameraEye.x);
        ImGui::DragFloat2("angle", &cameraAngle.x);
        ImGui::TreePop();
    }


    //ウィンドウ終了処理
    ImGui::End();
    ImGui::PopStyleColor(1);
}
