#include "imgui/imgui.h"
#include "stdinclude.hpp"
#include "scgui/scGUIData.hpp"

extern void* SetResolution_orig;

namespace SCGUILoop {
	void mainLoop() {
        if (ImGui::Begin("SC Plugin Config")) {
            ImGui::Checkbox("Waiting Extract Text", &SCGUIData::needExtractText);

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

        }
        ImGui::End();
	}
}
