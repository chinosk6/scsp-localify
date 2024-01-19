#include "imgui/imgui.h"
#include "stdinclude.hpp"
#include "scgui/scGUIData.hpp"

extern void* SetResolution_orig;


#define INPUT_AND_SLIDER_FLOAT(label, data, min, max) \
    ImGui::SetNextItemWidth(inputFloatWidth);\
    ImGui::InputFloat("##"##label, data);\
    ImGui::SameLine();\
    ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - inputFloatWidth - 1.0f);\
    ImGui::SliderFloat(label, data, min, max)


namespace SCGUILoop {
    static float inputFloatWidth = 50.0f;

	void mainLoop() {
        if (ImGui::Begin("SC Plugin Config")) {
            if (ImGui::Button("Reload Config And Translate Data")) {
                g_reload_all_data();
            }
            ImGui::Checkbox("Waiting Extract Text", &SCGUIData::needExtractText);
            ImGui::Checkbox("Live Allow Same Idol (Dangerous)", &g_allow_same_idol);
            ImGui::Checkbox("Live Allow Use Try On Costume (Dangerous)", &g_allow_use_tryon_costume);

            if (ImGui::CollapsingHeader("Resolution Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::Button("720P")) {
                    SCGUIData::screenW = 1280;
                    SCGUIData::screenH = 720;
                }
                ImGui::SameLine();
                if (ImGui::Button("1080P")) {
                    SCGUIData::screenW = 1920;
                    SCGUIData::screenH = 1080;
                }
                ImGui::SameLine();
                if (ImGui::Button("1440P")) {
                    SCGUIData::screenW = 2560;
                    SCGUIData::screenH = 1440;
                }
                ImGui::SameLine();
                if (ImGui::Button("1620P")) {
                    SCGUIData::screenW = 2880;
                    SCGUIData::screenH = 1620;
                }
                ImGui::SameLine();
                if (ImGui::Button("2160P")) {
                    SCGUIData::screenW = 3840;
                    SCGUIData::screenH = 2160;
                }

                ImGui::InputInt("Width", &SCGUIData::screenW);
                ImGui::InputInt("Height", &SCGUIData::screenH);
                ImGui::Checkbox("Full Screen", &SCGUIData::screenFull);
                if (ImGui::Button("Update Resolution")) {
                    if (SetResolution_orig) {
                        reinterpret_cast<void (*)(int, int, bool)>(SetResolution_orig)(SCGUIData::screenW, SCGUIData::screenH, SCGUIData::screenFull);
                    }

                }
            }

            if (ImGui::CollapsingHeader("Camera Info", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::InputFloat("Game Camera FOV", &SCGUIData::sysCamFov);
                ImGui::InputFloat3("Game Camera Pos (x, y, z)", &SCGUIData::sysCamPos.x);
                ImGui::InputFloat3("Game Camera LookAt (x, y, z)", &SCGUIData::sysCamLookAt.x);
                ImGui::InputFloat4("Game Camera Rotation (w, x, y, z)", &SCGUIData::sysCamRot.w);

                if(ImGui::CollapsingHeader("Free Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Checkbox("Enable Free Camera", &g_enable_free_camera);
                    INPUT_AND_SLIDER_FLOAT("Move Speed", &BaseCamera::moveStep, 0.0f, 0.5f);
                    INPUT_AND_SLIDER_FLOAT("Mouse Speed", &g_free_camera_mouse_speed, 0.0f, 100.0f);
                    INPUT_AND_SLIDER_FLOAT("Camera FOV", &SCCamera::baseCamera.fov, 0.0f, 360.0f);
                    ImGui::InputFloat3("Camera Pos (x, y, z)", &SCCamera::baseCamera.pos.x);
                    ImGui::InputFloat3("Camera LookAt (x, y, z)", &SCCamera::baseCamera.lookAt.x);
                }
            }

        }
        ImGui::End();
	}
}
