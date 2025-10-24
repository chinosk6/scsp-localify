#pragma once

#define NOMINMAX

#include <Windows.h>
#include <shlobj.h>

#include <cinttypes>

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <locale>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <thread>
#include <variant>

#include <exception>
#include <vector>
#include <regex>

#include <MinHook.h>

#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include "il2cpp/il2cpp_symbols.hpp"
#include "reflection.hpp"

#include <nlohmann/json.hpp>
#include <cpprest/uri.h>
#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>
#include <string>

#include "local/local.hpp"
#include "camera/camera.hpp"

#ifdef __SAFETYHOOK
#include <safetyhook.hpp>
#define HOOK_ORIG_TYPE SafetyHookInline
#define HOOK_GET_ORIG(_name_) (_name_##_orig.original<void*>())
#define HOOK_CAST_CALL(_ret_type_, _name_) _name_##_orig.call<_ret_type_>
#else
#define HOOK_ORIG_TYPE void*
#define HOOK_GET_ORIG(_name_) (_name_##_orig)
#define HOOK_CAST_CALL(_ret_type_, _name_, ...) (reinterpret_cast<decltype(_name_##_hook)*>(_name_##_orig))
#endif


class CharaParam_t {
public:
	CharaParam_t(float height, float bust, float head, float arm, float hand) :
		height(height), bust(bust), head(head), arm(arm), hand(hand) {
		objPtr = NULL;
		updateInitParam();
	}

	CharaParam_t(float height, float bust, float head, float arm, float hand, void* objPtr) :
		height(height), bust(bust), head(head), arm(arm), hand(hand), objPtr(objPtr) {
		updateInitParam();
	}

	void UpdateParam(float* height, float* bust, float* head, float* arm, float* hand) const {
		*height = this->height;
		*bust = this->bust;
		*head = this->head;
		*arm = this->arm;
		*hand = this->hand;
	}

	void SetObjPtr(void* ptr) {
		objPtr = ptr;
	}

	bool checkObjAlive() {
		if (!objPtr) return false;
		static auto Object_IsNativeObjectAlive = reinterpret_cast<bool(*)(void*)>(
			il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine",
				"Object", "IsNativeObjectAlive", 1)
			);
		const auto ret = Object_IsNativeObjectAlive(objPtr);
		if (!ret) objPtr = NULL;
		return ret;
	}

	void* getObjPtr() {
		checkObjAlive();
		return objPtr;
	}

	void Reset() {
		height = init_height;
		bust = init_bust;
		head = init_head;
		arm = init_arm;
		hand = init_hand;
	}

	void Apply();
	void ApplyOnMainThread();

	float height;
	float bust;
	float head;
	float arm;
	float hand;
	bool gui_real_time_apply = false;
private:
	void updateInitParam() {
		init_height = height;
		init_bust = bust;
		init_head = head;
		init_arm = arm;
		init_hand = hand;
	}

	void* objPtr;
	float init_height;
	float init_bust;
	float init_head;
	float init_arm;
	float init_hand;
};


class CharaSwayStringParam_t {
public:
	CharaSwayStringParam_t(float rate, float P_bendStrength, float P_baseGravity,
		float P_inertiaMoment, float P_airResistance, float P_deformResistance) :
		rate(rate), P_bendStrength(P_bendStrength), P_baseGravity(P_baseGravity),
		P_inertiaMoment(P_inertiaMoment), P_airResistance(P_airResistance), P_deformResistance(P_deformResistance) {

	}

	CharaSwayStringParam_t() :
		rate(0), P_bendStrength(0), P_baseGravity(0),
		P_inertiaMoment(0), P_airResistance(0), P_deformResistance(0) {

	}

	bool isEdited() const {
		return (rate != 0 || P_bendStrength != 0 || P_baseGravity != 0 ||
			P_inertiaMoment != 0 || P_airResistance != 0 || P_deformResistance != 0);
	}

	float rate;
	float P_bendStrength;
	float P_baseGravity;
	float P_inertiaMoment;
	float P_airResistance;
	float P_deformResistance;

};


namespace managed { struct UnitIdol {}; }
struct UnitIdol {
	static void* klass_UnitIdol;
	static void* field_UnitIdol_charaId;
	static void* field_UnitIdol_clothId;
	static void* field_UnitIdol_hairId;
	static void* field_UnitIdol_accessoryIds;

	static void InitUnitIdol(void* unitIdolInstance) {
		if (field_UnitIdol_accessoryIds == nullptr) {
			klass_UnitIdol = il2cpp_symbols::get_class_from_instance(unitIdolInstance);
			field_UnitIdol_charaId = il2cpp_class_get_field_from_name(klass_UnitIdol, "charaId");
			field_UnitIdol_clothId = il2cpp_class_get_field_from_name(klass_UnitIdol, "clothId");
			field_UnitIdol_hairId = il2cpp_class_get_field_from_name(klass_UnitIdol, "hairId");
			field_UnitIdol_accessoryIds = il2cpp_class_get_field_from_name(klass_UnitIdol, "accessoryIds");
		}
	}

	int CharaId = -1;
	int ClothId = 0;
	int HairId = 0;
	int* AccessoryIds = nullptr;
	int AccessoryIdsLength = 0;

	void ReadFrom(managed::UnitIdol* managed);
	void ApplyTo(managed::UnitIdol* managed);

	void Clear();
	bool IsEmpty() const;
	void Print(std::ostream& os) const;
	std::string ToString() const;
	void LoadJson(const char* json);
};


extern std::map<int, std::string> swayTypes;
extern std::map<int, CharaSwayStringParam_t> charaSwayStringOffset;
extern std::map<int, UnitIdol> savedCostumes;
extern UnitIdol lastSavedCostume;
extern UnitIdol overridenMvUnitIdols[8];
const int overridenMvUnitIdols_length = 8;

extern std::function<void()> g_reload_all_data;
extern bool g_enable_plugin;
extern int g_max_fps;
extern int g_vsync_count;
extern bool g_enable_console;
extern bool g_auto_dump_all_json;
extern bool g_dump_untrans_lyrics;
extern bool g_dump_untrans_unlocal;
extern std::string g_custom_font_path;
extern std::filesystem::path g_localify_base;
extern char hotKey;
extern bool g_enable_free_camera;
extern bool g_block_out_of_focus;
extern float g_free_camera_mouse_speed;
extern bool g_allow_use_tryon_costume;
extern bool g_allow_same_idol;
extern bool g_unlock_all_dress;
extern bool g_unlock_all_headwear;
extern bool g_override_isCostumeUnlimited;
extern bool g_save_and_replace_costume_changes;
extern bool g_overrie_mv_unit_idols;
extern bool g_override_isVocalSeparatedOn;
extern bool g_enable_chara_param_edit;
extern float g_font_size_offset;
extern float g_3d_resolution_scale;
extern bool g_unlock_PIdol_and_SChara_events;
extern int g_start_resolution_w;
extern int g_start_resolution_h;
extern bool g_start_resolution_fullScreen;
extern bool g_reenable_clipPlane;

namespace tools {
	extern bool output_networking_calls;
	extern void AddNetworkingHooks();
}