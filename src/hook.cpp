#include <stdinclude.hpp>
#include <unordered_set>
#include <ranges>
#include <set>
#include <Psapi.h>
#include <intsafe.h>
#include "scgui/scGUIData.hpp"
#include <scgui/scGUIMain.hpp>
#include <mhotkey.hpp>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <boost/beast/core/detail/base64.hpp>

#define PRINT(var) std::cout << #var << " = " << var << std::endl;

LONG WINAPI seh_filter(EXCEPTION_POINTERS* ep) {
	DWORD code = ep->ExceptionRecord->ExceptionCode;
	PVOID addr = ep->ExceptionRecord->ExceptionAddress;

	std::cerr << "SEH Exception caught!" << std::endl;
	std::cerr << "  Code: 0x" << std::hex << code << std::endl;
	std::cerr << "  Address: " << addr << std::endl;

	switch (code) {
	case EXCEPTION_ACCESS_VIOLATION:
		std::cerr << "  Type: Access Violation" << std::endl;
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		std::cerr << "  Type: Divide by Zero" << std::endl;
		break;
	case EXCEPTION_STACK_OVERFLOW:
		std::cerr << "  Type: Stack Overflow" << std::endl;
		break;
	default:
		std::cerr << "  Type: Unknown SEH Exception (code=" << code << ")" << std::endl;
		break;
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

#define __EXCEPT(strContext) __except (seh_filter(GetExceptionInformation())) { printf("SEH exception detected in '" strContext "'.\n"); }

//using namespace std;

std::function<void()> g_on_hook_ready;
std::function<void()> g_on_close;
std::function<void()> on_hotKey_0;
bool needPrintStack = false;
std::vector<std::pair<std::pair<int, int>, int>> replaceDressResIds{};
std::map<std::string, CharaParam_t> charaParam{};
CharaParam_t baseParam(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
std::vector<std::function<bool()>> mainThreadTasks{};  // 返回 true，执行后移除列表；返回 false，执行后不移除

std::map<int, CharaSwayStringParam_t> charaSwayStringOffset{};
std::map<int, std::string> swayTypes{
	{0x0, "Test"},
	{0x1, "HairPointed"},
	{0x2, "HairSmooth"},
	{0x3, "HairBundle"},
	{0x4, "HairFront"},
	{0x5, "ClothPointed"},
	{0x6, "ClothSmooth"},
	{0x7, "BreastPointed"},
	{0x8, "BreastSmooth"},
	{0x9, "AccessoryBundle"},
	{0xa, "AccessoryPendulum"},
	{0xb, "ClothBundle"},
	{0xc, "HairLong"},
	{0xd, "Max"},
};

std::map<int, UnitIdol> savedCostumes{};
UnitIdol lastSavedCostume;
UnitIdol overridenMvUnitIdols[8];


void loadGUIDataCache() {
	try {
		if (!std::filesystem::exists("scsp-gui-save.json")) return;
		std::ifstream file("scsp-gui-save.json");
		if (!file.is_open()) return;
		std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		auto fileData = nlohmann::json::parse(fileContent);

		if (fileData.contains("charaSwayStringOffset")) {
			auto& swaySaveData = fileData["charaSwayStringOffset"];
			for (auto& sway : swaySaveData.items()) {
				const std::string keyStr = sway.key();
				const int key = atoi(keyStr.c_str());

				const auto& value = sway.value();
				const float rate = value["rate"];
				const float P_bendStrength = value["P_bendStrength"];
				const float P_baseGravity = value["P_baseGravity"];
				const float P_inertiaMoment = value["P_inertiaMoment"];
				const float P_airResistance = value["P_airResistance"];
				const float P_deformResistance = value["P_deformResistance"];

				if (auto it = charaSwayStringOffset.find(key); it != charaSwayStringOffset.end()) {
					it->second.rate = rate;
					it->second.P_bendStrength = P_bendStrength;
					it->second.P_baseGravity = P_baseGravity;
					it->second.P_inertiaMoment = P_inertiaMoment;
					it->second.P_airResistance = P_airResistance;
					it->second.P_deformResistance = P_deformResistance;
				}
				else {
					charaSwayStringOffset.emplace(key, CharaSwayStringParam_t(rate, P_bendStrength, P_baseGravity,
						P_inertiaMoment, P_airResistance, P_deformResistance));
				}
			}
		}
	}
	catch (std::exception& e) {
		printf("initcharaSwayStringOffset failed: %s\n\n", e.what());
	}
}

void initcharaSwayStringOffset() {
	for (auto& i : swayTypes) {
		charaSwayStringOffset.emplace(i.first, CharaSwayStringParam_t());
	}
	loadGUIDataCache();
}

void saveGUIDataCache() {
	try {
		auto saveData = nlohmann::json::object();
		saveData["charaSwayStringOffset"] = nlohmann::json::object();

		auto& swaySaveData = saveData["charaSwayStringOffset"];
		for (const auto& pair : charaSwayStringOffset) {
			const auto currKey = std::to_string(pair.first);
			swaySaveData[currKey] = nlohmann::json::object();
			swaySaveData[currKey]["rate"] = pair.second.rate;
			swaySaveData[currKey]["P_bendStrength"] = pair.second.P_bendStrength;
			swaySaveData[currKey]["P_baseGravity"] = pair.second.P_baseGravity;
			swaySaveData[currKey]["P_inertiaMoment"] = pair.second.P_inertiaMoment;
			swaySaveData[currKey]["P_airResistance"] = pair.second.P_airResistance;
			swaySaveData[currKey]["P_deformResistance"] = pair.second.P_deformResistance;
		}

		const auto saveStr = saveData.dump(4);
		std::ofstream out("scsp-gui-save.json");
		out << saveStr;
		out.close();
	}
	catch (std::exception& e) {
		printf("saveGUIDataCache error: %s\n", e.what());
	}
}

template<typename T, typename TF>
void convertPtrType(T* cvtTarget, TF func_ptr) {
	*cvtTarget = reinterpret_cast<T>(func_ptr);
}

#pragma region HOOK_MACRO
#define ADD_HOOK(_name_, _fmt_) \
	auto _name_##_offset = reinterpret_cast<void*>(_name_##_addr); \
 	\
	const auto _name_##stat1 = MH_CreateHook(_name_##_offset, _name_##_hook, &_name_##_orig); \
	const auto _name_##stat2 = MH_EnableHook(_name_##_offset); \
	printf(_fmt_##" (%s, %s)\n", _name_##_offset, MH_StatusToString(_name_##stat1), MH_StatusToString(_name_##stat2)); 
#pragma endregion

bool exd = false;
void* SetResolution_orig;
void* AssembleCharacter_ApplyParam_orig;
Il2CppString* (*environment_get_stacktrace)();


void CharaParam_t::Apply() {
	const auto ApplyParamFunc = reinterpret_cast<void (*)(void*, float, float, float, float, float)>(
		AssembleCharacter_ApplyParam_orig
		);
	auto currObjPtr = getObjPtr();
	if (currObjPtr) {
		ApplyParamFunc(currObjPtr, height + baseParam.height, bust + baseParam.bust,
			head + baseParam.head, arm + baseParam.arm, hand + baseParam.hand);
	}
}

void CharaParam_t::ApplyOnMainThread() {
	mainThreadTasks.push_back([this]() {
		this->Apply();
		if (!g_enable_chara_param_edit) return true;
		return !(this->gui_real_time_apply || baseParam.gui_real_time_apply);
		});
}


namespace Utils {
	template <typename KT = void*, typename VT = void*>
	class CSDictEditor {
	public:
		// @param dict: Dictionary instance.
		// @param dictTypeStr: Reflection type. eg: "System.Collections.Generic.Dictionary`2[System.Int32, System.Int32]"
		CSDictEditor(void* dict, const char* dictTypeStr) {
			dic_klass = il2cpp_symbols::get_system_class_from_reflection_type_str(dictTypeStr);
			this->dict = dict;

			get_Item_method = il2cpp_class_get_method_from_name(dic_klass, "get_Item", 1);
			Add_method = il2cpp_class_get_method_from_name(dic_klass, "Add", 2);
			ContainsKey_method = il2cpp_class_get_method_from_name(dic_klass, "ContainsKey", 1);

			dic_get_Item = (dic_get_Item_t)get_Item_method->methodPointer;
			dic_Add = (dic_Add_t)Add_method->methodPointer;
			dic_containsKey = (dic_containsKey_t)ContainsKey_method->methodPointer;
		}

		void Add(KT key, VT value) {
			dic_Add(dict, key, value, Add_method);
		}

		bool ContainsKey(KT key) {
			return dic_containsKey(dict, key, ContainsKey_method);
		}

		VT get_Item(KT key) {
			return dic_get_Item(dict, key, get_Item_method);
		}

		VT operator[] (KT key) {
			return get_Item(key);
		}

		void* dict;
		void* dic_klass;

	private:
		typedef VT(*dic_get_Item_t)(void*, KT, void* mtd);
		typedef VT(*dic_Add_t)(void*, KT, VT, void* mtd);
		typedef VT(*dic_containsKey_t)(void*, KT, void* mtd);

		CSDictEditor();
		MethodInfo* get_Item_method;
		MethodInfo* Add_method;
		MethodInfo* ContainsKey_method;
		dic_get_Item_t dic_get_Item;
		dic_Add_t dic_Add;
		dic_containsKey_t dic_containsKey;
	};

	template <typename T = void*>
	class CSListEditor {
	public:
		CSListEditor(void* list) {
			list_klass = il2cpp_symbols::get_class_from_instance(list);
			lst = list;

			lst_get_Count_method = il2cpp_class_get_method_from_name(list_klass, "get_Count", 0);
			lst_get_Item_method = il2cpp_class_get_method_from_name(list_klass, "get_Item", 1);
			lst_Add_method = il2cpp_class_get_method_from_name(list_klass, "Add", 1);

			lst_get_Count = reinterpret_cast<lst_get_Count_t>(lst_get_Count_method->methodPointer);
			lst_get_Item = reinterpret_cast<lst_get_Item_t>(lst_get_Item_method->methodPointer);
			lst_Add = reinterpret_cast<lst_Add_t>(lst_Add_method->methodPointer);
		}

		void Add(T value) {
			lst_Add(lst, value, lst_Add_method);
		}

		T get_Item(int index) {
			return lst_get_Item(lst, index, lst_get_Item_method);
		}

		int get_Count() {
			return lst_get_Count(lst, lst_get_Count_method);
		}

		T operator[] (int key) {
			return get_Item(key);
		}

		class Iterator {
		public:
			Iterator(CSListEditor<T>* editor, int index) : editor(editor), index(index) {}

			T operator*() const {
				return editor->get_Item(index);
			}

			Iterator& operator++() {
				++index;
				return *this;
			}

			bool operator!=(const Iterator& other) const {
				return index != other.index;
			}

		private:
			CSListEditor<T>* editor;
			int index;
		};

		Iterator begin() {
			return Iterator(this, 0);
		}

		Iterator end() {
			return Iterator(this, get_Count());
		}

		void* lst;
		void* list_klass;
	private:
		typedef T(*lst_get_Item_t)(void*, int, void* mtd);
		typedef void(*lst_Add_t)(void*, T, void* mtd);
		typedef int(*lst_get_Count_t)(void*, void* mtd);

		MethodInfo* lst_get_Item_method;
		MethodInfo* lst_Add_method;
		MethodInfo* lst_get_Count_method;

		lst_get_Item_t lst_get_Item;
		lst_Add_t lst_Add;
		lst_get_Count_t lst_get_Count;
	};

}


namespace
{
	void path_game_assembly();
	void patchNP(HMODULE module);
	bool mh_inited = false;
	void* load_library_w_orig = nullptr;
	bool (*CloseNPGameMon)();

	void reopen_self() {
		int result = MessageBoxW(NULL, L"未检测到目标加载，插件初始化失败。是否重启游戏？", L"提示", MB_OKCANCEL);

		if (result == IDOK)
		{
			STARTUPINFOA startupInfo{ .cb = sizeof(STARTUPINFOA) };
			PROCESS_INFORMATION pi{};
			const auto commandLine = GetCommandLineA();
			if (CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &pi)) {
				// printf("open external plugin: %s (%lu)\n", commandLine, pi.dwProcessId);
				// DWORD dwRetun = 0;
				// WaitForSingleObject(pi.hProcess, INFINITE);
				// GetExitCodeProcess(pi.hProcess, &dwRetun);
				// printf("plugin exit: %d\n", dwRetun);
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
			}
			TerminateProcess(GetCurrentProcess(), 0);
		}
	}

	HMODULE __stdcall load_library_w_hook(const wchar_t* path)
	{
		using namespace std;
		// printf("load library: %ls\n", path);
		if (!exd && (wstring_view(path).find(L"NPGameDLL.dll") != wstring_view::npos)) {
			// printf("fing NP\n");
			// Sleep(10000000000000);
			// printf("end sleep\n");
			auto ret = reinterpret_cast<decltype(LoadLibraryW)*>(load_library_w_orig)(path);
			patchNP(ret);
			if (environment_get_stacktrace) {
				// printf("NP: %ls\n", environment_get_stacktrace()->start_char);
			}
			return ret;
		}
		// else if (path == L"GameAssembly.dll"sv) {
		//	auto ret = reinterpret_cast<decltype(LoadLibraryW)*>(load_library_w_orig)(path);
		//	return ret;
		// }
		else if (path == L"cri_ware_unity.dll"sv) {
			path_game_assembly();
		}

		return reinterpret_cast<decltype(LoadLibraryW)*>(load_library_w_orig)(path);
	}

	void* (*Encoding_get_UTF8)();
	Il2CppString* (*Encoding_GetString)(void* _this, void* bytes);
	bool isInitedEncoding = false;

	void initEncoding() {
		if (isInitedEncoding) return;
		isInitedEncoding = true;
		convertPtrType(&Encoding_get_UTF8, il2cpp_symbols::get_method_pointer("mscorlib.dll", "System.Text", "Encoding", "get_UTF8", 0));
		convertPtrType(&Encoding_GetString, il2cpp_symbols::get_method_pointer("mscorlib.dll", "System.Text", "Encoding", "GetString", 1) - 0x1B0);
		// convertPtrType(&Encoding_GetString, 0x182ABF800);  // F800 = PTR - 0x1B0
		// printf("Encoding_GetString at %p\n", Encoding_GetString);
		/*
		auto Encoding_GetString_addr = il2cpp_symbols::find_method("mscorlib.dll", "System.Text", "Encoding", [](const MethodInfo* mtd) {
			if (mtd->parameters_count == 1) {
				//if (strcmp(mtd->name, "GetString") == 0) {
				//	printf("GetString Type: %x\n", mtd->parameters->parameter_type->type);
				//}
			}
			return false;
			});
			*/

	}

	Il2CppString* bytesToIl2cppString(void* bytes) {
		initEncoding();
		auto encodingInstance = Encoding_get_UTF8();
		// return Encoding_GetString(encodingInstance, bytes);
		return Encoding_GetString(encodingInstance, bytes);
	}

	struct TransparentStringHash : std::hash<std::wstring>, std::hash<std::wstring_view>
	{
		using is_transparent = void;
	};

	typedef std::unordered_set<std::wstring, TransparentStringHash, std::equal_to<void>> AssetPathsType;
	std::unordered_map<std::string, void*> CustomAssetBundleGcHandleMap{};
	void* (*AssetBundle_LoadFromFile)(Il2CppString* path, UINT32 crc, UINT64 offset);

	void* ReplaceFontGcHandle;
	bool (*Object_IsNativeObjectAlive)(void*);
	void* (*AssetBundle_LoadAsset)(void* _this, Il2CppString* name, Il2CppReflectionType* type);
	Il2CppReflectionType* Font_Type;

	void* LoadExternAsset(std::string assetFullPath, Il2CppReflectionType* assetType) {
		const std::string delimiter = "::";
		size_t index = assetFullPath.rfind(delimiter);
		std::string bundlePath, assetPath;
		if (index == std::string::npos) {
			bundlePath = "";
			assetPath = assetFullPath;
		}
		else {
			bundlePath = assetFullPath.substr(0, index);
			assetPath = assetFullPath.substr(index + delimiter.length());
		}

		if (bundlePath.empty() || !std::filesystem::exists(bundlePath)) {
			std::cout << "[ERROR] AssetBundle \"" << bundlePath << "\" doesn't exist." << std::endl;
			return nullptr;
		}

		void* assetBundle;
		auto it = CustomAssetBundleGcHandleMap.find(bundlePath);
		if (it != CustomAssetBundleGcHandleMap.end()) {
			auto gcHandle = it->second;
			assetBundle = il2cpp_gchandle_get_target(gcHandle);
		}
		else
		{
			static auto func_Path_GetFullPath = reinterpret_cast<Il2CppString * (*)(Il2CppString*)>(il2cpp_symbols_logged::get_method_pointer("mscorlib.dll", "System.IO", "Path", "GetFullPath", 1));
			const auto extraAssetBundle = AssetBundle_LoadFromFile(func_Path_GetFullPath(il2cpp_string_new(bundlePath.c_str())), 0, 0);
			if (extraAssetBundle) {
				assetBundle = extraAssetBundle;
				CustomAssetBundleGcHandleMap.emplace(bundlePath, il2cpp_gchandle_new(extraAssetBundle, false));
			}
			else {
				std::cout << "[ERROR] Failed to load AssetBundle \"" << bundlePath << "\"." << std::endl;
				return nullptr;
			}
		}

		auto asset = AssetBundle_LoadAsset(assetBundle, il2cpp_string_new(assetPath.c_str()), assetType);
		if (!asset) {
			std::cout << "[ERROR] Failed to load asset \"" << assetFullPath << "\"." << std::endl;
			return nullptr;
		}
		return asset;
	}

	void* getReplaceFont() {
		void* replaceFont{};
		if (g_custom_font_path.empty()) return replaceFont;

		if (ReplaceFontGcHandle)
		{
			replaceFont = il2cpp_gchandle_get_target(ReplaceFontGcHandle);

			// 加载场景时会被 Resources.UnloadUnusedAssets 干掉，且不受 DontDestroyOnLoad 影响，暂且判断是否存活，并在必要的时候重新加载
			// TODO: 考虑挂载到 GameObject 上
			// AssetBundle 不会被干掉
			if (Object_IsNativeObjectAlive(replaceFont))
			{
				return replaceFont;
			}
			else
			{
				il2cpp_gchandle_free(std::exchange(ReplaceFontGcHandle, nullptr));
			}
		}

		replaceFont = LoadExternAsset(g_custom_font_path, Font_Type);
		if (replaceFont)
		{
			ReplaceFontGcHandle = il2cpp_gchandle_new(replaceFont, false);
		}
		else
		{
			std::wprintf(L"Cannot load asset font\n");
		}

		return replaceFont;
	}

	void* DataFile_GetBytes_orig;
	bool (*DataFile_IsKeyExist)(Il2CppString*);

	void set_fps_hook(int value);
	void set_vsync_count_hook(int value);

	std::filesystem::path dumpBasePath("dumps");

	// 调用之前检查 DataFile_IsKeyExist
	void fmtAndDumpJsonBytesData(const std::wstring& dumpName) {
		const auto dumpNameIl = il2cpp_symbols::NewWStr(dumpName);
		auto dataBytes = reinterpret_cast<void* (*)(Il2CppString*)>(DataFile_GetBytes_orig)(dumpNameIl);
		auto newStr = bytesToIl2cppString(dataBytes);
		auto writeWstr = std::wstring(newStr->start_char);
		const std::wstring searchStr = L"\r";
		const std::wstring replaceStr = L"\\r";
		size_t pos = writeWstr.find(searchStr);
		while (pos != std::wstring::npos) {
			writeWstr.replace(pos, 1, replaceStr);
			pos = writeWstr.find(searchStr, pos + replaceStr.length());
		}
		if (writeWstr.ends_with(L",]")) {  // 代哥的 Json 就是不一样
			writeWstr.erase(writeWstr.length() - 2, 1);
		}
		const auto dumpLocalFilePath = dumpBasePath / SCLocal::getFilePathByName(dumpName, true, dumpBasePath);
		std::ofstream dumpFile(dumpLocalFilePath, std::ofstream::out);
		dumpFile << utility::conversions::to_utf8string(writeWstr).c_str();
		printf("dump %ls success. (%ls)\n", dumpName.c_str(), dumpLocalFilePath.c_str());
	}

	int dumpScenarioDataById(const std::string& sid) {
		int i = 0;
		int notFoundCount = 0;
		int dumpedCount = 0;
		printf("dumping: %s\n", sid.c_str());
		while (true) {
			const auto dumpName = std::format(L"{}_{:02}.json", utility::conversions::to_utf16string(sid), i);
			const auto dumpNameIl = il2cpp_symbols::NewWStr(dumpName);
			if (!DataFile_IsKeyExist(dumpNameIl)) {
				// printf("%ls - continue.\n", dumpName.c_str());
				notFoundCount++;
				if (notFoundCount < 5) {
					i++;
					continue;
				}
				else {
					break;
				}
			}
			fmtAndDumpJsonBytesData(dumpName);
			dumpedCount++;
			i++;
		}
		return dumpedCount;
	}

	bool dumpScenarioDataByJsonFileName(const std::string& fileName) {
		std::string searchName;
		if (!fileName.ends_with(".json")) {
			searchName = fileName + ".json";
		}
		else {
			searchName = fileName;
		}
		auto searchNameStr = il2cpp_string_new(searchName.c_str());
		if (!DataFile_IsKeyExist(searchNameStr)) return false;
		fmtAndDumpJsonBytesData(utility::conversions::to_string_t(searchName));
		return true;
	}

	void dumpByteArray(const std::wstring& tag, const std::wstring& name, void* bytes) {
		static auto WriteAllBytes = reinterpret_cast<void (*)(Il2CppString*, void*)>(
			il2cpp_symbols::get_method_pointer("mscorlib.dll", "System.IO",
				"File", "WriteAllBytes", 2)
			);

		std::filesystem::path dumpPath = dumpBasePath / tag;
		if (!std::filesystem::is_directory(dumpPath)) {
			std::filesystem::create_directories(dumpPath);
		}
		std::filesystem::path dumpName = dumpPath / name;
		WriteAllBytes(il2cpp_symbols::NewWStr(dumpName.c_str()), bytes);

	}

	int dumpScenarioFromCatalog();

	std::string localizationDataCache("{}");
	int dumpAllScenarioData() {
		try {
			return dumpScenarioFromCatalog();
		}
		catch (std::exception& e) {
			printf("dumpScenarioFromCatalog error: %s\n", e.what());
		}
		// 下方方法已过时
		int totalCount = 0;
		const auto titleData = nlohmann::json::parse(localizationDataCache);
		for (auto& it : titleData.items()) {
			const auto& key = it.key();
			const std::string_view keyView(key);
			if (keyView.starts_with("mlADVInfo_Title")) {
				totalCount += dumpScenarioDataById(key.substr(16));
			}
		}
		return totalCount;
	}

	bool guiStarting = false;
	void startSCGUI() {
		if (guiStarting) return;
		guiStarting = true;
		std::thread([]() {
			printf("GUI START\n");
			guimain();
			guiStarting = false;
			printf("GUI END\n");
			}).detach();
	}

	void* (*ReadAllBytes)(Il2CppString*);
	void* readFileAllBytes(const std::wstring& name) {
		return ReadAllBytes(il2cpp_symbols::NewWStr(name));
	}

	void itLocalizationManagerDic(void* _this) {
		if (!SCGUIData::needExtractText) return;
		SCGUIData::needExtractText = false;

		static auto LocalizationManager_klass = il2cpp_symbols::get_class_from_instance(_this);
		static auto dic_field = il2cpp_class_get_field_from_name(LocalizationManager_klass, "dic");
		auto dic = il2cpp_symbols::read_field(_this, dic_field);

		static auto toJsonStr = reinterpret_cast<Il2CppString * (*)(void*)>(
			il2cpp_symbols::get_method_pointer("Newtonsoft.Json.dll", "Newtonsoft.Json",
				"JsonConvert", "SerializeObject", 1)
			);

		auto jsonStr = toJsonStr(dic);
		printf("dump text...\n");
		std::wstring jsonWStr(jsonStr->start_char);

		if (!std::filesystem::is_directory(dumpBasePath)) {
			std::filesystem::create_directories(dumpBasePath);
		}
		std::ofstream file(dumpBasePath / "localify.json", std::ofstream::out);
		const auto writeStr = utility::conversions::to_utf8string(jsonWStr);
		localizationDataCache = writeStr;
		file << writeStr.c_str();
		file.close();
		printf("write done.\n");

		const auto dumpCount = dumpAllScenarioData();
		printf("Dump finished. Total %d files\n", dumpCount);

		return;
	}

	void updateDicText(void* _this, Il2CppString* category, int id, std::string& newStr) {
		static auto LocalizationManager_klass = il2cpp_symbols::get_class_from_instance(_this);
		static auto dic_field = il2cpp_class_get_field_from_name(LocalizationManager_klass, "dic");
		auto dic = il2cpp_symbols::read_field(_this, dic_field);

		static auto dic_klass = il2cpp_symbols::get_class_from_instance(dic);
		static auto dict_get_Item = reinterpret_cast<void* (*)(void*, void*)>(
			il2cpp_class_get_method_from_name(dic_klass, "get_Item", 1)->methodPointer
			);

		auto categoryDict = dict_get_Item(dic, category);
		static auto cdic_klass = il2cpp_symbols::get_class_from_instance(categoryDict);
		static auto dict_set_Item = reinterpret_cast<void* (*)(void*, int, void*)>(
			il2cpp_class_get_method_from_name(cdic_klass, "set_Item", 2)->methodPointer
			);
		dict_set_Item(categoryDict, id, il2cpp_string_new(newStr.c_str()));

	}

	// NOT HOOK
	void* PIdolDetailPopupViewModel_Create_orig;
	void* PIdolDetailPopupViewModel_Create_hook(void* produceIdol, void* costumeSetInfoList, void* idolBase, void* idolParameter, bool isChangeableIdolSkill, bool isChangeableFavorite, void* produceAdvStatusList, bool isPlayableAdv, bool inLive, bool upgradingButtonActive) {
		auto ret = reinterpret_cast<decltype(PIdolDetailPopupViewModel_Create_hook)*>(PIdolDetailPopupViewModel_Create_orig)(
			produceIdol, costumeSetInfoList, idolBase, idolParameter, isChangeableIdolSkill, isChangeableFavorite, produceAdvStatusList, isPlayableAdv, inLive, upgradingButtonActive
			);
		return ret;
		/*  // 功能被 ScenarioContentViewModel_ctor_hook 替代
		static auto get_EventList = reinterpret_cast<void* (*)(void*)>(
			il2cpp_symbols::get_method_pointer("PRISM.Adapters.dll", "PRISM.Adapters",
				"PIdolDetailPopupViewModel", "get_EventList", 0)
			);
		static auto EventModel_klass = il2cpp_symbols::get_class("PRISM.Adapters.dll", "PRISM.Adapters", "EventModel");
		static auto Read_field = il2cpp_class_get_field_from_name(EventModel_klass, "<Read>k__BackingField");
		static auto IsAdvPlayable_field = il2cpp_class_get_field_from_name(EventModel_klass, "<IsAdvPlayable>k__BackingField");

		printf("PIdolDetailPopupViewModel_Create\n");

		auto events = get_EventList(ret);

		il2cpp_symbols::iterate_IEnumerable(events, [](void* event) {
			const auto read = il2cpp_symbols::read_field<bool>(event, Read_field);
			const auto isAdvPlayable = il2cpp_symbols::read_field<bool>(event, IsAdvPlayable_field);
			printf("read: %d, isAdvPlayable: %d\n", read, isAdvPlayable);

			il2cpp_symbols::write_field(event, Read_field, true);
			});

		return ret;*/
	}

	void* ScenarioContentViewModel_ctor_orig;
	void ScenarioContentViewModel_ctor_hook(void* _this, void* scenarioID, Il2CppString* title, Il2CppString* summary, bool isLocked,
		bool isAdvPlayable, Il2CppString* alias, Il2CppString* characterName, int unlockLevel) {

		if (g_unlock_PIdol_and_SChara_events) {
			if (!isLocked) {
				wprintf(L"Force Unlock Event: %ls\n", title->start_char);
				isLocked = true;
			}
		}
		return reinterpret_cast<decltype(ScenarioContentViewModel_ctor_hook)*>(ScenarioContentViewModel_ctor_orig)(_this, scenarioID, title, summary, isLocked, isAdvPlayable, alias, characterName, unlockLevel);
	}

	void* LocalizationManager_GetTextOrNull_orig;
	Il2CppString* LocalizationManager_GetTextOrNull_hook(void* _this, Il2CppString* category, int id) {
		if (g_max_fps != -1) set_fps_hook(g_max_fps);
		if (g_vsync_count != -1) set_vsync_count_hook(g_vsync_count);
		itLocalizationManagerDic(_this);
		std::string resultText = "";
		if (SCLocal::getLocalifyText(utility::conversions::to_utf8string(category->start_char), id, &resultText)) {
			//updateDicText(_this, category, id, resultText);
			return il2cpp_string_new(resultText.c_str());
		}
		// GG category: mlPublicText_GameGuard, id: 1
		return reinterpret_cast<decltype(LocalizationManager_GetTextOrNull_hook)*>(LocalizationManager_GetTextOrNull_orig)(_this, category, id);
	}

	void* GetResolutionSize_orig;
	Vector2Int_t GetResolutionSize_hook(void* camera, void* method) {
		auto ret = reinterpret_cast<decltype(GetResolutionSize_hook)*>(GetResolutionSize_orig)(camera, method);
		if (g_3d_resolution_scale != 1.0f) {
			ret.x *= g_3d_resolution_scale;
			ret.y *= g_3d_resolution_scale;
			SCCamera::currRenderResolution.x = ret.x;
			SCCamera::currRenderResolution.y = ret.y;
		}
		return ret;
	}

	void* AssetBundle_LoadAsset_orig;
	void* AssetBundle_LoadAsset_hook(void* _this, Il2CppString* name, Il2CppReflectionType* type)
	{
		// printf("AssetBundle_LoadAsset: %ls\n", name->start_char);
		return reinterpret_cast<decltype(AssetBundle_LoadAsset_hook)*>(AssetBundle_LoadAsset_orig)(_this, name, type);
	}

	class SpanReader {
	private:
		const uint8_t* dataPtr;
		size_t dataSize;
		size_t offset;

	public:
		SpanReader(const uint8_t* data, size_t size) : dataPtr(data), dataSize(size), offset(0) {}

		template<typename T>
		T Read() {
			T value;
			std::memcpy(&value, dataPtr + offset, sizeof(T));
			offset += sizeof(T);

			// if !littleEndian
			std::reverse(reinterpret_cast<uint8_t*>(&value), reinterpret_cast<uint8_t*>(&value) + sizeof(T));

			return value;
		}

		uint64_t ReadUInt64() {
			return Read<uint64_t>();
		}

		uint64_t ReadVarUInt64() {
			uint64_t value = 0;
			int shift = 0;

			do {
				uint8_t b = dataPtr[offset++];
				value |= static_cast<uint64_t>(b & 0x7FU) << shift;
				if (b < 0x80)
					break;
				shift += 7;
			} while (64 > shift);

			return value;
		}
	};

	void* FromCatalogInfo(const std::wstring& encodedInfo, const UINT64 rootHash) {
		static auto CatalogManifest_FromValues = reinterpret_cast<void* (*)(UINT64 labelCrc, UINT64 size, UINT64 checksum, UINT64 seed)>(
			il2cpp_symbols::get_method_pointer("Limelight.Shared.dll", "Limelight",
				"CatalogManifest", "FromValues", 4)
			);

		unsigned char info[5120];
		const auto cvString = utility::conversions::to_utf8string(encodedInfo);
		boost::beast::detail::base64::decode(&info, cvString.c_str(), cvString.size());

		SpanReader reader(info, sizeof(info));

		const auto checksum = reader.ReadUInt64();
		const auto size = reader.ReadVarUInt64();
		const auto seed = reader.ReadUInt64();

		// wprintf(L"encodedInfo: %ls, rootHash: %llu, size: %llu, checksum: %llu, seed: %llu\n", encodedInfo.c_str(), rootHash, size, checksum, seed);
		return CatalogManifest_FromValues(rootHash, size, checksum, seed);
	}

	void* entryAsManifest(void* entry) {
		static auto CatalogManifest_FromValues = reinterpret_cast<void* (*)(UINT64 labelCrc, UINT64 size, UINT64 checksum, UINT64 seed)>(
			il2cpp_symbols::get_method_pointer("Limelight.Shared.dll", "Limelight",
				"CatalogManifest", "FromValues", 4)
			);

		static auto CatalogBinaryEntry_klass = il2cpp_symbols::get_class("Limelight.Shared.dll", "Limelight", "CatalogBinaryEntry");
		static auto LabelCrc_field = il2cpp_class_get_field_from_name(CatalogBinaryEntry_klass, "<LabelCrc>k__BackingField");
		static auto Size_field = il2cpp_class_get_field_from_name(CatalogBinaryEntry_klass, "<Size>k__BackingField");
		static auto Checksum_field = il2cpp_class_get_field_from_name(CatalogBinaryEntry_klass, "<Checksum>k__BackingField");
		static auto Seed_field = il2cpp_class_get_field_from_name(CatalogBinaryEntry_klass, "<Seed>k__BackingField");

		const auto LabelCrc = il2cpp_symbols::read_field<UINT64>(entry, LabelCrc_field);
		const auto Size = il2cpp_symbols::read_field<UINT64>(entry, Size_field);
		const auto Checksum = il2cpp_symbols::read_field<UINT64>(entry, Checksum_field);
		const auto Seed = il2cpp_symbols::read_field<UINT64>(entry, Seed_field);

		return CatalogManifest_FromValues(LabelCrc, Size, Checksum, Seed);
	}


	void* checkAndDownloadFile(Il2CppString* fileNameStr) {
		try {
			const std::wstring fileName(fileNameStr->start_char);

			std::filesystem::path userProfile = getenv("UserProfile");
			std::filesystem::path filePath = userProfile / "AppData/LocalLow/BNE/imasscprism/D" / fileName.substr(0, 2) / fileName;
			if (std::filesystem::exists(filePath)) {
				return readFileAllBytes(filePath);
			}

			const auto url = std::format(L"https://asset.imassc.song4.prism.bn765.com/r/{}/{}", fileName.substr(0, 2), fileName);
			web::http::client::http_client_config cfg;
			cfg.set_timeout(utility::seconds(30));
			web::http::client::http_client client(url, cfg);
			auto response = client.request(web::http::methods::GET).get();
			if (response.status_code() != 200) {
				wprintf(L"File download failed: %ls (%d)\n", url.c_str(), response.status_code());
				return NULL;
			}

			concurrency::streams::fstream::open_ostream(filePath.c_str()).then([=](concurrency::streams::ostream output_stream) {
				return response.body().read_to_end(output_stream.streambuf());
				}).then([=](size_t) {
					wprintf(L"File downloaded successfully: %ls\n", filePath.c_str());
					}).wait();

				return readFileAllBytes(filePath);
		}
		catch (std::exception& e) {
			printf("checkAndDownloadFile error: %s\n", e.what());
			return NULL;
		}

	}

	struct GameVersion {
		std::wstring gameVersion;
		std::wstring resourceVersion;
	};

	GameVersion getGameVersions() {
		static auto getGameVersion = reinterpret_cast<Il2CppString * (*)()>(
			il2cpp_symbols::get_method_pointer(
				"UnityEngine.CoreModule.dll", "UnityEngine",
				"Application", "get_version", 0
			)
			);

		static auto Global_get_instance = reinterpret_cast<void* (*)()>(
			il2cpp_symbols::get_method_pointer("PRISM.ResourceManagement.dll", "PRISM",
				"Global", "get_Instance", 0)
			);

		static auto get_CurrentResourceVersion = reinterpret_cast<Il2CppString * (*)(void*)>(
			il2cpp_symbols::get_method_pointer("PRISM.ResourceManagement.dll", "PRISM",
				"Global", "get_CurrentResourceVersion", 0)
			);

		const std::wstring gameVer(getGameVersion()->start_char);
		const std::wstring resVersion(get_CurrentResourceVersion(Global_get_instance())->start_char);

		return GameVersion{ .gameVersion = gameVer , .resourceVersion = resVersion };
	}

	std::pair<void*, void*> getManifestFile(Il2CppString* defaultFileName) {
		try {
			auto defaultData = checkAndDownloadFile(defaultFileName);
			if (defaultData != NULL) {
				return std::make_pair(nullptr, defaultData);
			}
			printf("Try get fallback version...\n");

			static auto CatalogManifest_FromUniqueVersion = reinterpret_cast<void* (*)(Il2CppString * uniqueVersion, Il2CppString * bundleVersion)>(
				il2cpp_symbols::get_method_pointer("Limelight.Shared.dll", "Limelight",
					"CatalogManifest", "FromUniqueVersion", 2)
				);
			static auto CatalogManifest_GetRealName = reinterpret_cast<Il2CppString * (*)(void*)>(
				il2cpp_symbols::get_method_pointer("Limelight.Shared.dll", "Limelight",
					"CatalogManifest", "GetRealName", 0)
				);

			const auto gameVerInfo = getGameVersions();
			const std::wstring gameVer = gameVerInfo.gameVersion;
			const std::wstring resVersion = gameVerInfo.resourceVersion;

			const auto subPointPos = gameVer.rfind(L'.', -1);
			if (subPointPos == std::wstring::npos) {
				wprintf(L"Invalid game version: %ls\n", gameVer.c_str());
				return std::make_pair(nullptr, nullptr);
			}
			const auto baseVersionString = gameVer.substr(0, subPointPos + 1);
			auto subVersion = std::stoi(gameVer.substr(subPointPos + 1, gameVer.length()));

			while (subVersion >= 0) {
				const auto currentVersion = std::format(L"{}{}", baseVersionString, subVersion);
				wprintf(L"Checking version: %ls\n", currentVersion.c_str());
				auto manifest = CatalogManifest_FromUniqueVersion(il2cpp_symbols::NewWStr(resVersion), il2cpp_symbols::NewWStr(currentVersion));
				if (manifest) {
					auto data = checkAndDownloadFile(CatalogManifest_GetRealName(manifest));
					if (data != NULL) {
						return std::make_pair(manifest, data);
					}
				}
				else {
					printf("manifest is null.\n");
				}
				subVersion--;
			}
		}
		catch (std::exception& ex) {
			printf("getManifestFile error: %s\n", ex.what());
		}
		printf("Get manifest file failed.\n");
		return std::make_pair(nullptr, nullptr);
	}


	int dumpScenarioFromCatalog() {
		static auto string_op_Implicit = reinterpret_cast<void* (*)(void* retstr, Il2CppString*)>(
			il2cpp_symbols::get_method_pointer("mscorlib.dll", "System",
				"String", "op_Implicit", 1)
			);

		static auto ReadOnlySpan_char_klass = il2cpp_symbols::get_system_class_from_reflection_type_str(
			"System.ReadOnlySpan`1[System.Char]");

		static auto HashUtils_Crc64 = reinterpret_cast<UINT64(*)(void*, UINT64)>(
			il2cpp_symbols::get_method_pointer("Limelight.Shared.dll", "Limelight",
				"HashUtils", "Crc64", 2)
			);

		static auto CatalogManifest_GetRealName = reinterpret_cast<Il2CppString * (*)(void*)>(
			il2cpp_symbols::get_method_pointer("Limelight.Shared.dll", "Limelight",
				"CatalogManifest", "GetRealName", 0)
			);

		static auto CatalogBinaryParser_ParseEncoded = reinterpret_cast<void* (*)(void* manifest, void* input)>(
			il2cpp_symbols::get_method_pointer("Limelight.Shared.dll", "Limelight",
				"CatalogBinaryParser", "ParseEncoded", 2)
			);

		static auto CatalogBinary_get_Entries = reinterpret_cast<void* (*)(void*)>(
			il2cpp_symbols::get_method_pointer("Limelight.Shared.dll", "Limelight",
				"CatalogBinary", "get_Entries", 0)
			);

		static auto CatalogBinaryEntry_ToCatalogManifest = reinterpret_cast<void* (*)(void*)>(
			il2cpp_symbols::get_method_pointer("Limelight.Shared.dll", "Limelight",
				"CatalogBinaryEntry", "ToCatalogManifest", 0)
			);

		static auto CatalogBinaryEntry_get_Label = reinterpret_cast<void* (*)(void* retstr, void* _this)>(
			il2cpp_symbols::get_method_pointer("Limelight.Shared.dll", "Limelight",
				"CatalogBinaryEntry", "get_Label", 0)
			);

		static auto CatalogManifest_FromUniqueVersion = reinterpret_cast<void* (*)(Il2CppString * uniqueVersion, Il2CppString * bundleVersion)>(
			il2cpp_symbols::get_method_pointer("Limelight.Shared.dll", "Limelight",
				"CatalogManifest", "FromUniqueVersion", 2)
			);

		static auto CatalogBinaryEntry_klass = il2cpp_symbols::get_class("Limelight.Shared.dll", "Limelight", "CatalogBinaryEntry");
		static auto Label_field = il2cpp_class_get_field_from_name(CatalogBinaryEntry_klass, "<Label>k__BackingField");

		static auto Encoding_get_UTF8 = reinterpret_cast<void* (*)()>(
			il2cpp_symbols::get_method_pointer("mscorlib.dll", "System.Text",
				"Encoding", "get_UTF8", 0)
			);

		static auto Encoding_GetString = reinterpret_cast<Il2CppString * (*)(void*, void*, int, int)>(
			il2cpp_symbols::get_method_pointer("mscorlib.dll", "System.Text",
				"Encoding", "GetString", 3)
			);

		const auto gameVerInfo = getGameVersions();
		const std::wstring gameVer = gameVerInfo.gameVersion;
		const std::wstring resVersion = gameVerInfo.resourceVersion;

		const auto atPos = resVersion.find(L'@');
		const auto simpleVersion = resVersion.substr(0, atPos);
		const auto encodedInfo = resVersion.substr(atPos + 1);

		auto ReadOnlySpan_char = il2cpp_object_new(ReadOnlySpan_char_klass);

		const auto cmpStr = il2cpp_symbols::NewWStr(std::format(L"{}:{}", gameVer, simpleVersion));
		const auto rootHash = HashUtils_Crc64(string_op_Implicit(ReadOnlySpan_char, cmpStr), 0);

		auto manifest = FromCatalogInfo(encodedInfo, rootHash);
		// CatalogManifest_FromUniqueVersion(il2cpp_string_new("10003400@9t5HYphlvDRn0EijwOLgfp0="), il2cpp_string_new("1.7.0"));
		// auto manifest = CatalogManifest_FromUniqueVersion(il2cpp_symbols::NewWStr(resVersion), il2cpp_symbols::NewWStr(gameVer));

		wprintf(L"manifest at %p, rootHash: %llu (%ls), resVersion: %ls\n", manifest, rootHash, cmpStr->start_char, resVersion.c_str());

		auto [newManifest, encodedCatalog] = getManifestFile(CatalogManifest_GetRealName(manifest));
		if (encodedCatalog == nullptr) {
			printf("readCatalog failed: encodedCatalog is NULL.\n");
			return 0;
		}
		if (newManifest != nullptr) {
			manifest = newManifest;
		}

		auto catalog = CatalogBinaryParser_ParseEncoded(manifest, encodedCatalog);  // Limelight.CatalogBinary
		int dumpCount = 0;

		il2cpp_symbols::iterate_IEnumerable(CatalogBinary_get_Entries(catalog), [&dumpCount](void* entry) {
			auto manifest2 = entryAsManifest(entry);
			auto encodedCatalog2 = checkAndDownloadFile(CatalogManifest_GetRealName(manifest2));
			if (encodedCatalog2 == NULL) {
				printf("encodedCatalog2 failed: encodedCatalog is NULL.\n");
				return;
			}
			auto catalog2 = CatalogBinaryParser_ParseEncoded(manifest2, encodedCatalog2);  // Limelight.CatalogBinary

			il2cpp_symbols::iterate_IEnumerable(CatalogBinary_get_Entries(catalog2), [&dumpCount](void* info) {
				static auto toJsonStr = reinterpret_cast<Il2CppString * (*)(void*)>(
					il2cpp_symbols::get_method_pointer("Newtonsoft.Json.dll", "Newtonsoft.Json",
						"JsonConvert", "SerializeObject", 1)
					);

				/*
				wprintf(L"label:\n%ls\n\n", toJsonStr(info)->start_char);
				static auto ArraySegment_Byte_klass = il2cpp_symbols::get_system_class_from_reflection_type_str(
					"System.ArraySegment`1[System.Byte]");
				auto label = il2cpp_object_new(ArraySegment_Byte_klass);  // System.ArraySegment`1<unsigned int8>

				label = CatalogBinaryEntry_get_Label(label, info);  // System.ArraySegment`1<unsigned int8>
				*/

				auto label = il2cpp_field_get_value_object(Label_field, info);

				const std::wstring labelArrayStr(toJsonStr(label)->start_char);
				auto labelArray = nlohmann::json::parse(labelArrayStr);
				std::string labelStr;
				for (auto& i : labelArray) {
					int l = i;
					labelStr += (char)l;
				}

				static std::regex pattern("^s\\d+_\\d+_\\d+$");

				if (std::regex_match(labelStr, pattern)) {
					// printf("labelStr: %s\n", labelStr.c_str());
					if (dumpScenarioDataByJsonFileName(labelStr)) {
						dumpCount++;
					}
				}

				/*
				static auto ArraySegment_Byte_klass = il2cpp_symbols::get_class_from_instance(label);

				static auto ArraySegment_get_Array = reinterpret_cast<void* (*)(void*)>(
					il2cpp_class_get_method_from_name(ArraySegment_Byte_klass, "get_Array", 0)->methodPointer
					);
				static auto ArraySegment_get_Offset = reinterpret_cast<int (*)(void*)>(
					il2cpp_class_get_method_from_name(ArraySegment_Byte_klass, "get_Offset", 0)->methodPointer
					);
				static auto ArraySegment_get_Count = reinterpret_cast<int (*)(void*)>(
					il2cpp_class_get_method_from_name(ArraySegment_Byte_klass, "get_Count", 0)->methodPointer
					);

				auto labelStrArray = ArraySegment_get_Array(label);
				auto labelStrOffset = ArraySegment_get_Offset(label);
				auto labelStrCount = ArraySegment_get_Count(label);


				wprintf(L"labelStrOffset: %d, labelStrCount: %d\n", labelStrOffset, labelStrCount);
				auto labelStr = Encoding_GetString(Encoding_get_UTF8(), labelStrArray, labelStrOffset, labelStrCount);

				wprintf(L"labelStr: %ls\n", labelStr->start_char);*/
				});

			});
		return dumpCount;
	}

	void* LiveMVOverlayView_UpdateLyrics_orig;
	void LiveMVOverlayView_UpdateLyrics_hook(void* _this, Il2CppString* text) {
		const std::wstring origWstr(text->start_char);
		const auto newText = SCLocal::getLyricsTrans(origWstr);
		return reinterpret_cast<decltype(LiveMVOverlayView_UpdateLyrics_hook)*>(LiveMVOverlayView_UpdateLyrics_orig)(_this, il2cpp_string_new(newText.c_str()));
	}

	std::pair<std::wstring, std::string> lastLrc{};
	void* TimelineController_SetLyric_orig;
	void TimelineController_SetLyric_hook(void* _this, Il2CppString* text) {
		const std::wstring origWstr(text->start_char);
		std::string newText = "";
		if (!origWstr.empty()) {
			if (lastLrc.first.compare(origWstr) != 0) {
				newText = SCLocal::getLyricsTrans(origWstr);
				lastLrc.first = origWstr;
				lastLrc.second = newText;
			}
			else {
				newText = lastLrc.second;
			}
		}
		return reinterpret_cast<decltype(TimelineController_SetLyric_hook)*>(TimelineController_SetLyric_orig)(_this, il2cpp_string_new(newText.c_str()));
	}

	void* TMP_Text_set_text_orig;
	void TMP_Text_set_text_hook(void* _this, Il2CppString* value) {
		if (needPrintStack) {
			//wprintf(L"TMP_Text_set_text: %ls\n", value->start_char);
			//printf("%ls\n\n", environment_get_stacktrace()->start_char);
		}
		//if(value) value = il2cpp_symbols::NewWStr(std::format(L"(h){}", std::wstring(value->start_char)));
		reinterpret_cast<decltype(TMP_Text_set_text_hook)*>(TMP_Text_set_text_orig)(
			_this, value
			);
	}

	bool get_NeedsLocalization_func(void* instanceUITextMeshProUGUI) {
		return false;
		/*
		// static auto UITextMeshProUGUI_klass = il2cpp_symbols::get_class("PRISM.Legacy.dll", "ENTERPRISE.UI", "UITextMeshProUGUI");
		auto this_klass = il2cpp_symbols::get_class_from_instance(instanceUITextMeshProUGUI);
		auto m_EnvMapMatrix_field = il2cpp_class_get_field_from_name(this_klass, "m_EnvMapMatrix");
		auto Matrix4x4_value_field = il2cpp_symbols::get_field("UnityEngine.CoreModule.dll", "UnityEngine", "Matrix4x4", "m32");

		auto m_EnvMapMatrix = il2cpp_symbols::read_field(instanceUITextMeshProUGUI, m_EnvMapMatrix_field);

		printf("%p, m_EnvMapMatrix_field: %p, m_EnvMapMatrix at %p\n", instanceUITextMeshProUGUI, m_EnvMapMatrix_field, m_EnvMapMatrix);

		if (!m_EnvMapMatrix) return false;

		auto m_EnvMapMatrix_klass = reinterpret_cast<Il2CppClassHead*>(il2cpp_symbols::get_class_from_instance(m_EnvMapMatrix));
		printf("m_EnvMapMatrix class: %s::%s\n", m_EnvMapMatrix_klass->namespaze, m_EnvMapMatrix_klass->name);

		auto Matrix4x4_value = il2cpp_symbols::read_field(m_EnvMapMatrix, Matrix4x4_value_field);
		if (!Matrix4x4_value) return false;

		// printf("m_EnvMapMatrix(%p) - m32: %p\n", m_EnvMapMatrix, Matrix4x4_value);

		return true;*/
	}

	void* (*TMP_FontAsset_CreateFontAsset)(void* font);
	void (*TMP_Text_set_font)(void* _this, void* fontAsset);
	void* (*TMP_Text_get_font)(void* _this);
	// void* lastUpdateFontPtr = nullptr;
	std::unordered_set<void*> updatedFontPtrs{};

	void* UITextMeshProUGUI_Awake_orig;
	void UITextMeshProUGUI_Awake_hook(void* _this) {
		static auto get_Text = reinterpret_cast<Il2CppString * (*)(void*)>(
			il2cpp_symbols::get_method_pointer(
				"PRISM.Legacy.dll", "ENTERPRISE.UI",
				"UITextMeshProUGUI", "get_text", 0
			)
			);
		static auto set_Text = reinterpret_cast<void (*)(void*, Il2CppString*)>(
			il2cpp_symbols::get_method_pointer(
				"PRISM.Legacy.dll", "ENTERPRISE.UI",
				"UITextMeshProUGUI", "set_text", 1
			)
			);

		auto origText = get_Text(_this);
		if (origText) {
			// wprintf(L"UITextMeshProUGUI_Awake: %ls\n", origText->start_char);
			if (!get_NeedsLocalization_func(_this)) {
				std::string newTrans("");
				if (SCLocal::getGameUnlocalTrans(std::wstring(origText->start_char), &newTrans)) {
					set_Text(_this, il2cpp_string_new(newTrans.c_str()));
				}
			}
			else {
				// set_Text(_this, il2cpp_symbols::NewWStr(std::format(L"(接口l){}", std::wstring(origText->start_char))));
			}
		}

		static auto set_sourceFontFile = reinterpret_cast<void (*)(void*, void* font)>(
			il2cpp_symbols::get_method_pointer("Unity.TextMeshPro.dll", "TMPro",
				"TMP_FontAsset", "set_sourceFontFile", 1)
			);
		static auto UpdateFontAssetData = reinterpret_cast<void (*)(void*)>(
			il2cpp_symbols::get_method_pointer("Unity.TextMeshPro.dll", "TMPro",
				"TMP_FontAsset", "UpdateFontAssetData", 0)
			);
		static auto set_fontSize = reinterpret_cast<void (*)(void*, float)>(
			il2cpp_symbols::get_method_pointer("Unity.TextMeshPro.dll", "TMPro",
				"TMP_Text", "set_fontSize", 1)
			);
		static auto get_fontSize = reinterpret_cast<float (*)(void*)>(
			il2cpp_symbols::get_method_pointer("Unity.TextMeshPro.dll", "TMPro",
				"TMP_Text", "get_fontSize", 0)
			);

		auto replaceFont = getReplaceFont();
		if (replaceFont) {
			auto origFont = TMP_Text_get_font(_this);
			set_sourceFontFile(origFont, replaceFont);
			if (!updatedFontPtrs.contains(origFont)) {
				updatedFontPtrs.emplace(origFont);
				UpdateFontAssetData(origFont);
			}
			/*
			if (origFont != lastUpdateFontPtr) {
				UpdateFontAssetData(origFont);
				lastUpdateFontPtr = origFont;
			}*/
		}
		set_fontSize(_this, get_fontSize(_this) + g_font_size_offset);
		reinterpret_cast<decltype(UITextMeshProUGUI_Awake_hook)*>(UITextMeshProUGUI_Awake_orig)(_this);
	}

	void* ScenarioManager_Init_orig;
	void* ScenarioManager_Init_hook(void* retstr, void* _this, Il2CppString* scrName) {
		// printf("ScenarioManager_Init: %ls\n%ls\n\n", scrName->start_char, environment_get_stacktrace()->start_char);
		return reinterpret_cast<decltype(ScenarioManager_Init_hook)*>(ScenarioManager_Init_orig)(retstr, _this, scrName);
	}

	void* DataFile_GetBytes_hook(Il2CppString* path) {
		std::wstring pathStr(path->start_char);
		std::wstring_view pathStrView(pathStr);
		//wprintf(L"DataFile_GetBytes: %ls\n", pathStr.c_str());


		std::filesystem::path localFileName;
		if (SCLocal::getLocalFileName(pathStr, &localFileName)) {
			return readFileAllBytes(localFileName);
		}

		auto ret = reinterpret_cast<decltype(DataFile_GetBytes_hook)*>(DataFile_GetBytes_orig)(path);
		if (g_auto_dump_all_json) {
			if (pathStrView.ends_with(L".json")) {
				auto basePath = std::filesystem::path("dumps") / "autoDumpJson";
				std::filesystem::path fileName = basePath / SCLocal::getFilePathByName(pathStr, true, basePath);

				if (!std::filesystem::is_directory(basePath)) {
					std::filesystem::create_directories(basePath);
				}
				auto newStr = bytesToIl2cppString(ret);
				std::ofstream fileDump(fileName, std::ofstream::out);
				auto writeWstr = std::wstring(newStr->start_char);

				const std::wstring searchStr = L"\r";
				const std::wstring replaceStr = L"\\r";
				size_t pos = writeWstr.find(searchStr);
				while (pos != std::wstring::npos) {
					writeWstr.replace(pos, 1, replaceStr);
					pos = writeWstr.find(searchStr, pos + replaceStr.length());
				}

				fileDump << utility::conversions::to_utf8string(writeWstr).c_str();
				fileDump.close();
				printf("Auto dump: %ls\n", fileName.c_str());
			}
		}
		return ret;
	}

	bool isFirstTimeSetResolution = true;
	void SetResolution_hook(int width, int height, bool fullscreen) {
		// printf("SetResolution: %d, %d, %d, isFirstTimeSetResolution: %d\n", width, height, fullscreen, isFirstTimeSetResolution);
		if (isFirstTimeSetResolution) {
			isFirstTimeSetResolution = false;
			if ((g_start_resolution_w < 0) || (g_start_resolution_h < 0)) {
				g_start_resolution_w = width;
				g_start_resolution_h = height;
				g_start_resolution_fullScreen = fullscreen;
			}
			return reinterpret_cast<decltype(SetResolution_hook)*>(SetResolution_orig)(g_start_resolution_w,
				g_start_resolution_h, g_start_resolution_fullScreen);
		}
		return;
		/*
		width = SCGUIData::screenW;
		height = SCGUIData::screenH;
		fullscreen = SCGUIData::screenFull;
		return reinterpret_cast<decltype(SetResolution_hook)*>(SetResolution_orig)(width, height, fullscreen);
		*/
	}

	void* InvokeMoveNext_orig;
	void InvokeMoveNext_hook(void* enumerator, void* returnValueAddress) {
		MHotkey::start_hotkey(hotKey);
		return reinterpret_cast<decltype(InvokeMoveNext_hook)*>(InvokeMoveNext_orig)(enumerator, returnValueAddress);
	}

	// Normal 3D Live
	void* DepthOfFieldClip_CreatePlayable_orig;
	void* DepthOfFieldClip_CreatePlayable_hook(void* retstr, void* _this, void* graph, void* go, void* mtd) {
		if (g_enable_free_camera) {
			static auto DepthOfFieldClip_klass = il2cpp_symbols::get_class("PRISM.Legacy.dll", "PRISM", "DepthOfFieldClip");
			static auto DepthOfFieldClip_behaviour_field = il2cpp_class_get_field_from_name(DepthOfFieldClip_klass, "behaviour");

			static auto DepthOfFieldBehaviour_klass = il2cpp_symbols::get_class("PRISM.Legacy.dll", "PRISM", "DepthOfFieldBehaviour");
			static auto DepthOfFieldBehaviour_focusDistance_field = il2cpp_class_get_field_from_name(DepthOfFieldBehaviour_klass, "focusDistance");
			static auto DepthOfFieldBehaviour_aperture_field = il2cpp_class_get_field_from_name(DepthOfFieldBehaviour_klass, "aperture");
			static auto DepthOfFieldBehaviour_focalLength_field = il2cpp_class_get_field_from_name(DepthOfFieldBehaviour_klass, "focalLength");
			auto depthOfFieldBehaviour = il2cpp_symbols::read_field(_this, DepthOfFieldClip_behaviour_field);
			/*
			auto focusDistance = il2cpp_symbols::read_field<float>(depthOfFieldBehaviour, DepthOfFieldBehaviour_focusDistance_field);
			auto aperture = il2cpp_symbols::read_field<float>(depthOfFieldBehaviour, DepthOfFieldBehaviour_aperture_field);
			auto focalLength = il2cpp_symbols::read_field<float>(depthOfFieldBehaviour, DepthOfFieldBehaviour_focalLength_field);
			*/

			// invalid values from game version v2.6.1 and crash at the original call; bypass temporarily
			// il2cpp_symbols::write_field(depthOfFieldBehaviour, DepthOfFieldBehaviour_focusDistance_field, 1000.0f);
			// il2cpp_symbols::write_field(depthOfFieldBehaviour, DepthOfFieldBehaviour_aperture_field, 32.0f);
			// il2cpp_symbols::write_field(depthOfFieldBehaviour, DepthOfFieldBehaviour_focalLength_field, 1.0f);

			// printf("DepthOfFieldClip_CreatePlayable, focusDistance: %f, aperture: %f, focalLength: %f\n", focusDistance, aperture, focalLength);
		}
		return reinterpret_cast<decltype(DepthOfFieldClip_CreatePlayable_hook)*>(DepthOfFieldClip_CreatePlayable_orig)(retstr, _this, graph, go, mtd);
	}

	// HDR Live
	void* PostProcess_DepthOfFieldClip_CreatePlayable_orig;
	void PostProcess_DepthOfFieldClip_CreatePlayable_hook(void* retstr, void* _this, void* graph, void* go, void* mtd) {

		if (g_enable_free_camera) {
			static auto DepthOfFieldClip_klass = il2cpp_symbols::get_class("PRISM.Legacy.dll", "UnityEngine.Rendering.Universal.PostProcess", "DepthOfFieldClip");
			static auto DepthOfFieldClip_behaviour_field = il2cpp_class_get_field_from_name(DepthOfFieldClip_klass, "behaviour");

			static auto DepthOfFieldBehaviour_klass = il2cpp_symbols::get_class("PRISM.Legacy.dll", "UnityEngine.Rendering.Universal.PostProcess", "DepthOfFieldBehaviour");
			static auto DepthOfFieldBehaviour_focusDistance_field = il2cpp_class_get_field_from_name(DepthOfFieldBehaviour_klass, "focusDistance");
			static auto DepthOfFieldBehaviour_aperture_field = il2cpp_class_get_field_from_name(DepthOfFieldBehaviour_klass, "aperture");
			static auto DepthOfFieldBehaviour_focalLength_field = il2cpp_class_get_field_from_name(DepthOfFieldBehaviour_klass, "focalLength");
			auto depthOfFieldBehaviour = il2cpp_symbols::read_field(_this, DepthOfFieldClip_behaviour_field);
			/*
			auto focusDistance = il2cpp_symbols::read_field<float>(depthOfFieldBehaviour, DepthOfFieldBehaviour_focusDistance_field);
			auto aperture = il2cpp_symbols::read_field<float>(depthOfFieldBehaviour, DepthOfFieldBehaviour_aperture_field);
			auto focalLength = il2cpp_symbols::read_field<float>(depthOfFieldBehaviour, DepthOfFieldBehaviour_focalLength_field);
			*/

			il2cpp_symbols::write_field(depthOfFieldBehaviour, DepthOfFieldBehaviour_focusDistance_field, 1000.0f);
			il2cpp_symbols::write_field(depthOfFieldBehaviour, DepthOfFieldBehaviour_aperture_field, 32.0f);
			il2cpp_symbols::write_field(depthOfFieldBehaviour, DepthOfFieldBehaviour_focalLength_field, 1.0f);

			// printf("DepthOfFieldClip_CreatePlayable, focusDistance: %f, aperture: %f, focalLength: %f\n", focusDistance, aperture, focalLength);
		}

		reinterpret_cast<decltype(PostProcess_DepthOfFieldClip_CreatePlayable_hook)*>(PostProcess_DepthOfFieldClip_CreatePlayable_orig)(retstr, _this, graph, go, mtd);
	}

	// 已过时
	void* Live_SetEnableDepthOfField_orig;
	void Live_SetEnableDepthOfField_hook(void* _this, bool isEnable) {
		if (g_enable_free_camera) {
			isEnable = false;
		}
		return reinterpret_cast<decltype(Live_SetEnableDepthOfField_hook)*>(Live_SetEnableDepthOfField_orig)(_this, isEnable);
	}

	// 未hook
	void* Live_Update_orig;
	void Live_Update_hook(void* _this) {
		reinterpret_cast<decltype(Live_Update_hook)*>(Live_Update_orig)(_this);
		if (g_enable_free_camera) {
			// Live_SetEnableDepthOfField_hook(_this, false);
		}
	}

	bool isCancelTryOn = false;
	void* LiveCostumeChangeView_setTryOnMode_orig;
	void LiveCostumeChangeView_setTryOnMode_hook(void* _this, void* idol, bool isTryOn) {
		if (!isTryOn) isCancelTryOn = true;
		reinterpret_cast<decltype(LiveCostumeChangeView_setTryOnMode_hook)*>(LiveCostumeChangeView_setTryOnMode_orig)(_this, idol, isTryOn);
		isCancelTryOn = false;
	}

	void* LiveCostumeChangeView_setIdolCostume_orig;
	void LiveCostumeChangeView_setIdolCostume_hook(void* _this, void* idol, int category, int costumeId) {
		if (g_allow_use_tryon_costume) {
			static auto iidol_klass = il2cpp_symbols::get_class_from_instance(idol);
			static auto get_CharacterId_mtd = il2cpp_class_get_method_from_name(iidol_klass, "get_CharacterId", 0);
			if (get_CharacterId_mtd) {
				const auto idolId = reinterpret_cast<int (*)(void*)>(get_CharacterId_mtd->methodPointer)(idol);
				printf("LiveCostumeChangeView_setIdolCostume idol: %d, category: %d, costumeId: %d\n", idolId, category, costumeId);
			}
			if (isCancelTryOn) return;
		}

		return reinterpret_cast<decltype(LiveCostumeChangeView_setIdolCostume_hook)*>(LiveCostumeChangeView_setIdolCostume_orig)(_this, idol, category, costumeId);
	}

	void* (*LiveCostumeChangeModel_get_Idol)(void*);

#define replaceResourceId(currHook, currOrig) \
	const auto idol = LiveCostumeChangeModel_get_Idol(_this);\
	static auto iidol_klass = il2cpp_symbols::get_class_from_instance(idol);\
	static auto get_CharacterId_mtd = il2cpp_class_get_method_from_name(iidol_klass, "get_CharacterId", 0);\
	const auto idolId = reinterpret_cast<int (*)(void*)>(get_CharacterId_mtd->methodPointer)(idol);\
	const auto resId = il2cpp_symbols::read_field<int>(ret, resourceId_field);\
	printf(#currHook" idol: %d, dressId: %d, ResId: %d, this at %p\n", idolId, id, resId, _this);\
	\
	int replaceDressId = baseId + idolId * 1000;\
	\
	auto replaceResId = id == replaceDressId ? baseResId : il2cpp_symbols::read_field<int>(ret, resourceId_field);\
	auto newRet = reinterpret_cast<decltype(currHook)*>(currOrig)(_this, replaceDressId);\
	if (!newRet) {\
		printf("ReplaceId: %d not found. try++\n", replaceDressId);\
		replaceDressId++;\
		newRet = reinterpret_cast<decltype(currHook)*>(currOrig)(_this, replaceDressId);\
		if (!newRet){\
			printf("replaceId: %d not found. Replace failed.\n", replaceDressId);\
			return ret;\
		}\
		replaceResId = id == replaceDressId ? baseResId : il2cpp_symbols::read_field<int>(ret, resourceId_field);\
	}\
	il2cpp_symbols::write_field(newRet, resourceId_field, replaceResId)


	void* LiveCostumeChangeModel_GetDress_orig;
	void* LiveCostumeChangeModel_GetDress_hook(void* _this, int id) {  // 替换服装 ResID
		auto ret = reinterpret_cast<decltype(LiveCostumeChangeModel_GetDress_hook)*>(LiveCostumeChangeModel_GetDress_orig)(_this, id);
		if (!g_unlock_all_dress) return ret;
		if (!ret) {
			printf("LiveCostumeChangeModel_GetDress returns NULL ResId: %d, this at %p\n", id, _this);
			return ret;
		}
		static auto ret_klass = il2cpp_symbols::get_class_from_instance(ret);
		static auto resourceId_field = il2cpp_class_get_field_from_name(ret_klass, "resourceId_");

		const auto baseId = 100001;
		const auto baseResId = 1;
		replaceResourceId(LiveCostumeChangeModel_GetDress_hook, LiveCostumeChangeModel_GetDress_orig);
		return newRet;
	}

	void* LiveCostumeChangeModel_GetAccessory_orig;
	void* LiveCostumeChangeModel_GetAccessory_hook(void* _this, int id) {  // 替换饰品 ResID
		auto ret = reinterpret_cast<decltype(LiveCostumeChangeModel_GetAccessory_hook)*>(LiveCostumeChangeModel_GetAccessory_orig)(_this, id);
		if (!(g_unlock_all_dress && g_unlock_all_headwear)) return ret;
		if (!ret) {
			printf("LiveCostumeChangeModel_GetAccessory returns NULL ResId: %d, this at %p\n", id, _this);
			return ret;
		}
		static auto ret_klass = il2cpp_symbols::get_class_from_instance(ret);
		static auto resourceId_field = il2cpp_class_get_field_from_name(ret_klass, "resourceId_");
		static auto accessoryType_field = il2cpp_class_get_field_from_name(ret_klass, "accessoryType_");

		const auto accessoryType = il2cpp_symbols::read_field<int>(ret, accessoryType_field);
		printf("accessoryType: %d\n", accessoryType);
		int baseId;
		int baseResId;
		switch (accessoryType) {
		case 0x1: {  // Glasses
			baseId = 800006;
			baseResId = 1103400;
		}; break;
		case 0x2: {  // Earrings
			baseId = 800001;
			baseResId = 2101300;
		}; break;
		case 0x3: {  // Makeup
			baseId = 800009;
			baseResId = 9100103;
		}; break;
		default: return ret;
		}

		replaceResourceId(LiveCostumeChangeModel_GetAccessory_hook, LiveCostumeChangeModel_GetAccessory_orig);
		// LiveCostumeChangeModel_GetAccessory_hook idol : 15, dressId : 815004, ResId : 1103200, this at 0000021E62ADDD80
		return newRet;
	}

	void* LiveCostumeChangeModel_GetHairstyle_orig;
	void* LiveCostumeChangeModel_GetHairstyle_hook(void* _this, int id) {  // 替换头发 ResID
		auto ret = reinterpret_cast<decltype(LiveCostumeChangeModel_GetHairstyle_hook)*>(LiveCostumeChangeModel_GetHairstyle_orig)(_this, id);
		if (!(g_unlock_all_dress && g_unlock_all_headwear)) return ret;
		if (!ret) {
			printf("LiveCostumeChangeModel_GetHairstyle returns NULL ResId: %d, this at %p\n", id, _this);
			return ret;
		}
		static auto ret_klass = il2cpp_symbols::get_class_from_instance(ret);
		static auto resourceId_field = il2cpp_class_get_field_from_name(ret_klass, "resourceId_");

		const auto baseId = 600001;
		const auto baseResId = 1;
		replaceResourceId(LiveCostumeChangeModel_GetHairstyle_hook, LiveCostumeChangeModel_GetHairstyle_orig);

		static auto accessoryResourceIdList_field = il2cpp_class_get_field_from_name(ret_klass, "accessoryResourceIdList_");
		const auto replaceaccessoryResourceIdList = il2cpp_symbols::read_field(ret, accessoryResourceIdList_field);
		if (id == replaceDressId) {
			static auto RepeatedField_klass = il2cpp_symbols::get_class_from_instance(replaceaccessoryResourceIdList);
			static auto RepeatedField_Add_mtd = il2cpp_class_get_method_from_name(RepeatedField_klass, "Add", 1);
			static auto RepeatedField_Add = reinterpret_cast<void (*)(void*, int, void* mtd)>(RepeatedField_Add_mtd->methodPointer);
			static auto RepeatedField_ctor_mtd = il2cpp_class_get_method_from_name(RepeatedField_klass, ".ctor", 0);
			static auto RepeatedField_ctor = reinterpret_cast<void (*)(void*, void* mtd)>(RepeatedField_ctor_mtd->methodPointer);

			auto newRepeatedField = il2cpp_object_new(RepeatedField_klass);
			RepeatedField_ctor(newRepeatedField, RepeatedField_ctor_mtd);
			RepeatedField_Add(newRepeatedField, 0, RepeatedField_ctor_mtd);
			RepeatedField_Add(newRepeatedField, 0, RepeatedField_ctor_mtd);
			il2cpp_symbols::write_field(newRet, accessoryResourceIdList_field, newRepeatedField);
		}
		else {
			il2cpp_symbols::write_field(newRet, accessoryResourceIdList_field, replaceaccessoryResourceIdList);
		}

		return newRet;
	}

	bool confirmationingModel = false;
	std::map<int, void*> cacheDressMap{};
	std::map<int, void*> cacheHairMap{};
	std::map<int, void*> cacheAccessoryMap{};

	void* LiveCostumeChangeModel_ctor_orig;
	void LiveCostumeChangeModel_ctor_hook(void* _this, void* reply, void* idol, int costumeType, bool forceDressOrdered) {  // 添加服装到 dressDic
		/*
		static auto iidol_klass = il2cpp_symbols::get_class_from_instance(idol);
		static auto get_CharacterId_mtd = il2cpp_class_get_method_from_name(iidol_klass, "get_CharacterId", 0);
		if (get_CharacterId_mtd) {
			const auto idolId = reinterpret_cast<int (*)(void*)>(get_CharacterId_mtd->methodPointer)(idol);
			printf("LiveCostumeChangeModel_ctor idolId: %d, costumeType: %d\n", idolId, costumeType);
		}
		*/
		reinterpret_cast<decltype(LiveCostumeChangeModel_ctor_hook)*>(LiveCostumeChangeModel_ctor_orig)(_this, reply, idol, costumeType, forceDressOrdered);

		if (g_unlock_all_dress && (costumeType == 1)) {
			static auto LiveCostumeChangeModel_klass = il2cpp_symbols::get_class("PRISM.Adapters.dll", "PRISM.Adapters", "LiveCostumeChangeModel");
			static auto dressDic_field = il2cpp_class_get_field_from_name(LiveCostumeChangeModel_klass, "dressDic");
			static auto hairstyleDic_field = il2cpp_class_get_field_from_name(LiveCostumeChangeModel_klass, "hairstyleDic");
			static auto accessoryDic_field = il2cpp_class_get_field_from_name(LiveCostumeChangeModel_klass, "accessoryDic");

			auto dressDicInstance = il2cpp_field_get_value_object(dressDic_field, _this);
			// auto hairstyleDicInstance = il2cpp_field_get_value_object(hairstyleDic_field, _this);
			// auto accessoryDicInstance = il2cpp_field_get_value_object(accessoryDic_field, _this);

			auto dressDic = Utils::CSDictEditor<int>(dressDicInstance, "System.Collections.Generic.Dictionary`2[System.Int32, PRISM.Module.Networking.ICostumeStatus]");
			// auto hairstyleDic = Utils::CSDictEditor<int>(hairstyleDicInstance, "System.Collections.Generic.Dictionary`2[System.Int32, PRISM.Module.Networking.IHairstyleStatus]");
			// auto accessoryDic = Utils::CSDictEditor<int>(accessoryDicInstance, "System.Collections.Generic.Dictionary`2[System.Int32, PRISM.Module.Networking.IAccessoryStatus]");

			const auto addToDic = [](std::map<int, void*>& cacheMap, Utils::CSDictEditor<int>& dict) {
				for (auto& i : cacheMap) {
					const auto currId = i.first;
					if (!dict.ContainsKey(currId)) {
						auto& costumeStat = i.second;
						// printf("add: %p\n", costumeStat);
						dict.Add(currId, costumeStat);
					}
				}};

			addToDic(cacheDressMap, dressDic);
			// addToDic(cacheHairMap, hairstyleDic);  // TODO PRISM.ResourceManagement.ResourceLoader._throwMissingKeyException
			// addToDic(cacheAccessoryMap, accessoryDic);  // TODO PRISM.ResourceManagement.ResourceLoader._throwMissingKeyException
		}
	}


	void* CostumeChangeViewModel_ctor_orig;
	void CostumeChangeViewModel_ctor_hook(void* _this, int characterId, void* costumeService, bool isCostumeUnlimited) {
		if (g_override_isCostumeUnlimited) {
			if (isCostumeUnlimited) {
				printf("isCostumeUnlimited is already true.\n");
			}
			else {
				isCostumeUnlimited = true;
				printf("isCostumeUnlimited is overriden to true.\n");
			}
		}
		return reinterpret_cast<decltype(CostumeChangeViewModel_ctor_hook)*>(CostumeChangeViewModel_ctor_orig)(_this, characterId, costumeService, isCostumeUnlimited);
	}

	// [Muitsonz/#1](https://github.com/Muitsonz/scsp-localify/issues/1)
	void* CostumeChangeView_Reload_orig;
	void* CostumeChangeView_Reload_hook(void* _this, void* viewModel) {
		auto ret = reinterpret_cast<decltype(CostumeChangeView_Reload_hook)*>(CostumeChangeView_Reload_orig)(_this, viewModel);

		static void* klass_CostumeChangeViewModel;
		static MethodInfo* mtd_CostumeChangeViewModel_GetPreviewUnitIdol;
		static managed::UnitIdol* (*func_CostumeChangeViewModel_GetPreviewUnitIdol)(void* _this, void* mtd);

		if (g_save_and_replace_costume_changes) {
			__try {
				if (klass_CostumeChangeViewModel == nullptr) {
					klass_CostumeChangeViewModel = il2cpp_symbols::get_class_from_instance(viewModel);
					mtd_CostumeChangeViewModel_GetPreviewUnitIdol = il2cpp_class_get_method_from_name(klass_CostumeChangeViewModel, "GetPreviewUnitIdol", 0);
					func_CostumeChangeViewModel_GetPreviewUnitIdol = reinterpret_cast<managed::UnitIdol * (*)(void* _this, void* mtd)>(mtd_CostumeChangeViewModel_GetPreviewUnitIdol->methodPointer);
				}

				auto idol = func_CostumeChangeViewModel_GetPreviewUnitIdol(viewModel, mtd_CostumeChangeViewModel_GetPreviewUnitIdol);

				UnitIdol data;
				data.ReadFrom(idol);
				std::cout << "Saved UnitIdel = ";
				data.Print(std::cout);

				if (data.CharaId >= 0)
					savedCostumes[data.CharaId] = data;

				lastSavedCostume = data;
			}
			__except (seh_filter(GetExceptionInformation())) {
				printf("SEH exception detected in 'CostumeChangeView_Reload_hook'.\n");
			}
		}

		return ret;
	}

	// [Muitsonz/#1](https://github.com/Muitsonz/scsp-localify/issues/1)
	void* LiveMVStartData_ctor_orig;
	void* LiveMVStartData_ctor_hook(void* _this, void* musicMaster, void* onStageIdols, int cameraIndex, bool isVocalSeparatedOn, int backgroundMode, int renderingDynamicRange, int soundEffectMode) {
		if (g_override_isVocalSeparatedOn) {
			if (isVocalSeparatedOn) {
				printf("isVocalSeparatedOn is already true.\n");
			}
			else {
				isVocalSeparatedOn = true;
				printf("isVocalSeparatedOn is overriden to true.\n");
			}
		}

		auto ret = reinterpret_cast<decltype(LiveMVStartData_ctor_hook)*>(LiveMVStartData_ctor_orig)(_this, musicMaster, onStageIdols, cameraIndex, isVocalSeparatedOn, backgroundMode, renderingDynamicRange, soundEffectMode);

		__try {
			auto idolsLength = il2cpp_array_length(onStageIdols);
			if (g_save_and_replace_costume_changes) {
				for (int i = 0; i < idolsLength; i++) {
					auto item = (managed::UnitIdol*)il2cpp_symbols::array_get_value(onStageIdols, i);

					UnitIdol idol;
					idol.ReadFrom(item);

					auto it = savedCostumes.find(idol.CharaId);
					if (it != savedCostumes.end()) {
						it->second.ApplyTo(item);
						std::cout << "CharaId " << it->first << " has been modified." << std::endl;
					}
				}
			}
			if (g_save_and_replace_costume_changes && g_overrie_mv_unit_idols) {
				auto loopMax = idolsLength;
				if (idolsLength > overridenMvUnitIdols_length) {
					printf("[WARNING]: `onStageIdols.Length` = %d, greater than expected `overridenMvUnitIdols_length`.\n", idolsLength);
					loopMax = overridenMvUnitIdols_length;
				}
				for (int i = 0; i < idolsLength; ++i) {
					if (!overridenMvUnitIdols[i].IsEmpty()) {
						auto item = (managed::UnitIdol*)il2cpp_symbols::array_get_value(onStageIdols, i);
						overridenMvUnitIdols[i].ApplyTo(item);
						printf("MV unit idol [%d] is overriden.\n", i);
					}
				}
			}
		}
		__except (seh_filter(GetExceptionInformation())) {
			printf("SEH exception detected in `LiveMVStartData_ctor_hook`.\n");
		}

		return ret;
	}


	int getCharacterParamAliveCount(const std::string& objName) {
		int aliveCount = 0;
		for (auto& i : charaParam) {
			if (i.first.starts_with(objName)) {
				if (i.second.checkObjAlive()) {
					aliveCount++;
				}
			}
		}
		return aliveCount;
	}

	void AssembleCharacter_ApplyParam_hook(void* mdl, float height, float bust, float head, float arm, float hand) {
		if (g_enable_chara_param_edit) {
			static auto get_ObjectName = reinterpret_cast<Il2CppString * (*)(void*)>(
				il2cpp_symbols::get_method_pointer(
					"UnityEngine.CoreModule.dll", "UnityEngine",
					"Object", "GetName", 0
				)
				);
			const auto objNameIlStr = get_ObjectName(mdl);
			const std::string objName = objNameIlStr ? utility::conversions::to_utf8string(std::wstring(objNameIlStr->start_char)) : std::format("Unnamed Obj {:p}", mdl);
			std::string showObjName;
			if (objName.starts_with("m_ALL_")) {
				const auto nextFlagPos = objName.find_first_of(L'_', 6);
				if (nextFlagPos > 6) {
					showObjName = objName.substr(0, nextFlagPos);
				}
				else {
					showObjName = objName;
				}

				const auto objAliveCount = getCharacterParamAliveCount(showObjName);
				if (objAliveCount > 0) {
					showObjName = std::format("{} [{}]", showObjName, objAliveCount);
				}
				/*
				if (auto it = charaParam.find(showObjName); it != charaParam.end()) {
					if (it->second.checkObjAlive()) {
						showObjName = std::format("{} ({:p})", showObjName, mdl);
					}
				}*/
			}
			else {
				showObjName = objName;
			}
			if (auto it = charaParam.find(showObjName); it != charaParam.end()) {
				it->second.SetObjPtr(mdl);
				it->second.UpdateParam(&height, &bust, &head, &arm, &hand);
				return it->second.Apply();
			}
			else {
				charaParam.emplace(showObjName, CharaParam_t(height, bust, head, arm, hand, mdl));
			}
		}
		return reinterpret_cast<decltype(AssembleCharacter_ApplyParam_hook)*>(AssembleCharacter_ApplyParam_orig)(mdl, height, bust, head, arm, hand);
	}

	void* MainThreadDispatcher_LateUpdate_orig;
	void MainThreadDispatcher_LateUpdate_hook(void* _this, void* method) {
		try {
			auto it = mainThreadTasks.begin();
			while (it != mainThreadTasks.end()) {
				try {
					bool result = (*it)();
					if (result) {
						it = mainThreadTasks.erase(it);
					}
					else {
						++it;
					}
				}
				catch (std::exception& e) {
					printf("MainThreadDispatcher Tasks Error: %s\n", e.what());
					it = mainThreadTasks.erase(it);
				}
			}
		}
		catch (std::exception& ex) {
			printf("MainThreadDispatcher Error: %s\n", ex.what());
		}
		return reinterpret_cast<decltype(MainThreadDispatcher_LateUpdate_hook)*>(MainThreadDispatcher_LateUpdate_orig)(_this, method);
	}

	void checkAndAddCostume(int key, void* value) {
		static auto CostumeStatus_klass = il2cpp_symbols::get_class("PRISM.Module.Networking.dll",
			"PRISM.Module.Networking.Stub.Status", "CostumeStatus");
		static auto HairstyleStatus_klass = il2cpp_symbols::get_class("PRISM.Module.Networking.dll",
			"PRISM.Module.Networking.Stub.Status", "HairstyleStatus");
		static auto AccessoryStatus_klass = il2cpp_symbols::get_class("PRISM.Module.Networking.dll",
			"PRISM.Module.Networking.Stub.Status", "AccessoryStatus");
		auto value_klass = il2cpp_symbols::get_class_from_instance(value);

#define unlockAndAddToMap(klass, cacheMap) \
		static auto klass##_isUnlocked_field = il2cpp_class_get_field_from_name(klass, "isUnlocked_"); \
		il2cpp_symbols::write_field(value, klass##_isUnlocked_field, true); \
		cacheMap.emplace(key, value)

		if (value_klass == CostumeStatus_klass) {
			// wprintf(L"dic_int_ICostumeStatus_add: this: %p, key: %d\n", _this, key);
			unlockAndAddToMap(CostumeStatus_klass, cacheDressMap);
		}
		else if (value_klass == HairstyleStatus_klass) {
			if (g_unlock_all_headwear) {
				unlockAndAddToMap(HairstyleStatus_klass, cacheHairMap);
			}
		}
		else if (value_klass == AccessoryStatus_klass) {
			if (g_unlock_all_headwear) {
				unlockAndAddToMap(AccessoryStatus_klass, cacheAccessoryMap);
			}
		}
	}

	void* dic_int_ICostumeStatus_add_orig;
	void dic_int_ICostumeStatus_add_hook(void* _this, int key, void* value, MethodInfo* method) {  // 添加服装到缓存表(失效)
		if ((g_unlock_all_dress || g_allow_use_tryon_costume) && confirmationingModel) checkAndAddCostume(key, value);
		return reinterpret_cast<decltype(dic_int_ICostumeStatus_add_hook)*>(dic_int_ICostumeStatus_add_orig)(_this, key, value, method);
	}

	void checkCostumeListReply(void* _this, void* ret, const char* costumeIdFieldName) {
		if (g_unlock_all_dress || g_allow_use_tryon_costume) {
			static auto GetCostumeListReply_klass = il2cpp_symbols::get_class("PRISM.Module.Networking.dll",
				"PRISM.Module.Networking.Stub.Api", "GetCostumeListReply");
			auto this_klass = il2cpp_symbols::get_class_from_instance(_this);

			if (this_klass == GetCostumeListReply_klass) {
				void* CostumeStatus_klass = NULL;
				FieldInfo* mstCostumeId_field;

				Utils::CSListEditor retList(ret);
				for (auto i : retList) {
					if (CostumeStatus_klass == NULL) {
						CostumeStatus_klass = il2cpp_symbols::get_class_from_instance(i);
						mstCostumeId_field = il2cpp_class_get_field_from_name(CostumeStatus_klass, costumeIdFieldName);
					}
					const auto mstCostumeId = il2cpp_symbols::read_field<int>(i, mstCostumeId_field);
					checkAndAddCostume(mstCostumeId, i);
				}
			}
		}
	}

	void* GetCostumeListReply_get_CostumeList_orig;
	void* GetCostumeListReply_get_CostumeList_hook(void* _this) {  // 添加服装到缓存表
		auto ret = reinterpret_cast<decltype(GetCostumeListReply_get_CostumeList_hook)*>(GetCostumeListReply_get_CostumeList_orig)(_this);
		checkCostumeListReply(_this, ret, "mstCostumeId_");
		return ret;
	}

	// The first registered auto getter
	void* GetCostumeListReply_get_HairstyleList_orig;
	void* GetCostumeListReply_get_HairstyleList_hook(void* _this) {  // 添加服装到缓存表
		auto ret = reinterpret_cast<decltype(GetCostumeListReply_get_HairstyleList_hook)*>(GetCostumeListReply_get_HairstyleList_orig)(_this);
		checkCostumeListReply(_this, ret, "mstHairstyleId_");
		return ret;
	}

	void* GetCostumeListReply_get_AccessoryList_orig;
	void* GetCostumeListReply_get_AccessoryList_hook(void* _this) {  // 添加服装到缓存表
		auto ret = reinterpret_cast<decltype(GetCostumeListReply_get_AccessoryList_hook)*>(GetCostumeListReply_get_AccessoryList_orig)(_this);
		checkCostumeListReply(_this, ret, "mstAccessoryId_");
		return ret;
	}

	void* LiveMVUnitConfirmationModel_ctor_orig;
	void LiveMVUnitConfirmationModel_ctor_hook(void* _this, void* musicData, void* saveData, void* unitListReply, void* costumeService) {
		confirmationingModel = true;
		cacheDressMap.clear();
		cacheHairMap.clear();
		cacheAccessoryMap.clear();

		reinterpret_cast<decltype(LiveMVUnitConfirmationModel_ctor_hook)*>(LiveMVUnitConfirmationModel_ctor_orig)(_this, musicData, saveData, unitListReply, costumeService);
		confirmationingModel = false;
		return;
	}

	void updateSwayStringPoint(void* _this) {
		if (!g_enable_chara_param_edit) return;

		static auto SwayString_klass = il2cpp_symbols::get_class("PRISM.Legacy.dll", "PRISM", "SwayString");
		static auto swayType_field = il2cpp_class_get_field_from_name(SwayString_klass, "swayType");
		static auto swaySubType_field = il2cpp_class_get_field_from_name(SwayString_klass, "swaySubType");
		static auto forceParam_field = il2cpp_class_get_field_from_name(SwayString_klass, "forceParam");

		static auto SetTest = reinterpret_cast<void (*)(void*, int swayType, float bendStrength, float baseGravity, float inertiaMoment, float airResistance)>(
			il2cpp_symbols::get_method_pointer("PRISM.Legacy.dll", "PRISM", "SwayString", "SetupForEditor", 5)
			);
		static auto SetRate = reinterpret_cast<void (*)(void*, float)>(
			il2cpp_symbols::get_method_pointer("PRISM.Legacy.dll", "PRISM", "SwayString", "SetRate", 1)
			);
		static auto GetRate = reinterpret_cast<float (*)(void*)>(
			il2cpp_symbols::get_method_pointer("PRISM.Legacy.dll", "PRISM", "SwayString", "GetRate", 0)
			);

		static std::unordered_set<int> targetTypes{ 0x7, 0x8 };
		const auto swayType = il2cpp_symbols::read_field<int>(_this, swayType_field);
		const auto swaySubType = il2cpp_symbols::read_field<int>(_this, swaySubType_field);
		if (!charaSwayStringOffset.contains(swayType)) return;
		const auto& currConfig = charaSwayStringOffset[swayType];
		if (!currConfig.isEdited()) return;

		const auto rate = GetRate(_this);
		SetRate(_this, rate + currConfig.rate);

		auto forceParam = il2cpp_field_get_value_object(forceParam_field, _this);
		int currIndex = 0;
		il2cpp_symbols::iterate_IEnumerable(forceParam, [&](void* param) {
			if (currIndex == swayType) {
				if (!param) {
					// printf("Invalid param.\n");
					return;
				}
				static auto forceParam_klass = il2cpp_symbols::get_class_from_instance(param);
				static auto bendStrength_field = il2cpp_class_get_field_from_name(forceParam_klass, "bendStrength");
				static auto baseGravity_field = il2cpp_class_get_field_from_name(forceParam_klass, "baseGravity");
				static auto inertiaMoment_field = il2cpp_class_get_field_from_name(forceParam_klass, "inertiaMoment");
				static auto airResistance_field = il2cpp_class_get_field_from_name(forceParam_klass, "airResistance");
				static auto deformResistance_field = il2cpp_class_get_field_from_name(forceParam_klass, "deformResistance");

				const auto bendStrength = il2cpp_symbols::read_field<float>(param, bendStrength_field);
				const auto baseGravity = il2cpp_symbols::read_field<float>(param, baseGravity_field);
				const auto inertiaMoment = il2cpp_symbols::read_field<float>(param, inertiaMoment_field);
				const auto airResistance = il2cpp_symbols::read_field<float>(param, airResistance_field);
				const auto deformResistance = il2cpp_symbols::read_field<float>(param, deformResistance_field);

				printf("SwayString original: swayType: %d, swaySubType: %d, rate: %f, bendStrength: %f, baseGravity: %f, "
					"inertiaMoment: %f, airResistance: %f, deformResistance: %f\n",
					swayType, swaySubType, rate, bendStrength, baseGravity, inertiaMoment, airResistance, deformResistance);

				il2cpp_symbols::write_field(param, bendStrength_field, bendStrength + currConfig.P_bendStrength);
				il2cpp_symbols::write_field(param, baseGravity_field, baseGravity + currConfig.P_baseGravity);
				il2cpp_symbols::write_field(param, inertiaMoment_field, inertiaMoment + currConfig.P_inertiaMoment);
				il2cpp_symbols::write_field(param, airResistance_field, airResistance + currConfig.P_airResistance);
				il2cpp_symbols::write_field(param, deformResistance_field, deformResistance + currConfig.P_deformResistance);
			}

			currIndex++;
			});
	}

	void* SwayString_SetupPoint_orig;
	void SwayString_SetupPoint_hook(void* _this) {
		if (g_enable_chara_param_edit) {
			static auto SwayString_klass = il2cpp_symbols::get_class("PRISM.Legacy.dll", "PRISM", "SwayString");
			auto this_klass = il2cpp_symbols::get_class_from_instance(_this);
			if (this_klass == SwayString_klass) {
				updateSwayStringPoint(_this);
			}
		}
		reinterpret_cast<decltype(SwayString_SetupPoint_hook)*>(SwayString_SetupPoint_orig)(_this);
	}

	void* LiveMVUnit_GetMemberChangeRequestData_orig;
	void* LiveMVUnit_GetMemberChangeRequestData_hook(void* _this, int position, void* idol, int exchangePosition) {
		if (g_allow_same_idol) {  // 此方法已过时
			exchangePosition = -1;
		}
		return reinterpret_cast<decltype(LiveMVUnit_GetMemberChangeRequestData_hook)*>(LiveMVUnit_GetMemberChangeRequestData_orig)(_this, position, idol, exchangePosition);
	}

	int slotNewCount = 0;
	bool settingLiveIdolSlot = false;
	void* lastIdol = NULL;

	void* MvUnitSlotGenerator_NewMvUnitSlot_orig;
	void* MvUnitSlotGenerator_NewMvUnitSlot_hook(int slot, void* idol, void* method) {
		if (g_allow_same_idol && settingLiveIdolSlot) {
			if (slotNewCount >= 1) {
				printf("catch exchange.\n");
				idol = lastIdol;
			}
			else {
				slotNewCount++;
				lastIdol = idol;
			}
		}
		return reinterpret_cast<decltype(MvUnitSlotGenerator_NewMvUnitSlot_hook)*>(MvUnitSlotGenerator_NewMvUnitSlot_orig)(slot, idol, method);
	}

	bool checkMusicDataSatisfy(void* _this, void* unit, int pos = -1) {
		static auto musicData_klass = il2cpp_symbols_logged::get_class("PRISM.Legacy.dll", "PRISM.Live", "MusicData");
		static auto master_klass = il2cpp_symbols_logged::get_class("PRISM.Definitions.dll", "PRISM.Definitions", "MstSong");

		static auto masterData_field = il2cpp_symbols_logged::il2cpp_class_get_field_from_name(musicData_klass, "<Master>k__BackingField");
		static auto isSongParts_field = il2cpp_symbols_logged::il2cpp_class_get_field_from_name(master_klass, "<IsSongParts>k__BackingField");

		const auto masterData = il2cpp_symbols::read_field(_this, masterData_field);
		const auto isSongParts = il2cpp_symbols::read_field<bool>(masterData, isSongParts_field);
		if (!isSongParts) return false;

		static auto klass_LiveUnit = il2cpp_symbols::get_class_from_instance(unit);
		static auto mtd_LiveUnit_get_Idols = il2cpp_class_get_method_from_name(klass_LiveUnit, "get_Idols", 0);
		static auto func_LiveUnit_get_Idols = reinterpret_cast<void* (*)(void*, MethodInfo*)>(mtd_LiveUnit_get_Idols->methodPointer);

		static auto MusicData_IsOriginalMember = reinterpret_cast<bool (*)(void*, int)>(
			il2cpp_symbols_logged::get_method_pointer("PRISM.Legacy.dll", "PRISM.Live", "MusicData", "IsOriginalMember", 1)
			);

		auto idols = func_LiveUnit_get_Idols(unit, mtd_LiveUnit_get_Idols);
		bool ret = true;
		int currentSlot = 0;
		il2cpp_symbols::iterate_IEnumerable(idols, [&](void* idol) {
			__try {
				const auto idol_klass = il2cpp_symbols::get_class_from_instance(idol);
				const auto characterId_field = il2cpp_class_get_field_from_name(idol_klass, "<CharacterId>k__BackingField");
				//const auto characterId = il2cpp_symbols::read_field<int>(idol, characterId_field);
				const auto get_CharacterId = reinterpret_cast<int (*)(void*)>(
					il2cpp_symbols::get_method_pointer("PRISM.Legacy.dll", "PRISM.Live", "LiveIdol", "get_CharacterId", 0)
					);
				const auto characterId = get_CharacterId(idol);
				const auto isOrigMember = MusicData_IsOriginalMember(_this, characterId);
				if (!isOrigMember) {
					if (pos == -1) {
						ret = false;
					}
					else {
						if (currentSlot == pos) ret = false;
					}
				}
				currentSlot++;
			}
			__except (seh_filter(GetExceptionInformation())) {
				printf("SEH exception detected in `checkMusicDataSatisfy|iterate_IEnumerable`.\n");
			}
			});

		return ret;
	}

	void* CheckVocalSeparatedSatisfy_orig;
	bool CheckVocalSeparatedSatisfy_hook(void* _this, void* unit, void* mtd) {
		if (!g_allow_same_idol) {
			return reinterpret_cast<decltype(CheckVocalSeparatedSatisfy_hook)*>(CheckVocalSeparatedSatisfy_orig)(
				_this, unit, mtd);
		}

		return checkMusicDataSatisfy(_this, unit);
	}

	void* CheckLimitedVocalSeparatedSatisfy_2_orig;
	bool CheckLimitedVocalSeparatedSatisfy_2_hook(void* _this, void* unit, int pos, void* mtd) {
		if (!g_allow_same_idol) {
			return reinterpret_cast<decltype(CheckLimitedVocalSeparatedSatisfy_2_hook)*>(CheckLimitedVocalSeparatedSatisfy_2_orig)(
				_this, unit, pos, mtd);
		}
		// printf("CheckLimitedVocalSeparatedSatisfy: %d\n", pos);
		return checkMusicDataSatisfy(_this, unit, pos);
	}

	void* LiveMVUnitMemberChangePresenter_initializeAsync_b_4_MoveNext_orig;
	void LiveMVUnitMemberChangePresenter_initializeAsync_b_4_MoveNext_hook(void* _this, MethodInfo* method) {
		slotNewCount = 0;
		settingLiveIdolSlot = true;

		reinterpret_cast<decltype(LiveMVUnitMemberChangePresenter_initializeAsync_b_4_MoveNext_hook)*>
			(LiveMVUnitMemberChangePresenter_initializeAsync_b_4_MoveNext_orig)(_this, method);

		slotNewCount = 0;
		settingLiveIdolSlot = false;
	}

	void* CriWareErrorHandler_HandleMessage_orig;
	void CriWareErrorHandler_HandleMessage_hook(void* _this, Il2CppString* msg) {
		// wprintf(L"CriWareErrorHandler_HandleMessage: %ls\n%ls\n\n", msg->start_char, environment_get_stacktrace()->start_char);
		environment_get_stacktrace();
		return reinterpret_cast<decltype(CriWareErrorHandler_HandleMessage_hook)*>(CriWareErrorHandler_HandleMessage_orig)(_this, msg);
	}

	void* UnsafeLoadBytesFromKey_orig;
	void* UnsafeLoadBytesFromKey_hook(void* _this, Il2CppString* tagName, Il2CppString* assetKey) {
		auto ret = reinterpret_cast<decltype(UnsafeLoadBytesFromKey_hook)*>(UnsafeLoadBytesFromKey_orig)(_this, tagName, assetKey);
		// wprintf(L"UnsafeLoadBytesFromKey: tag: %ls, assetKey: %ls\n", tagName->start_char, assetKey->start_char);
		// dumpByteArray(tagName->start_char, assetKey->start_char, ret);
		return ret;
	}

	void* TextLog_AddLog_orig;
	void TextLog_AddLog_hook(void* _this, int speakerFlag, Il2CppString* textID, Il2CppString* text, bool isChoice) {
		return reinterpret_cast<decltype(TextLog_AddLog_hook)*>(TextLog_AddLog_orig)(_this, speakerFlag, textID, text, isChoice);

		static auto TextLog_klass = il2cpp_symbols::get_class_from_instance(_this);
		static auto dicConvertID_field = il2cpp_class_get_field_from_name(TextLog_klass, "dicConvertID");
		static auto speakerTable_field = il2cpp_class_get_field_from_name(TextLog_klass, "speakerTable");
		static auto listTextLogData_field = il2cpp_class_get_field_from_name(TextLog_klass, "listTextLogData");
		static auto unitIdol_field = il2cpp_class_get_field_from_name(TextLog_klass, "unitIdol");
		static auto convertList_field = il2cpp_class_get_field_from_name(TextLog_klass, "convertList");

		static auto toJsonStr = reinterpret_cast<Il2CppString * (*)(void*)>(
			il2cpp_symbols::get_method_pointer("Newtonsoft.Json.dll", "Newtonsoft.Json",
				"JsonConvert", "SerializeObject", 1)
			);

		auto dicConvertID = il2cpp_field_get_value_object(dicConvertID_field, _this);
		auto speakerTable = il2cpp_field_get_value_object(speakerTable_field, _this);
		auto listTextLogData = il2cpp_field_get_value_object(listTextLogData_field, _this);
		auto unitIdol = il2cpp_field_get_value_object(unitIdol_field, _this);
		auto convertList = il2cpp_field_get_value_object(convertList_field, _this);

		wprintf(L"TextLog_AddLog: speakerFlag: %d, textID: %ls, text: %ls\ndicConvertID:\n%ls\n\nspeakerTable:\n%ls\n\nlistTextLogData:\n%ls\n\nunitIdol:\n%ls\n\nconvertList:\n%ls\n\n",
			speakerFlag, textID->start_char, text->start_char,
			toJsonStr(dicConvertID)->start_char,
			toJsonStr(speakerTable)->start_char,
			toJsonStr(listTextLogData)->start_char,
			toJsonStr(unitIdol)->start_char,
			toJsonStr(convertList)->start_char
		);
	}

	void* baseCameraTransform = nullptr;
	void* baseCamera = nullptr;

	void* Unity_set_position_orig;
	void Unity_set_position_hook(void* _this, Vector3_t value) {
		return reinterpret_cast<decltype(Unity_set_position_hook)*>(Unity_set_position_orig)(_this, value);
	}

	void* Unity_get_position_orig;
	Vector3_t Unity_get_position_hook(void* _this) {
		auto data = reinterpret_cast<decltype(Unity_get_position_hook)*>(Unity_get_position_orig)(_this);

		if (_this == baseCameraTransform) {
			if (guiStarting) {
				SCGUIData::sysCamPos.x = data.x;
				SCGUIData::sysCamPos.y = data.y;
				SCGUIData::sysCamPos.z = data.z;
			}
			if (g_enable_free_camera) {
				SCCamera::baseCamera.updateOtherPos(&data);
				Unity_set_position_hook(_this, data);
			}
		}

		return data;
	}

	void* Unity_set_fieldOfView_orig;
	void Unity_set_fieldOfView_hook(void* _this, float single) {
		return reinterpret_cast<decltype(Unity_set_fieldOfView_hook)*>(Unity_set_fieldOfView_orig)(_this, single);
	}
	void* Unity_get_fieldOfView_orig;
	float Unity_get_fieldOfView_hook(void* _this) {
		const auto origFov = reinterpret_cast<decltype(Unity_get_fieldOfView_hook)*>(Unity_get_fieldOfView_orig)(_this);
		if (_this == baseCamera) {
			if (guiStarting) {
				SCGUIData::sysCamFov = origFov;
			}
			if (g_enable_free_camera) {
				const auto fov = SCCamera::baseCamera.fov;
				Unity_set_fieldOfView_hook(_this, fov);
				return fov;
			}
		}
		// printf("get_fov: %f\n", ret);
		return origFov;
	}

	void* Unity_InternalLookAt_orig;
	void Unity_InternalLookAt_hook(void* _this, Vector3_t worldPosition, Vector3_t worldUp) {
		if (_this == baseCameraTransform) {
			if (g_enable_free_camera) {
				auto pos = SCCamera::baseCamera.getLookAt();
				worldPosition.x = pos.x;
				worldPosition.y = pos.y;
				worldPosition.z = pos.z;
			}
		}
		return reinterpret_cast<decltype(Unity_InternalLookAt_hook)*>(Unity_InternalLookAt_orig)(_this, worldPosition, worldUp);
	}

	void* Unity_set_nearClipPlane_orig;
	void Unity_set_nearClipPlane_hook(void* _this, float single) {
		if (_this == baseCamera) {
			if (g_enable_free_camera && g_reenable_clipPlane) {
				single = 0.001f;
			}
		}
		return reinterpret_cast<decltype(Unity_set_nearClipPlane_hook)*>(Unity_set_nearClipPlane_orig)(_this, single);
	}

	void* Unity_get_nearClipPlane_orig;
	float Unity_get_nearClipPlane_hook(void* _this) {
		auto ret = reinterpret_cast<decltype(Unity_get_nearClipPlane_hook)*>(Unity_get_nearClipPlane_orig)(_this);
		if (_this == baseCamera) {
			if (g_enable_free_camera && g_reenable_clipPlane) {
				ret = 0.001f;
			}
		}
		return ret;
	}
	void* Unity_get_farClipPlane_orig;
	float Unity_get_farClipPlane_hook(void* _this) {
		auto ret = reinterpret_cast<decltype(Unity_get_farClipPlane_hook)*>(Unity_get_farClipPlane_orig)(_this);
		if (_this == baseCamera) {
			if (g_enable_free_camera && g_reenable_clipPlane) {
				ret = 2500.0f;
			}
		}
		return ret;
	}

	void* Unity_set_farClipPlane_orig;
	void Unity_set_farClipPlane_hook(void* _this, float value) {
		if (_this == baseCamera) {
			if (g_enable_free_camera && g_reenable_clipPlane) {
				value = 2500.0f;
			}
		}
		reinterpret_cast<decltype(Unity_set_farClipPlane_hook)*>(Unity_set_farClipPlane_orig)(_this, value);
	}

	void* Unity_set_rotation_orig;
	void Unity_set_rotation_hook(void* _this, Quaternion_t value) {
		return reinterpret_cast<decltype(Unity_set_rotation_hook)*>(Unity_set_rotation_orig)(_this, value);
	}
	void* Unity_get_rotation_orig;
	Quaternion_t Unity_get_rotation_hook(void* _this) {
		auto ret = reinterpret_cast<decltype(Unity_get_rotation_hook)*>(Unity_get_rotation_orig)(_this);

		if (_this == baseCameraTransform) {
			if (guiStarting) {
				SCGUIData::sysCamRot.w = ret.w;
				SCGUIData::sysCamRot.x = ret.x;
				SCGUIData::sysCamRot.y = ret.y;
				SCGUIData::sysCamRot.z = ret.z;
				SCGUIData::updateSysCamLookAt();
			}
			if (g_enable_free_camera) {
				ret.w = 0;
				ret.x = 0;
				ret.y = 0;
				ret.z = 0;
				Unity_set_rotation_hook(_this, ret);

				static auto Vector3_klass = il2cpp_symbols::get_class("UnityEngine.CoreModule.dll", "UnityEngine", "Vector3");
				Vector3_t* pos = reinterpret_cast<Vector3_t*>(il2cpp_object_new(Vector3_klass));
				Vector3_t* up = reinterpret_cast<Vector3_t*>(il2cpp_object_new(Vector3_klass));
				up->x = 0;
				up->y = 1;
				up->z = 0;
				Unity_InternalLookAt_hook(_this, *pos, *up);
			}
		}

		return ret;
	}


	void* get_baseCamera_orig;
	void* get_baseCamera_hook(void* _this) {
		// printf("get_baseCamera\n");
		auto ret = reinterpret_cast<decltype(get_baseCamera_hook)*>(get_baseCamera_orig)(_this);  // UnityEngine.Camera

		// always save the camera as users may enable the gui option after the init
		static auto GetComponent = reinterpret_cast<void* (*)(void*, Il2CppReflectionType*)>(
			il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine",
				"Component", "GetComponent", 1));

		static auto transClass = il2cpp_symbols::get_class("UnityEngine.CoreModule.dll", "UnityEngine", "Transform");
		static auto transType = il2cpp_type_get_object(il2cpp_class_get_type(transClass));

		auto transform = GetComponent(ret, transType);  // UnityEngine.Transform
		if (transform) {
			baseCameraTransform = transform;
			baseCamera = ret;
		}

		return ret;
	}

	void* CameraWorkEvent_ApplyCamera_orig;
	void CameraWorkEvent_ApplyCamera_hook(void* _this) {
		static auto klass_CameraWorkEvent = il2cpp_symbols_logged::get_class("PRISM.Legacy", "", "CameraWorkEvent");
		static auto field_CameraWorkEvent_camera = il2cpp_class_get_field_from_name(klass_CameraWorkEvent, "_cameraCache");
		auto camera = il2cpp_field_get_value_object(field_CameraWorkEvent_camera, _this);

		reinterpret_cast<decltype(CameraWorkEvent_ApplyCamera_hook)*>(CameraWorkEvent_ApplyCamera_orig)(_this);

		if (g_enable_free_camera && baseCameraTransform != nullptr && camera == baseCamera) {
			Unity_get_position_hook(baseCameraTransform);
			Unity_get_rotation_hook(baseCameraTransform);
			Vector3_t worldPosition{}, worldUp{};
			worldUp.y = 1;
			Unity_InternalLookAt_hook(baseCameraTransform, worldPosition, worldUp);
		}
	}

	void readDMMGameGuardData();

	void* GGIregualDetector_ShowPopup_orig;
	void GGIregualDetector_ShowPopup_hook(void* _this, int code) {
		static auto End = reinterpret_cast<void (*)(void*)>(
			il2cpp_symbols::get_method_pointer(
				"PRISM.Legacy.dll", "PRISM",
				"GGIregualDetector", "End", 0
			));

		//printf("GGIregualDetector_ShowPopup: code: %d\n%ls\n\n", code, environment_get_stacktrace()->start_char);
		//readDMMGameGuardData();

		return End(_this);

		static auto this_klass = il2cpp_symbols::get_class_from_instance(_this);
		static FieldInfo* isShowPopup_field = il2cpp_class_get_field_from_name(this_klass, "_isShowPopup");
		static FieldInfo* isError_field = il2cpp_class_get_field_from_name(this_klass, "_isError");
		auto fieldData = il2cpp_class_get_static_field_data(this_klass);
		auto isError = il2cpp_symbols::read_field<bool>(_this, isError_field);

		if (isError) {
			// if (code == 0) return;
		}

		return reinterpret_cast<decltype(GGIregualDetector_ShowPopup_hook)*>(GGIregualDetector_ShowPopup_orig)(_this, code);
	}

	void* DMMGameGuard_NPGameMonCallback_orig;
	int DMMGameGuard_NPGameMonCallback_hook(UINT dwMsg, UINT dwArg) {
		// printf("DMMGameGuard_NPGameMonCallback: dwMsg: %u, dwArg: %u\n%ls\n\n", dwMsg, dwArg, environment_get_stacktrace()->start_char);
		return 1;
		// return reinterpret_cast<decltype(DMMGameGuard_NPGameMonCallback_hook)*>(DMMGameGuard_NPGameMonCallback_orig)(dwMsg, dwArg);
	}

	void* set_vsync_count_orig;
	void set_vsync_count_hook(int value) {
		return reinterpret_cast<decltype(set_vsync_count_hook)*>(set_vsync_count_orig)(g_vsync_count == -1 ? value : g_vsync_count);
	}

	void* set_fps_orig;
	void set_fps_hook(int value) {
		return reinterpret_cast<decltype(set_fps_hook)*>(set_fps_orig)(g_max_fps == -1 ? value : g_max_fps);
	}

	void* Unity_Quit_orig;
	void Unity_Quit_hook(int code) {
		printf("Quit code: %d\n", code);
		TerminateProcess(GetCurrentProcess(), 0);
		// printf("Quit code: %d\n%ls\n\n", code, environment_get_stacktrace()->start_char);
		// return reinterpret_cast<decltype(Unity_Quit_hook)*>(Unity_Quit_orig)(code);
	}

	void* SetCallbackToGameMon_orig;
	int SetCallbackToGameMon_hook(void* DMMGameGuard_NPGameMonCallback_addr) {
		// printf("SetCallbackToGameMon: %p\n%ls\n\n", DMMGameGuard_NPGameMonCallback_addr, environment_get_stacktrace()->start_char);
		ADD_HOOK(DMMGameGuard_NPGameMonCallback, "DMMGameGuard_NPGameMonCallback at %p");
		return reinterpret_cast<decltype(SetCallbackToGameMon_hook)*>(SetCallbackToGameMon_orig)(DMMGameGuard_NPGameMonCallback_addr);
	}

	void* DMMGameGuard_Setup_orig;
	void DMMGameGuard_Setup_hook() {
		// printf("\n\nDMMGameGuard_Setup\n\n");
		readDMMGameGuardData();
	}

	void* DMMGameGuard_SetCheckMode_orig;
	void DMMGameGuard_SetCheckMode_hook(bool isCheck) {
		// printf("DMMGameGuard_SetCheckMode before\n");
		// readDMMGameGuardData();
		reinterpret_cast<decltype(DMMGameGuard_SetCheckMode_hook)*>(DMMGameGuard_SetCheckMode_orig)(isCheck);
		// printf("DMMGameGuard_SetCheckMode after\n");
		// readDMMGameGuardData();
	}

	void* PreInitNPGameMonW_orig;
	void PreInitNPGameMonW_hook(int64_t v) {
		// printf("PreInitNPGameMonW\n");
		// readDMMGameGuardData();
	}
	void* PreInitNPGameMonA_orig;
	void PreInitNPGameMonA_hook(int64_t v) {
		// printf("PreInitNPGameMonA\n");
		// readDMMGameGuardData();
	}

	void* InitNPGameMon_orig;
	UINT InitNPGameMon_hook() {
		// printf("InitNPGameMon\n");
		// readDMMGameGuardData();
		return 1877;
	}

	void* SetHwndToGameMon_orig;
	void SetHwndToGameMon_hook(int64_t v) {
		// printf("SetHwndToGameMon\n");
		// readDMMGameGuardData();
	}

	void* DMMGameGuard_klass;
	FieldInfo* isCheck_field;
	FieldInfo* isInit_field;
	FieldInfo* bAppExit_field;
	FieldInfo* errCode_field;

	void readDMMGameGuardData() {
		if (!DMMGameGuard_klass) return;
		if (!isCheck_field) {
			printf("isCheck_field invalid: %p\n", isCheck_field);
			return;
		}

		auto staticData = il2cpp_class_get_static_field_data(DMMGameGuard_klass);
		auto isCheck = il2cpp_symbols::read_field<bool>(staticData, isCheck_field);
		auto isInit = il2cpp_symbols::read_field<bool>(staticData, isInit_field);
		auto bAppExit = il2cpp_symbols::read_field<bool>(staticData, bAppExit_field);
		auto errCode = il2cpp_symbols::read_field<int>(staticData, errCode_field);
		if (bAppExit) {
			il2cpp_symbols::write_field(staticData, bAppExit_field, false);
		}

		printf("DMMGameGuardData: isCheck: %d, isInit: %d, bAppExit: %d, errCode: %d\n\n", isCheck, isInit, bAppExit, errCode);
	}

	bool pathed = false;
	bool npPatched = false;

	void patchNP(HMODULE module) {
		if (npPatched) return;
		npPatched = true;
		// auto np_module = GetModuleHandle("NPGameDLL");
		if (module) {
			auto SetCallbackToGameMon_addr = GetProcAddress(module, "SetCallbackToGameMon");
			auto PreInitNPGameMonW_addr = GetProcAddress(module, "PreInitNPGameMonW");
			auto PreInitNPGameMonA_addr = GetProcAddress(module, "PreInitNPGameMonA");
			auto InitNPGameMon_addr = GetProcAddress(module, "InitNPGameMon");
			auto SetHwndToGameMon_addr = GetProcAddress(module, "SetHwndToGameMon");

			ADD_HOOK(SetCallbackToGameMon, "SetCallbackToGameMon ap %p");
			ADD_HOOK(PreInitNPGameMonW, "PreInitNPGameMonW ap %p");
			ADD_HOOK(PreInitNPGameMonA, "PreInitNPGameMonA ap %p");
			ADD_HOOK(InitNPGameMon, "InitNPGameMon ap %p");
			ADD_HOOK(SetHwndToGameMon, "SetHwndToGameMon ap %p");
		}
		else {
			printf("NPGameDLL Get failed.\n");
		}
	}

	int retryCount = 0;
	void path_game_assembly()
	{
		if (!mh_inited)
			return;

		auto il2cpp_module = GetModuleHandle("GameAssembly.dll");
		if (!il2cpp_module) {
			printf("GameAssembly.dll not loaded.\n");
			retryCount++;
			if (retryCount == 15) {
				reopen_self();
				return;
			}
		}

		if (pathed) return;
		pathed = true;

		printf("Trying to patch GameAssembly.dll...\n");
		initcharaSwayStringOffset();

		// load il2cpp exported functions
		il2cpp_symbols::init(il2cpp_module);

#pragma region HOOK_ADDRESSES

		environment_get_stacktrace = reinterpret_cast<decltype(environment_get_stacktrace)>(
			il2cpp_symbols::get_method_pointer("mscorlib.dll", "System",
				"Environment", "get_StackTrace", 0)
			);

		DMMGameGuard_klass = il2cpp_symbols::get_class("PRISM.Legacy.dll", "PRISM", "DMMGameGuard");
		isCheck_field = il2cpp_class_get_field_from_name(DMMGameGuard_klass, "_isCheck");
		isInit_field = il2cpp_class_get_field_from_name(DMMGameGuard_klass, "_isInit");
		bAppExit_field = il2cpp_class_get_field_from_name(DMMGameGuard_klass, "_bAppExit");
		errCode_field = il2cpp_class_get_field_from_name(DMMGameGuard_klass, "_errCode");
		initEncoding();
		ReadAllBytes = reinterpret_cast<decltype(ReadAllBytes)>(
			il2cpp_symbols::get_method_pointer("mscorlib.dll", "System.IO", "File", "ReadAllBytes", 1)
			);

		AssetBundle_LoadFromFile = reinterpret_cast<decltype(AssetBundle_LoadFromFile)>(
			il2cpp_symbols::get_method_pointer(
				"UnityEngine.AssetBundleModule.dll", "UnityEngine",
				"AssetBundle", "LoadFromFile", 3
			)
			);
		Object_IsNativeObjectAlive = reinterpret_cast<bool(*)(void*)>(
			il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine",
				"Object", "IsNativeObjectAlive", 1)
			);
		AssetBundle_LoadAsset = reinterpret_cast<decltype(AssetBundle_LoadAsset)>(
			il2cpp_symbols_logged::get_method_pointer("UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundle", "LoadAsset_Internal", 2)
			);
		const auto FontClass = il2cpp_symbols::get_class("UnityEngine.TextRenderingModule.dll", "UnityEngine", "Font");
		Font_Type = il2cpp_type_get_object(il2cpp_class_get_type(FontClass));
		TMP_FontAsset_CreateFontAsset = reinterpret_cast<decltype(TMP_FontAsset_CreateFontAsset)>(il2cpp_symbols::get_method_pointer(
			"Unity.TextMeshPro.dll", "TMPro",
			"TMP_FontAsset", "CreateFontAsset", 1
		));
		TMP_Text_set_font = reinterpret_cast<decltype(TMP_Text_set_font)>(il2cpp_symbols::get_method_pointer(
			"Unity.TextMeshPro.dll", "TMPro",
			"TMP_Text", "set_font", 1
		));
		TMP_Text_get_font = reinterpret_cast<decltype(TMP_Text_get_font)>(il2cpp_symbols::get_method_pointer(
			"Unity.TextMeshPro.dll", "TMPro",
			"TMP_Text", "get_font", 0
		));

		const auto TMP_Text_set_text_addr = il2cpp_symbols::get_method_pointer(
			"Unity.TextMeshPro.dll", "TMPro",
			"TMP_Text", "set_text", 1
		);
		const auto UITextMeshProUGUI_Awake_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "ENTERPRISE.UI",
			"UITextMeshProUGUI", "Awake", 0
		);

		auto ScenarioManager_Init_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM.Scenario",
			"ScenarioManager", "_initializeAsync", 1
		);
		auto DataFile_GetBytes_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"DataFile", "GetBytes", 1
		);
		DataFile_IsKeyExist = reinterpret_cast<decltype(DataFile_IsKeyExist)>(
			il2cpp_symbols::get_method_pointer(
				"PRISM.Legacy.dll", "PRISM",
				"DataFile", "IsKeyExist", 1
			)
			);

		auto SetResolution_addr = il2cpp_symbols::get_method_pointer(
			"UnityEngine.CoreModule.dll", "UnityEngine",
			"Screen", "SetResolution", 3
		);

		//auto UnsafeLoadBytesFromKey_addr = il2cpp_symbols::get_method_pointer(
		//	"PRISM.ResourceManagement.dll", "PRISM.ResourceManagement",
		//	"ResourceLoader", "UnsafeLoadBytesFromKey", 2
		//);

		auto TextLog_AddLog_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM.Scenario",
			"TextLog", "AddLog", 4
		);

		//auto PIdolDetailPopupViewModel_Create_addr = il2cpp_symbols::get_method_pointer(
		//	"PRISM.Adapters.dll", "PRISM.Adapters",
		//	"PIdolDetailPopupViewModel", "Create", 10
		//);
		auto ScenarioContentViewModel_ctor_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Adapters.dll", "PRISM.Adapters",
			"ScenarioContentViewModel", ".ctor", 8
		);

		auto LocalizationManager_GetTextOrNull_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "ENTERPRISE.Localization",
			"LocalizationManager", "GetTextOrNull", 2
		);
		auto GetResolutionSize_addr = il2cpp_symbols::get_method_pointer(
			"Prism.Rendering.Runtime.dll", "PRISM.Rendering",
			"RenderManager", "GetResolutionSize", 1
		);
		auto LiveMVOverlayView_UpdateLyrics_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Interactions.Live.dll", "PRISM.Interactions.Live",
			"LiveMVOverlayView", "UpdateLyrics", 1
		);
		auto TimelineController_SetLyric_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"TimelineController", "SetLyric", 1
		);

		auto get_baseCamera_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"CameraController", "get_BaseCamera", 0
		);

		auto CameraWorkEvent_ApplyCamera_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy", "",
			"CameraWorkEvent", "ApplyCamera", 0
		);

		const auto AssetBundle_LoadAsset_addr = il2cpp_symbols_logged::get_method_pointer("UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundle", "LoadAsset_Internal", 2);

		auto Unity_get_position_addr = il2cpp_symbols_logged::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine", "Transform", "get_position", 0);
		auto Unity_set_position_addr = il2cpp_symbols_logged::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine", "Transform", "set_position", 1);

		auto Unity_get_fieldOfView_addr = il2cpp_symbols_logged::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine", "Camera", "get_fieldOfView", 0);
		auto Unity_set_fieldOfView_addr = il2cpp_symbols_logged::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine", "Camera", "set_fieldOfView", 1);
		auto Unity_InternalLookAt_addr = il2cpp_symbols_logged::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine", "Transform", "Internal_LookAt", 2);
		auto Unity_set_nearClipPlane_addr = il2cpp_symbols_logged::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine", "Camera", "set_nearClipPlane", 1);
		auto Unity_get_nearClipPlane_addr = il2cpp_symbols_logged::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine", "Camera", "get_nearClipPlane", 0);
		auto Unity_get_farClipPlane_addr = il2cpp_symbols_logged::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine", "Camera", "get_farClipPlane", 0);
		auto Unity_set_farClipPlane_addr = il2cpp_symbols_logged::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine", "Camera", "set_farClipPlane", 1);
		auto Unity_get_rotation_addr = il2cpp_symbols_logged::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine", "Transform", "get_rotation", 0);
		auto Unity_set_rotation_addr = il2cpp_symbols_logged::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine", "Transform", "set_rotation", 1);

		auto InvokeMoveNext_addr = il2cpp_symbols::get_method_pointer(
			"UnityEngine.CoreModule.dll", "UnityEngine",
			"SetupCoroutine", "InvokeMoveNext", 2
		);
		/*
		auto Live_SetEnableDepthOfField_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"LiveScene", "SetEnableDepthOfField", 1
		);*/

		auto DepthOfFieldClip_CreatePlayable_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"DepthOfFieldClip", "CreatePlayable", 2
		);

		auto PostProcess_DepthOfFieldClip_CreatePlayable_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "UnityEngine.Rendering.Universal.PostProcess",
			"DepthOfFieldClip", "CreatePlayable", 2
		);

		auto Live_Update_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"LiveScene", "Update", 0
		);
		auto LiveCostumeChangeView_setTryOnMode_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Interactions.Live.dll", "PRISM.Interactions",
			"LiveCostumeChangeView", "_setTryOnMode", 2
		);
		auto LiveCostumeChangeView_setIdolCostume_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Interactions.Live.dll", "PRISM.Interactions",
			"LiveCostumeChangeView", "_setIdolCostume", 3
		);

		LiveCostumeChangeModel_get_Idol = reinterpret_cast<decltype(LiveCostumeChangeModel_get_Idol)>(il2cpp_symbols::get_method_pointer(
			"PRISM.Adapters.dll", "PRISM.Adapters",
			"LiveCostumeChangeModel", "get_Idol", 0
		));

		auto LiveCostumeChangeModel_GetDress_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Adapters.dll", "PRISM.Adapters",
			"LiveCostumeChangeModel", "GetDress", 1
		);
		auto LiveCostumeChangeModel_GetHairstyle_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Adapters.dll", "PRISM.Adapters",
			"LiveCostumeChangeModel", "GetHairstyle", 1
		);
		auto LiveCostumeChangeModel_GetAccessory_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Adapters.dll", "PRISM.Adapters",
			"LiveCostumeChangeModel", "GetAccessory", 1
		);
		auto LiveCostumeChangeModel_ctor_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Adapters.dll", "PRISM.Adapters",
			"LiveCostumeChangeModel", ".ctor", 4
		);

		auto AssembleCharacter_ApplyParam_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"AssembleCharacter", "ApplyParam", 6
		);
		/*
		auto AssembleCharacter_ApplyParam_addr = il2cpp_symbols::find_method("PRISM.Legacy.dll", "PRISM", "AssembleCharacter", [=](const MethodInfo* mtd) {
			const std::string mtdName = mtd->name;

			if ((mtdName == "ApplyParam") && (mtd->parameters_count == 6)) {
				if (mtd->methodPointer != AssembleCharacter_ApplyParam_mdl_addr) {
					return true;
				}
			}

			return false;
			});*/

		auto MainThreadDispatcher_LateUpdate_addr = il2cpp_symbols::get_method_pointer(
			"UniRx.dll", "UniRx",
			"MainThreadDispatcher", "LateUpdate", 0
		);

		auto LiveMVUnit_GetMemberChangeRequestData_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM.Live",
			"LiveMVUnit", "GetMemberChangeRequestData", 3
		);

		auto LiveMVUnitMemberChangePresenter_klass = il2cpp_symbols::get_class("PRISM.Adapters.dll", "PRISM.Adapters", "LiveMvUnitMemberChangePresenter");
		auto LiveMVUnitMemberChangePresenter_c__DisplayClass7_0_klass
			= il2cpp_symbols::find_nested_class_from_name(LiveMVUnitMemberChangePresenter_klass, "<>c__DisplayClass5_0");
		auto LiveMVUnitMemberChangePresenter_c__DisplayClass7_0_klass_initializeAsync_b__4_d_klass
			= il2cpp_symbols::find_nested_class_from_name(LiveMVUnitMemberChangePresenter_c__DisplayClass7_0_klass, "<<InitializeAsync>b__0>d");
		auto LiveMVUnitMemberChangePresenter_initializeAsync_b_4_MoveNext_method =
			il2cpp_class_get_method_from_name(LiveMVUnitMemberChangePresenter_c__DisplayClass7_0_klass_initializeAsync_b__4_d_klass, "MoveNext", 0);
		uintptr_t LiveMVUnitMemberChangePresenter_initializeAsync_b_4_MoveNext_addr = NULL;

		if (LiveMVUnitMemberChangePresenter_initializeAsync_b_4_MoveNext_method) {
			LiveMVUnitMemberChangePresenter_initializeAsync_b_4_MoveNext_addr = LiveMVUnitMemberChangePresenter_initializeAsync_b_4_MoveNext_method->methodPointer;
		}
		else {
			printf("LiveMVUnitMemberChangePresenter_initializeAsync_b_4_MoveNext_method not found.\n");
		}

		auto MvUnitSlotGenerator_NewMvUnitSlot_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"MvUnitSlotGenerator", "NewMvUnitSlot", 2
		);

		auto CheckVocalSeparatedSatisfy_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM.Live",
			"MusicData", "CheckVocalSeparatedSatisfy", 1
		);
		auto CheckLimitedVocalSeparatedSatisfy_2_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM.Live",
			"MusicData", "CheckLimitedVocalSeparatedSatisfy", 2
		);

		auto CriWareErrorHandler_HandleMessage_addr = il2cpp_symbols::get_method_pointer(
			"CriMw.CriWare.Runtime.dll", "CriWare",
			"CriWareErrorHandler", "HandleMessage", 1
		);

		auto GGIregualDetector_ShowPopup_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"GGIregualDetector", "ShowPopup", 1
		);
		auto DMMGameGuard_NPGameMonCallback_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"DMMGameGuard", "NPGameMonCallback", 2
		);
		auto DMMGameGuard_Setup_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"DMMGameGuard", "Setup", 0
		);
		auto DMMGameGuard_SetCheckMode_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"DMMGameGuard", "SetCheckMode", 1
		);
		/*
		CloseNPGameMon = reinterpret_cast<decltype(CloseNPGameMon)>(il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"DMMGameGuard", "CloseNPGameMon", 0
		));*/

		auto set_fps_addr = il2cpp_symbols_logged::il2cpp_resolve_icall("UnityEngine.Application::set_targetFrameRate(System.Int32)");
		auto set_vsync_count_addr = il2cpp_symbols_logged::il2cpp_resolve_icall("UnityEngine.QualitySettings::set_vSyncCount(System.Int32)");
		auto Unity_Quit_addr = il2cpp_symbols_logged::il2cpp_resolve_icall("UnityEngine.Application::Quit(System.Int32)");


		auto assemblyLoad = reinterpret_cast<void* (*)(Il2CppString*)>(
			il2cpp_symbols::get_method_pointer("mscorlib.dll", "System.Reflection",
				"Assembly", "Load", 1)
			);
		auto assemblyGetType = reinterpret_cast<Il2CppReflectionType * (*)(void*, Il2CppString*)>(
			il2cpp_symbols::get_method_pointer("mscorlib.dll", "System.Reflection",
				"Assembly", "GetType", 1)
			);
		auto reflectionAssembly = assemblyLoad(il2cpp_string_new("mscorlib"));
		auto reflectionType = assemblyGetType(
			reflectionAssembly, il2cpp_string_new("System.Collections.Generic.Dictionary`2[System.Int32, PRISM.Module.Networking.ICostumeStatus]"));
		auto dic_klass = il2cpp_class_from_system_type(reflectionType);
		auto dic_int_ICostumeStatus_add_addr = il2cpp_class_get_method_from_name(dic_klass, "Add", 2)->methodPointer;

		auto GetCostumeListReply_get_CostumeList_addr = il2cpp_symbols::get_method_pointer("PRISM.Module.Networking.dll",
			"PRISM.Module.Networking.Stub.Api", "GetCostumeListReply", "get_CostumeList", 0);
		auto GetCostumeListReply_get_HairstyleList_addr = il2cpp_symbols::get_method_pointer("PRISM.Module.Networking.dll",
			"PRISM.Module.Networking.Stub.Api", "GetCostumeListReply", "get_HairstyleList", 0);
		auto GetCostumeListReply_get_AccessoryList_addr = il2cpp_symbols::get_method_pointer("PRISM.Module.Networking.dll",
			"PRISM.Module.Networking.Stub.Api", "GetCostumeListReply", "get_AccessoryList", 0);

		auto LiveMVUnitConfirmationModel_ctor_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Service.dll", "PRISM.Service.Live",
			"LiveMvUnitConfirmationModel", ".ctor", 4
		);

		auto SwayString_SetupPoint_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy.dll", "PRISM",
			"SwayString", "SetupPoint", 0
		);

		auto CostumeChangeViewModel_ctor_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Adapters", "PRISM.Adapters.CostumeChange",
			"CostumeChangeViewModel", ".ctor", 3
		);

		// [Muitsonz/#1](https://github.com/Muitsonz/scsp-localify/issues/1)
		auto CostumeChangeView_Reload_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Interactions", "PRISM.Interactions.CostumeChange",
			"CostumeChangeView", "Reload", 1
		);
		auto LiveMVStartData_ctor_addr = il2cpp_symbols::get_method_pointer(
			"PRISM.Legacy", "PRISM.Live",
			"LiveMVStartData", ".ctor", 7
		);

