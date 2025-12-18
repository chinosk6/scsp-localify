#include "imgui/imgui.h"
#include "stdinclude.hpp"
#include "scgui/scGUIData.hpp"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

extern HOOK_ORIG_TYPE SetResolution_orig;
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

	struct FreeCamState {
		bool hasData = false;
		std::string name = "";
		float fov = 0.0f;

		struct { float x, y, z; } pos = { 0,0,0 };
		struct { float x, y, z; } lookAt = { 0,0,0 };
	};

	rapidjson::Value SerializeFreeCamState(const FreeCamState& state, rapidjson::Document::AllocatorType& allocator) {
		rapidjson::Value obj(rapidjson::kObjectType);
		obj.AddMember("hasData", rapidjson::Value(state.hasData), allocator);
		if (!state.name.empty()) {
			obj.AddMember("name", rapidjson::Value(state.name.c_str(), allocator), allocator);
		}
		if (state.hasData) {
			obj.AddMember("fov", rapidjson::Value(state.fov), allocator);

			rapidjson::Value valPos(rapidjson::kArrayType);
			valPos.PushBack(state.pos.x, allocator);
			valPos.PushBack(state.pos.y, allocator);
			valPos.PushBack(state.pos.z, allocator);
			obj.AddMember("pos", valPos, allocator);

			rapidjson::Value valLookAt(rapidjson::kArrayType);
			valLookAt.PushBack(state.lookAt.x, allocator);
			valLookAt.PushBack(state.lookAt.y, allocator);
			valLookAt.PushBack(state.lookAt.z, allocator);
			obj.AddMember("lookAt", valLookAt, allocator);
		}
		return obj;
	}
	FreeCamState DeserializeFreeCameraState(const rapidjson::Value& v) {
		FreeCamState state;
		if (v.HasMember("hasData") && v["hasData"].IsBool()) state.hasData = v["hasData"].GetBool();
		if (v.HasMember("name") && v["name"].IsString()) state.name = v["name"].GetString();
		if (state.hasData) {
			if (v.HasMember("fov") && v["fov"].IsNumber()) state.fov = v["fov"].GetFloat();
			if (v.HasMember("pos") && v["pos"].IsArray()) {
				const auto& arr = v["pos"].GetArray();
				auto length = arr.Size();
				if (length > 0 && arr[0].IsNumber()) state.pos.x = arr[0].GetFloat();
				if (length > 1 && arr[1].IsNumber()) state.pos.y = arr[1].GetFloat();
				if (length > 2 && arr[2].IsNumber()) state.pos.z = arr[2].GetFloat();
			}
			if (v.HasMember("lookAt") && v["lookAt"].IsArray()) {
				const auto& arr = v["lookAt"].GetArray();
				auto length = arr.Size();
				if (length > 0 && arr[0].IsNumber()) state.lookAt.x = arr[0].GetFloat();
				if (length > 1 && arr[1].IsNumber()) state.lookAt.y = arr[1].GetFloat();
				if (length > 2 && arr[2].IsNumber()) state.lookAt.z = arr[2].GetFloat();
			}
		}
		return state;
	}


	static std::vector<FreeCamState> freeCamSlots(1);
	char newFreeCamSlotName[32]{};

	std::string messageBoxContent{};
	void ShowMessageBox(const std::string& message) {
		messageBoxContent = message;
		ImGui::OpenPopup("MessageBox");
	}

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
				}
				else {
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

				ImGui::Checkbox("Show hidden costumes (override IsAllDressOrdered)", &g_show_hidden_costumes);
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

			ImGui::Checkbox("Unlock Stories", &g_unlock_PIdol_and_SChara_events);

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
						(reinterpret_cast<void (*)(int, int, bool)>HOOK_GET_ORIG(SetResolution))(SCGUIData::screenW, SCGUIData::screenH, SCGUIData::screenFull);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Swap")) {
					int temp = SCGUIData::screenW;
					SCGUIData::screenW = SCGUIData::screenH;
					SCGUIData::screenH = temp;
					if (SetResolution_orig) {
						(reinterpret_cast<void (*)(int, int, bool)>HOOK_GET_ORIG(SetResolution))(SCGUIData::screenW, SCGUIData::screenH, SCGUIData::screenFull);
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

					ImGui::Separator();
					ImGui::Text("Save Camera State:");

					// export, append; remove all
					ImGui::Dummy(ImVec2(40, 0));
					ImGui::SameLine();
					if (ImGui::Button("Export to clipboard")) {
						rapidjson::Document doc;
						doc.SetArray();
						auto& allocator = doc.GetAllocator();
						for (auto& state : freeCamSlots) {
							doc.PushBack(SerializeFreeCamState(state, allocator), allocator);
						}
						rapidjson::StringBuffer buffer;
						rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
						doc.Accept(writer);
						std::string serialized(buffer.GetString());
						auto r = WriteClipboard(serialized);
						ShowMessageBox((r ? "Success" : "Failed"));
					}
					ImGui::SameLine();
					if (ImGui::Button("Append from clipboard")) {
						std::string text;
						if (!ReadClipboard(&text)) {
							ShowMessageBox("Failed to read clipboard.");
						}
						else {
							rapidjson::Document doc;
							doc.Parse(text.c_str());
							if (doc.IsArray()) {
								const auto& arr = doc.GetArray();
								for (auto& value : arr) {
									auto parsed = DeserializeFreeCameraState(value);
									freeCamSlots.emplace_back(parsed);
								}
							}
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Remove all saved states")) {
						freeCamSlots.clear();
					}

					// Camera Slot
					for (int i = 0; i < freeCamSlots.size(); ++i) {
						ImGui::PushID(i);

						// --- Save button ---
						if (ImGui::Button("Save")) {
							freeCamSlots[i].hasData = true;
							freeCamSlots[i].fov = SCCamera::baseCamera.fov;

							freeCamSlots[i].pos.x = SCCamera::baseCamera.pos.x;
							freeCamSlots[i].pos.y = SCCamera::baseCamera.pos.y;
							freeCamSlots[i].pos.z = SCCamera::baseCamera.pos.z;

							freeCamSlots[i].lookAt.x = SCCamera::baseCamera.lookAt.x;
							freeCamSlots[i].lookAt.y = SCCamera::baseCamera.lookAt.y;
							freeCamSlots[i].lookAt.z = SCCamera::baseCamera.lookAt.z;
						}
						ImGui::SameLine();

						// --- SLOT BUTTON ---
						std::string slotName;
						if (!freeCamSlots[i].name.empty()) {
							slotName = freeCamSlots[i].name;
						}
						else {
							char slotNameBuffer[32];
							snprintf(slotNameBuffer, 32, "Slot %d\0", i + 1);
							slotName = slotNameBuffer;
						}

						if (freeCamSlots[i].hasData) {
							ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.0f, 0.4f, 0.8f, 1.0f)); // Blue
						}
						else {
							ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 1.0f)); // Grey
						}

						if (ImGui::Button(slotName.c_str(), ImVec2(80, 0))) {
							if (freeCamSlots[i].hasData) {
								SCCamera::baseCamera.fov = freeCamSlots[i].fov;

								SCCamera::baseCamera.pos.x = freeCamSlots[i].pos.x;
								SCCamera::baseCamera.pos.y = freeCamSlots[i].pos.y;
								SCCamera::baseCamera.pos.z = freeCamSlots[i].pos.z;

								SCCamera::baseCamera.lookAt.x = freeCamSlots[i].lookAt.x;
								SCCamera::baseCamera.lookAt.y = freeCamSlots[i].lookAt.y;
								SCCamera::baseCamera.lookAt.z = freeCamSlots[i].lookAt.z;
							}
						}
						ImGui::PopStyleColor();

						// --- Clear button ---
						ImGui::SameLine();
						if (ImGui::Button("Clear")) {
							freeCamSlots[i].hasData = false;
							freeCamSlots[i].name = "";
						}

						// --- Remove button ---
						ImGui::SameLine();
						if (ImGui::Button("Remove")) {
							freeCamSlots.erase(freeCamSlots.begin() + i);
						}

						// --- Rename button ---
						ImGui::SameLine();
						if (ImGui::Button("Rename")) {
							std::strncpy(newFreeCamSlotName, slotName.c_str(), sizeof(newFreeCamSlotName));
							newFreeCamSlotName[sizeof(newFreeCamSlotName) - 1] = '\0';
							ImGui::OpenPopup("RenameFreecamstateSlot");
						}
						if (ImGui::BeginPopupModal("RenameFreecamstateSlot", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
							ImGui::Text("New name:  (up to 31 bytes)");
							ImGui::InputText("##FcsSlotNameInput", newFreeCamSlotName, IM_ARRAYSIZE(newFreeCamSlotName));

							if (ImGui::Button("OK", ImVec2(120, 0))) {
								freeCamSlots[i].name = newFreeCamSlotName;
								ImGui::CloseCurrentPopup();
							}
							ImGui::SameLine();
							if (ImGui::Button("Cancel", ImVec2(120, 0))) {
								editingOverrideMvUnitIdolSlot = -1;
								ImGui::CloseCurrentPopup();
							}

							ImGui::EndPopup();
						}

						ImGui::PopID();
					}

					// --- Add New Slot Button ---
					ImGui::Dummy(ImVec2(40, 0));
					ImGui::SameLine();
					if (ImGui::Button("Add New Slot")) {
						freeCamSlots.emplace_back();
					}
				}
			}

			if (ImGui::CollapsingHeader("Assets"), ImGuiTreeNodeFlags_DefaultOpen) {
				ImGui::Checkbox("Use quick probing for unknown shaders", &g_shader_quickprobing);
				ImGui::SameLine();
				HELP_TOOLTIP("(?)", "对不认识的渲染程序启用快速探测。\nUse quick probing for unknwon shaders. (quick upper: 8192)");
				ImGui::Checkbox("Output asset names", &g_loadasset_output);
				ImGui::SameLine();
				HELP_TOOLTIP("(?)", "在资源加载时输出资源和资源包的名称。\nOutput assets' name and AssetBundle's name when loaded.");
				ImGui::Checkbox("Extract assets of:", &g_extract_asset);
				ImGui::SameLine();
				HELP_TOOLTIP("(?)", "启用资源提取功能。注意：此选项本身不进行提取。\nEnable the process of extracting assets.\nNote: This option itself doesn't extract anything.");

				if (g_extract_asset) {
					ImGui::Checkbox("Renderer", &g_extract_asset_renderer);
					ImGui::SameLine();
					HELP_TOOLTIP("(?)", "此选项将提取所有Renderer类型子类的资源。\nThis option will extract all assets inheriting class Renderer.");
					ImGui::SameLine();
					ImGui::Checkbox("Texture2D", &g_extract_asset_texture2d);
					ImGui::SameLine();
					HELP_TOOLTIP("(?)", "此选项只提取Texture2D类型对象。\nThis option only extract instances of class Texture2D.");
					ImGui::SameLine();
					ImGui::Checkbox("Image", &g_extract_asset_image);
					ImGui::SameLine();
					ImGui::Checkbox("RawImage", &g_extract_asset_rawimage);
					ImGui::SameLine();
					ImGui::Checkbox("Sprite", &g_extract_asset_sprite);
				}
			}

			if (ImGui::CollapsingHeader("Devs", ImGuiTreeNodeFlags_None)) {
#ifdef __TOOL_HOOK_NETWORKING__
				ImGui::Checkbox("Output networking calls", &tools::output_networking_calls);
#endif
			}

		}
		if (ImGui::BeginPopupModal("MessageBox", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("%s", messageBoxContent.c_str());
			if (ImGui::Button("OK")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::End();
		if (g_enable_chara_param_edit) charaParamEditLoop();
		if (g_save_and_replace_costume_changes) savedCostumeDataLoop();
		if (g_save_and_replace_costume_changes && g_overrie_mv_unit_idols) overrideMvUnitIdolLoop();
	}
}
