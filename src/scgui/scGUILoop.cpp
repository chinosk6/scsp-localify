#include "imgui/imgui.h"
#include "stdinclude.hpp"
#include "scgui/scGUIData.hpp"

extern void* SetResolution_orig;
// extern std::vector<std::pair<std::pair<int, int>, int>> replaceDressResIds;
extern std::map<std::string, CharaParam_t> charaParam;
extern CharaParam_t baseParam;
extern void saveGUIDataCache();


#define INPUT_AND_SLIDER_FLOAT(label, data, min, max) \
    ImGui::SetNextItemWidth(inputFloatWidth);\
    ImGui::InputFloat("##"##label, data);\
    ImGui::SameLine();\
    ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - inputFloatWidth - 1.0f);\
    ImGui::SliderFloat(label, data, min, max)

#define FOR_INPUT_AND_SLIDER_FLOAT(label, data, min, max, hideIdName) \
    ImGui::SetNextItemWidth(inputFloatWidth);\
    ImGui::InputFloat(("##"##label + hideIdName).c_str(), data);\
    ImGui::SameLine();\
    ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - inputFloatWidth - 1.0f);\
    ImGui::SliderFloat((label##"##" + hideIdName).c_str(), data, min, max)

#define HELP_TOOLTIP(label, text) \
    ImGui::TextDisabled(label); \
    if (ImGui::IsItemHovered()) { \
        ImGui::BeginTooltip(); \
        ImGui::Text(text); \
        ImGui::EndTooltip(); \
    }

namespace SCGUILoop {
	static float inputFloatWidth = 50.0f;

	void charaParamEditLoop() {
		if (ImGui::Begin("Character Parameter Edit")) {
			if (ImGui::CollapsingHeader("Sway offset (Non real-time, requires reloading)")) {
				static int currentEdit = 0x7;  // BreastPointed

				if (ImGui::BeginCombo("Edit Type", swayTypes[currentEdit].c_str())) {
					for (const auto& pair : swayTypes) {
						bool isSelected = (currentEdit == pair.first);
						if (ImGui::Selectable(pair.second.c_str(), isSelected)) {
							currentEdit = pair.first;
						}
						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				auto& currEditData = charaSwayStringOffset[currentEdit];

				INPUT_AND_SLIDER_FLOAT("Rate##sway", &currEditData.rate, -1.5, 1.5);
				INPUT_AND_SLIDER_FLOAT("BendStrength##sway", &currEditData.P_bendStrength, -5, 5.0);
				INPUT_AND_SLIDER_FLOAT("BaseGravity##sway", &currEditData.P_baseGravity, -50, 50.0);
				INPUT_AND_SLIDER_FLOAT("InertiaMoment##sway", &currEditData.P_inertiaMoment, -2, 2.0);
				INPUT_AND_SLIDER_FLOAT("AirResistance##sway", &currEditData.P_airResistance, -2, 2.0);
				INPUT_AND_SLIDER_FLOAT("DeformResistance##sway", &currEditData.P_deformResistance, -30, 30.0);

				if (ImGui::Button("Reset##sway")) {
					currEditData.rate = 0;
					currEditData.P_bendStrength = 0;
					currEditData.P_baseGravity = 0;
					currEditData.P_inertiaMoment = 0;
					currEditData.P_airResistance = 0;
					currEditData.P_deformResistance = 0;
				}
				ImGui::SameLine();
				if (ImGui::Button("Save##sway")) {
					saveGUIDataCache();
				}
				ImGui::NewLine();
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("SwayString 偏移调整。加载角色时生效，非实时生效。");
				ImGui::EndTooltip();
			}

			bool baseApply = false;
			bool resetAll = false;
			if (ImGui::CollapsingHeader("Base Offset", ImGuiTreeNodeFlags_DefaultOpen)) {
				INPUT_AND_SLIDER_FLOAT("Height##base", &baseParam.height, -1.5, 1.5);
				INPUT_AND_SLIDER_FLOAT("Bust##base", &baseParam.bust, -1.5, 1.5);
				INPUT_AND_SLIDER_FLOAT("Head##base", &baseParam.head, -1.5, 1.5);
				INPUT_AND_SLIDER_FLOAT("Arm##base", &baseParam.arm, -1.5, 1.5);
				INPUT_AND_SLIDER_FLOAT("Hand##base", &baseParam.hand, -1.5, 1.5);
				if (ImGui::Button("Apply##base")) {
					baseApply = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("Reset Base##base")) {
					baseParam.Reset();
					baseApply = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("Reset All##base")) {
					baseParam.Reset();
					resetAll = true;
					baseApply = true;
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("Real-time Update##base", &baseParam.gui_real_time_apply)) {
					if (baseParam.gui_real_time_apply) {
						baseApply = true;
					}
				}
			}

			for (auto& i : charaParam) {
				if (i.second.checkObjAlive()) {
					if (ImGui::CollapsingHeader(i.first.c_str())) {
						FOR_INPUT_AND_SLIDER_FLOAT("Height", &i.second.height, 0.0, 2.0, i.first);
						FOR_INPUT_AND_SLIDER_FLOAT("Bust", &i.second.bust, 0.0, 2.0, i.first);
						FOR_INPUT_AND_SLIDER_FLOAT("Head", &i.second.head, 0.0, 2.0, i.first);
						FOR_INPUT_AND_SLIDER_FLOAT("Arm", &i.second.arm, 0.0, 2.0, i.first);
						FOR_INPUT_AND_SLIDER_FLOAT("Hand", &i.second.hand, 0.0, 2.0, i.first);
						if (ImGui::Button(("Apply##" + i.first).c_str()) || baseApply) {
							i.second.ApplyOnMainThread();
						}
						ImGui::SameLine();
						if (ImGui::Button(("Reset##" + i.first).c_str()) || resetAll) {
							i.second.Reset();
							i.second.ApplyOnMainThread();
						}
						ImGui::SameLine();
						if (ImGui::Checkbox(("Real-time Update##" + i.first).c_str(), &i.second.gui_real_time_apply)) {
							if (i.second.gui_real_time_apply) {
								i.second.ApplyOnMainThread();
							}
						}
					}
					else {
						if (baseApply) {
							if (resetAll) {
								i.second.Reset();
							}
							i.second.ApplyOnMainThread();
						}
					}
				}
			}
		}
		ImGui::End();
	}

	void savedCostumeDataLoop() {
		if (ImGui::Begin("Saved Costume Data")) {
			for (auto it = savedCostumes.begin(); it != savedCostumes.end(); ) {
				char label[32];

				std::snprintf(label, sizeof(label), "%d", it->first);
				ImGui::Text(label);
				ImGui::SameLine();

				std::snprintf(label, sizeof(label), "%s##%s%d", "Remove", "scd", it->first);
				auto strUnitIdol = it->second.ToString();

				if (ImGui::Button(label)) {
					it = savedCostumes.erase(it);
				} else {
					++it;
				}

				ImGui::SameLine();
				ImGui::Text(strUnitIdol.c_str());
			}
		}
		ImGui::End();
	}

	int editingOverrideMvUnitIdolSlot = -1;
	char inputOverrideMvUnitIdol[1024] = ""; 
	void overrideMvUnitIdolLoop() {
		if (ImGui::Begin("Override MvUnit Idols")) {
			for (int i = 0; i < overridenMvUnitIdols_length; ++i) {
				char label[1024];

				std::snprintf(label, sizeof(label), "%s##%s%d", "Remove", "omui", i);
				if (ImGui::Button(label)) {
					overridenMvUnitIdols[i].Clear();
				}
				ImGui::SameLine();
				std::snprintf(label, sizeof(label), "%s%d##%s", "Slot ", i, "omui");
				if (ImGui::Button(label)) {
					if (lastSavedCostume.IsEmpty()) {
						printf("No costume data saved yet.\n");
					}
					else {
						overridenMvUnitIdols[i] = lastSavedCostume;
					}
				}
				ImGui::SameLine();
				auto strIdolData = overridenMvUnitIdols[i].ToString();
				std::snprintf(label, sizeof(label), "%s##%d", strIdolData.c_str(), i);
				if (ImGui::Button(label)) {
					editingOverrideMvUnitIdolSlot = i;
					std::strncpy(inputOverrideMvUnitIdol, strIdolData.c_str(), sizeof(inputOverrideMvUnitIdol) - 1);
					inputOverrideMvUnitIdol[sizeof(inputOverrideMvUnitIdol) - 1] = '\0'; // Ensure null-termination
				}
			}
		}
		ImGui::End();

		if (editingOverrideMvUnitIdolSlot >= 0) {
			ImGui::OpenPopup("InputManuallyOverrideMvUnitIdol");
		}
		if (ImGui::BeginPopupModal("InputManuallyOverrideMvUnitIdol", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Set override data manually: (in JSON)");
			ImGui::InputText("##NameInput", inputOverrideMvUnitIdol, IM_ARRAYSIZE(inputOverrideMvUnitIdol));

			if (ImGui::Button("OK", ImVec2(120, 0))) {
				if (editingOverrideMvUnitIdolSlot >= 0 && editingOverrideMvUnitIdolSlot < 8) {
					overridenMvUnitIdols[editingOverrideMvUnitIdolSlot].LoadJson(inputOverrideMvUnitIdol);
				}
				else {
					printf("ArgumentOutOfRangeException at `overridenMvUnitIdols[editingOverrideMvUnitIdolSlot]`; editingOverrideMvUnitIdolSlot = %d.\n", editingOverrideMvUnitIdolSlot);
				}
				editingOverrideMvUnitIdolSlot = -1;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				editingOverrideMvUnitIdolSlot = -1;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void mainLoop() {
		if (ImGui::Begin("SC Plugin Config")) {
			if (ImGui::Button("Reload Config And Translate Data")) {
				g_reload_all_data();
			}
			ImGui::Checkbox("Waiting Extract Text", &SCGUIData::needExtractText);

			ImGui::Checkbox("Save & Replace costume changes", &g_save_and_replace_costume_changes);
			ImGui::SameLine();
			HELP_TOOLTIP("(?)", "保存服装编辑信息并在MV播放时替换。\nSave costumes changing data and Replace when MV starts.");
			
			if (g_save_and_replace_costume_changes) {
				ImGui::Indent(30);
				ImGui::Checkbox("Override MV unit idols", &g_overrie_mv_unit_idols);
				ImGui::SameLine();
				HELP_TOOLTIP("(?)", "在操作窗口中保存用于替换MV播放时的角色信息。\nSave idols' data in control panel to replace them when playing MV.");

				ImGui::Checkbox("Show hidden costumes (override isCostumeUnlimited)", &g_override_isCostumeUnlimited);
				ImGui::SameLine();
				HELP_TOOLTIP("(?)", "在进入换装窗口前勾选有效。\nActive only when checked before entering costume changing view.");
				ImGui::Unindent(30);
			}

			ImGui::Checkbox("Enable VocalSeparatedOn forcibly", &g_override_isVocalSeparatedOn);
			ImGui::SameLine();
			HELP_TOOLTIP("(?)", "若最终播放MV时的歌曲或所选角色不支持分段演唱则会导致卡死或崩溃。\nGame will freeze or crash if finally selected song or idols don't support separated vocal.");

			ImGui::Checkbox("[Legacy] Live Allow Same Idol (Dangerous)", &g_allow_same_idol);
			ImGui::SameLine();
			HELP_TOOLTIP("(?)", "影分身术！\n允许在 Live 中选择同一人。\n（此模式的编组数据会上传，请小心你的号）")

			ImGui::Checkbox("Enable Character Parameter Editor", &g_enable_chara_param_edit);
			ImGui::SameLine();
			HELP_TOOLTIP("(?)", "启用角色身体参数编辑器")

			ImGui::Checkbox("Unlock PIdol And SChara Events", &g_unlock_PIdol_and_SChara_events);
			ImGui::SameLine();
			HELP_TOOLTIP("(?)", "解锁 角色 - 一览 中的P卡和S卡事件\nUnlock Idol Event (アイドルイベント) and Support Event (サポートイベント)")

				if (ImGui::CollapsingHeader("Resolution Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
					ImGui::Text("Window Resolution Settings");

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

					ImGui::Separator();

					INPUT_AND_SLIDER_FLOAT("3D Resolution Scale", &g_3d_resolution_scale, 0.1f, 5.0f);
					if (g_3d_resolution_scale == 1.0f) {
						SCCamera::currRenderResolution.x = SCGUIData::screenW;
						SCCamera::currRenderResolution.y = SCGUIData::screenH;
					}
					ImGui::Text("Current 3D Resolution: %d, %d", SCCamera::currRenderResolution.x, SCCamera::currRenderResolution.y);
				}

			if (ImGui::CollapsingHeader("Camera Info", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::InputFloat("Game Camera FOV", &SCGUIData::sysCamFov);
				ImGui::InputFloat3("Game Camera Pos (x, y, z)", &SCGUIData::sysCamPos.x);
				ImGui::InputFloat3("Game Camera LookAt (x, y, z)", &SCGUIData::sysCamLookAt.x);
				ImGui::InputFloat4("Game Camera Rotation (w, x, y, z)", &SCGUIData::sysCamRot.w);

				if (ImGui::CollapsingHeader("Free Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
					ImGui::Checkbox("Enable Free Camera", &g_enable_free_camera);
					ImGui::Checkbox("[Legacy] Re-enable ClipPlane overriding", &g_reenable_clipPlane);
					ImGui::SameLine();
					HELP_TOOLTIP("(?)", "Re-enable the modification of nearClipPlane & farClipPlane like old versions.");
					INPUT_AND_SLIDER_FLOAT("Move Speed", &BaseCamera::moveStep, 0.0f, 0.5f);
					INPUT_AND_SLIDER_FLOAT("Mouse Speed", &g_free_camera_mouse_speed, 0.0f, 100.0f);
					INPUT_AND_SLIDER_FLOAT("Camera FOV", &SCCamera::baseCamera.fov, 0.0f, 360.0f);
					ImGui::InputFloat3("Camera Pos (x, y, z)", &SCCamera::baseCamera.pos.x);
					ImGui::InputFloat3("Camera LookAt (x, y, z)", &SCCamera::baseCamera.lookAt.x);
				}
			}

			if (ImGui::CollapsingHeader("Devs", ImGuiTreeNodeFlags_None)) {
#ifdef __TOOL_HOOK_NETWORKING__
				ImGui::Checkbox("Output networking calls", &tools::output_networking_calls);
#endif
			}

		}
		ImGui::End();
		if (g_enable_chara_param_edit) charaParamEditLoop();
		if (g_save_and_replace_costume_changes) savedCostumeDataLoop();
		if (g_save_and_replace_costume_changes && g_overrie_mv_unit_idols) overrideMvUnitIdolLoop();
	}
}
