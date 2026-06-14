#pragma once

#define NOMINMAX

#include <Windows.h>
#include <shlobj.h>

#include <cinttypes>
#include <cstddef>

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

#define KEY_W  87
#define KEY_S  83
#define KEY_A  65
#define KEY_D  68
#define KEY_Q  81
#define KEY_E  69
#define KEY_R  82
#define KEY_UP  38
#define KEY_DOWN  40
#define KEY_LEFT  37
#define KEY_RIGHT  39
#define KEY_CTRL  17
#define KEY_SHIFT  16
#define KEY_ALT  18
#define KEY_SPACE  32
#define KEY_192 192

#ifdef __SAFETYHOOK
#include <safetyhook.hpp>
#define HOOK_ORIG_TYPE SafetyHookInline
#define HOOK_GET_ORIG(_name_) (_name_##_orig.original<void*>())
// _ret_type_ should follow the function's signature, or minhook side will report error
#define HOOK_CAST_CALL(_ret_type_, _name_) _name_##_orig.call<_ret_type_>
#else
#define HOOK_ORIG_TYPE void*
#define HOOK_GET_ORIG(_name_) (_name_##_orig)
#define HOOK_CAST_CALL(_ret_type_, _name_) (reinterpret_cast<decltype(_name_##_hook)*>(_name_##_orig))
#endif
#define HOOK_DEF(_return_type_, _name_) HOOK_ORIG_TYPE _name_##_orig; _return_type_ _name_##_hook


#define PRINT(var) std::cout << #var << " = " << var << std::endl;
#define PRINT_ONCE(_txt_var_) static bool __print_once_##_txt_var_ = [] { PRINT(_txt_var_); return true; }();
LONG WINAPI seh_filter(EXCEPTION_POINTERS* ep);
#define __EXCEPT() __except (seh_filter(GetExceptionInformation())) { }
#define __EXCEPT(strContext) __except (seh_filter(GetExceptionInformation())) { std::cout << "SEH exception detected in '" << strContext << "'.\n"; }


namespace debug {
	void DumpRelationMemoryHex(const void* target, const size_t length = 0x40);
	void DumpRegisters();

	const std::vector<const MethodInfo*>& GetManagedMethodTable();
	const MethodInfo* ResolveAddress(uintptr_t pc);
	std::string FormatMethodInfo(const MethodInfo* method);

	void PrintManagedStackTrace(ULONG framesToSkip = 0, ULONG framesToCapture = 64);
}

// @return remove the callback after call or not; true to remove, false to keep.
extern std::vector<std::function<bool()>> mainThreadTasks;

bool WriteClipboard(std::string& text);
bool ReadClipboard(std::string* text);

struct LocalTransform {
	const Il2CppObject* transform{};
	Vector3_t localPosition{};
	Quaternion_t localRotation{};
	Vector3_t localScale{};

	LocalTransform() {}
	LocalTransform(const Il2CppObject* transform, bool readTransform);
	LocalTransform(const Il2CppObject* transform, Vector3_t localPosition, Quaternion_t localRotation, Vector3_t localScale);

	void ReadLocalPosition(const Il2CppObject* transform);
	void ReadLocalRotation(const Il2CppObject* transform);
	void ReadLocalScale(const Il2CppObject* transform);

	void WriteLocalPosition(Il2CppObject* transform);
	void WriteLocalRotation(Il2CppObject* transform);
	void WriteLocalScale(Il2CppObject* transform);
};
extern std::unordered_map<Il2CppObject*, std::unique_ptr<LocalTransform>> transformOverriding;
// pair { first = displayName, second = jsonText }
extern std::vector<std::pair<std::string, std::string>> savedTransformOverridingJson;


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

	// get the `MstCostume` FieldInfo if the given managed object contains such field; otherwise, returns nullptr
	static FieldInfo* GetMstCostumeField(managed::UnitIdol* managed) {
		return il2cpp_class_get_field_from_name(
			il2cpp_symbols::get_class_from_instance(managed),
			"<MstCostume>k__BackingField"
		);
	}

	int CharaId = -1;
	int ClothId = 0;
	int HairId = 0;
	int* AccessoryIds = nullptr;
	int AccessoryIdsLength = 0;
	int MstCostumeId = -1;

	void ReadFrom(managed::UnitIdol* managed);
	void ApplyTo(managed::UnitIdol* managed);

	void Clear();
	bool IsEmpty() const;
	void Print(std::ostream& os) const;
	std::string ToString() const;
	void LoadJson(const char* json);
};