#pragma endregion
		ADD_HOOK(SetResolution, "SetResolution at %p");
		// ADD_HOOK(PIdolDetailPopupViewModel_Create, "PIdolDetailPopupViewModel_Create at %p");
		ADD_HOOK(ScenarioContentViewModel_ctor, "ScenarioContentViewModel_ctor at %p");
		ADD_HOOK(LocalizationManager_GetTextOrNull, "LocalizationManager_GetTextOrNull at %p");
		ADD_HOOK(GetResolutionSize, "GetResolutionSize at %p");
		ADD_HOOK(AssetBundle_LoadAsset, "AssetBundle_LoadAsset at %p");
		ADD_HOOK(LiveMVOverlayView_UpdateLyrics, "LiveMVOverlayView_UpdateLyrics at %p");
		ADD_HOOK(TimelineController_SetLyric, "TimelineController_SetLyric at %p");
		ADD_HOOK(get_baseCamera, "get_baseCamera at %p");
		ADD_HOOK(CameraWorkEvent_ApplyCamera, "CameraWorkEvent_ApplyCamera at %p");
		ADD_HOOK(Unity_get_position, "Unity_get_position at %p");
		ADD_HOOK(Unity_set_position, "Unity_set_position at %p");
		ADD_HOOK(Unity_get_fieldOfView, "Unity_get_fieldOfView at %p");
		ADD_HOOK(Unity_set_fieldOfView, "Unity_set_fieldOfView at %p");
		ADD_HOOK(Unity_InternalLookAt, "Unity_InternalLookAt at %p");
		ADD_HOOK(Unity_set_nearClipPlane, "Unity_set_nearClipPlane at %p");
		ADD_HOOK(Unity_get_nearClipPlane, "Unity_get_nearClipPlane at %p");
		ADD_HOOK(Unity_get_farClipPlane, "Unity_get_farClipPlane at %p");
		ADD_HOOK(Unity_set_farClipPlane, "Unity_set_farClipPlane at %p");
		ADD_HOOK(Unity_get_rotation, "Unity_get_rotation at %p");
		ADD_HOOK(Unity_set_rotation, "Unity_set_rotation at %p");

		ADD_HOOK(TMP_Text_set_text, "TMP_Text_set_text at %p");
		ADD_HOOK(UITextMeshProUGUI_Awake, "UITextMeshProUGUI_Awake at %p");
		ADD_HOOK(ScenarioManager_Init, "ScenarioManager_Init at %p");
		ADD_HOOK(DataFile_GetBytes, "DataFile_GetBytes at %p");
		//ADD_HOOK(UnsafeLoadBytesFromKey, "UnsafeLoadBytesFromKey at %p");
		ADD_HOOK(TextLog_AddLog, "TextLog_AddLog at %p");
		ADD_HOOK(InvokeMoveNext, "InvokeMoveNext at %p");
		// ADD_HOOK(Live_SetEnableDepthOfField, "Live_SetEnableDepthOfField at %p");
		ADD_HOOK(DepthOfFieldClip_CreatePlayable, "DepthOfFieldClip_CreatePlayable at %p");
		ADD_HOOK(PostProcess_DepthOfFieldClip_CreatePlayable, "PostProcess_DepthOfFieldClip_CreatePlayable at %p");
		// ADD_HOOK(Live_Update, "Live_Update at %p");
		ADD_HOOK(LiveCostumeChangeView_setTryOnMode, "LiveCostumeChangeView_setTryOnMode at %p");
		ADD_HOOK(LiveCostumeChangeView_setIdolCostume, "LiveCostumeChangeView_setIdolCostume at %p");
		ADD_HOOK(LiveCostumeChangeModel_GetDress, "LiveCostumeChangeModel_GetDress at %p");
		ADD_HOOK(LiveCostumeChangeModel_GetHairstyle, "LiveCostumeChangeModel_GetHairstyle at %p");
		ADD_HOOK(LiveCostumeChangeModel_GetAccessory, "LiveCostumeChangeModel_GetAccessory at %p");
		ADD_HOOK(LiveCostumeChangeModel_ctor, "LiveCostumeChangeModel_ctor at %p");
		ADD_HOOK(AssembleCharacter_ApplyParam, "AssembleCharacter_ApplyParam at %p");
		ADD_HOOK(MainThreadDispatcher_LateUpdate, "MainThreadDispatcher_LateUpdate at %p");
		ADD_HOOK(dic_int_ICostumeStatus_add, "dic_int_ICostumeStatus_add at %p");
		ADD_HOOK(GetCostumeListReply_get_CostumeList, "GetCostumeListReply_get_CostumeList at %p");
		ADD_HOOK(GetCostumeListReply_get_HairstyleList, "GetCostumeListReply_get_HairstyleList at %p");
		ADD_HOOK(GetCostumeListReply_get_AccessoryList, "GetCostumeListReply_get_AccessoryList at %p");
		ADD_HOOK(LiveMVUnitConfirmationModel_ctor, "LiveMVUnitConfirmationModel_ctor at %p");
		ADD_HOOK(SwayString_SetupPoint, "SwayString_SetupPoint at %p");
		ADD_HOOK(LiveMVUnit_GetMemberChangeRequestData, "LiveMVUnit_GetMemberChangeRequestData at %p");
		ADD_HOOK(LiveMVUnitMemberChangePresenter_initializeAsync_b_4_MoveNext, "LiveMVUnitMemberChangePresenter_initializeAsync_b_4_MoveNext at %p");
		ADD_HOOK(MvUnitSlotGenerator_NewMvUnitSlot, "MvUnitSlotGenerator_NewMvUnitSlot at %p");
		ADD_HOOK(CheckVocalSeparatedSatisfy, "CheckVocalSeparatedSatisfy at %p");
		ADD_HOOK(CheckLimitedVocalSeparatedSatisfy_2, "CheckLimitedVocalSeparatedSatisfy_2 at %p");
		ADD_HOOK(CriWareErrorHandler_HandleMessage, "CriWareErrorHandler_HandleMessage at %p");
		ADD_HOOK(GGIregualDetector_ShowPopup, "GGIregualDetector_ShowPopup at %p");
		ADD_HOOK(DMMGameGuard_NPGameMonCallback, "DMMGameGuard_NPGameMonCallback at %p");
		// ADD_HOOK(DMMGameGuard_Setup, "DMMGameGuard_Setup at %p");
		ADD_HOOK(DMMGameGuard_SetCheckMode, "DMMGameGuard_SetCheckMode at %p");
		ADD_HOOK(set_fps, "set_fps at %p");
		ADD_HOOK(set_vsync_count, "set_vsync_count at %p");
		ADD_HOOK(Unity_Quit, "Unity_Quit at %p");

		ADD_HOOK(CostumeChangeViewModel_ctor, "CostumeChangeViewModel_ctor at %p");
		// [Muitsonz/#1](https://github.com/Muitsonz/scsp-localify/issues/1)
		ADD_HOOK(CostumeChangeView_Reload, "CostumeChangeView_Reload at %p");
		ADD_HOOK(LiveMVStartData_ctor, "LiveMVStartData_ctor at %p");

		tools::AddNetworkingHooks();

		on_hotKey_0 = []() {
			startSCGUI();
			// needPrintStack = !needPrintStack;
			};
		SCCamera::initCameraSettings();
		g_on_hook_ready();

		const auto gameVersionInfo = getGameVersions();
		wprintf(L"Plugin Loaded - Game Version: %ls, Resource Version: %ls\n", gameVersionInfo.gameVersion.c_str(), gameVersionInfo.resourceVersion.c_str());
	}
}

void uninit_hook()
{
	if (!mh_inited)
		return;

	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
}

bool init_hook()
{
	if (mh_inited)
		return false;

	if (MH_Initialize() != MH_OK)
		return false;

	g_on_close = []() {
		uninit_hook();
		TerminateProcess(GetCurrentProcess(), 0);
		};

	mh_inited = true;

	MH_CreateHook(LoadLibraryW, load_library_w_hook, &load_library_w_orig);
	MH_EnableHook(LoadLibraryW);
	return true;
}