namespace managed::MagicaCloth2 {
	struct InertiaConstraintSerializeData : Il2CppObject {
		// Transform
		void* anchor;
		// [Range(0f, 1f)]
		float anchorInertia;
		// [FormerlySerializedAs("movementInertia")]; [Range(0f, 1f)]
		float worldInertia;
		// [Range(0f, 1f)]
		float movementInertiaSmoothing;
		// CheckSliderSerializeData
		void* movementSpeedLimit;
		// CheckSliderSerializeData
		void* rotationSpeedLimit;
		// [Range(0f, 1f)]
		float localInertia;
		// CheckSliderSerializeData
		void* localMovementSpeedLimit;
		// CheckSliderSerializeData
		void* localRotationSpeedLimit;
		// [Range(0f, 1f)]
		float depthInertia;
		// [Range(0f, 1f)]
		float centrifualAcceleration;
		// CheckSliderSerializeData
		void* particleSpeedLimit;
		// enum TeleportMode { None, Reset, Keep }
		int32_t teleportMode;
		float teleportDistance;
		float teleportRotation;
	};

	struct CurveSerializeData : Il2CppObject {
		float value;
		bool useCurve;
		// AnimationCurve
		void* curve;
	};
	static_assert(offsetof(CurveSerializeData, value) == 0x10);
	static_assert(offsetof(CurveSerializeData, useCurve) == 0x14);
	static_assert(offsetof(CurveSerializeData, curve) == 0x18);

	struct CheckSliderSerializeData : Il2CppObject {
		float value;
		bool use;
	};
	static_assert(offsetof(CheckSliderSerializeData, value) == 0x10);
	static_assert(offsetof(CheckSliderSerializeData, use) == 0x14);

	struct SpringConstraintSerializeData : Il2CppObject {
		bool useSpring;
		float springPower;
		float limitDistance;
		float normalLimitRatio;
		float springNoise;
	};
	static_assert(offsetof(SpringConstraintSerializeData, useSpring) == 0x10);
	static_assert(offsetof(SpringConstraintSerializeData, springPower) == 0x14);
	static_assert(offsetof(SpringConstraintSerializeData, limitDistance) == 0x18);
	static_assert(offsetof(SpringConstraintSerializeData, normalLimitRatio) == 0x1C);
	static_assert(offsetof(SpringConstraintSerializeData, springNoise) == 0x20);

	struct BodyParamFloatProperty : Il2CppObject {
		bool IsEnable;
		float MinValue;
		float MaxValue;
	};
}


enum class ClothForceMode {
	None = 0,
	VelocityAdd = 1,
	VelocityChange = 2,
	VelocityAddWithoutDepth = 10,
	VelocityChangeWithoutDepth = 11
};


// @return (const Il2CppObject* gameObject, const Il2CppObject* transform)[]
std::vector<std::pair<const Il2CppObject*, const Il2CppObject*>> GetActiveIdolObjects();

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
extern bool g_show_hidden_costumes;
extern bool g_save_and_replace_costume_changes;
extern bool g_overrie_mv_unit_idols;
extern bool g_apply_costumes_automatically;
extern bool g_override_isVocalSeparatedOn;
extern bool g_enable_chara_param_edit;
extern float g_font_size_offset;
extern float g_3d_resolution_scale;
extern bool g_unlock_PIdol_and_SChara_events;
extern int g_start_resolution_w;
extern int g_start_resolution_h;
extern bool g_start_resolution_fullScreen;
extern bool g_reenable_clipPlane;
extern float g_nearClipPlane;
extern float g_farClipPlane;
extern bool g_shader_quickprobing;
extern bool g_shader_unsafe_probing;
extern bool g_loadasset_output;
extern bool g_extract_asset;
extern bool g_extract_asset_image;
extern bool g_extract_asset_rawimage;
extern bool g_extract_asset_renderer;
extern bool g_extract_asset_sprite;
extern bool g_extract_asset_texture2d;
extern bool g_extract_asset_log_unknown_asset;
extern bool g_magicacloth_override;
extern bool g_magicacloth_output_cloth;
extern bool g_magicacloth_output_controller;
extern float g_magicacloth_inertia_min;
extern float g_magicacloth_inertia_max;
extern float g_magicacloth_radius_min;
extern float g_magicacloth_radius_max;
extern float g_magicacloth_damping;
extern float g_magicacloth_movementSpeedLimit;
extern float g_magicacloth_rotationSpeedLimit;
extern float g_magicacloth_localMovementSpeedLimit;
extern float g_magicacloth_localRotationSpeedLimit;
extern float g_magicacloth_particleSpeedLimit;
extern float g_magicacloth_limitAngle;
extern float g_magicacloth_springLimitDistance;
extern float g_magicacloth_springNoise;


namespace tools {
	extern bool output_networking_calls;
	extern void AddNetworkingHooks();
	extern void BuildCallingRelations();
}