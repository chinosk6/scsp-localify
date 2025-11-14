#include <stdinclude.hpp>

#include <minizip/unzip.h>
#include <TlHelp32.h>

#include <unordered_set>
#include <charconv>
#include <cassert>
#include <format>
#include <cpprest/uri.h>
#include <cpprest/http_listener.h>
#include <ranges>
#include <mhotkey.hpp>

extern bool init_hook();
extern void uninit_hook();
extern void start_console();

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;


bool g_enable_plugin = true;
bool g_enable_console = true;
bool g_auto_dump_all_json = false;
bool g_dump_untrans_lyrics = false;
bool g_dump_untrans_unlocal = false;
int g_max_fps = 60;
int g_vsync_count = 0;
float g_3d_resolution_scale = 1.0f;
std::string g_custom_font_path = "";
char hotKey = 'u';
float g_font_size_offset = -3.0f;

bool g_enable_free_camera = false;
bool g_block_out_of_focus = false;
float g_free_camera_mouse_speed = 35;
bool g_allow_use_tryon_costume = false;
bool g_allow_same_idol = false;
bool g_unlock_all_dress = false;
bool g_unlock_all_headwear = false;
bool g_show_hidden_costumes = false;
bool g_save_and_replace_costume_changes = false;
bool g_overrie_mv_unit_idols = false;
bool g_override_isVocalSeparatedOn = false;
bool g_enable_chara_param_edit = false;
bool g_unlock_PIdol_and_SChara_events = false;
int g_start_resolution_w = -1;
int g_start_resolution_h = -1;
bool g_start_resolution_fullScreen = false;
bool g_reenable_clipPlane = false;
bool g_dev_loadasset_output = false;
bool g_dev_loadasset_extract = false;
bool g_dev_shader_quickprobing = true;

std::filesystem::path g_localify_base("scsp_localify");
constexpr const char ConfigJson[] = "scsp-config.json";

const auto CONSOLE_TITLE = L"iM@S SCSP Tools By chinosk";

namespace
{
	void create_debug_console()
	{
		AllocConsole();

		// open stdout stream
		auto _ = freopen("CONOUT$", "w+t", stdout);
		_ = freopen("CONOUT$", "w", stderr);
		_ = freopen("CONIN$", "r", stdin);

		SetConsoleTitleW(CONSOLE_TITLE);

		// set this to avoid turn japanese texts into question mark
		SetConsoleOutputCP(65001);
		std::locale::global(std::locale(""));

		wprintf(L"THEiDOLM@STER ShinyColors Song for Prism tools loaded! - By chinosk\n");
	}
}



namespace
{
	std::vector<std::string> read_config(std::vector<std::string>& logs)
	{
		std::ifstream config_stream{ ConfigJson };
		std::vector<std::string> dicts{};

		if (!config_stream.is_open())
			return dicts;

		rapidjson::IStreamWrapper wrapper{ config_stream };
		rapidjson::Document document;

		document.ParseStream(wrapper);

		if (!document.HasParseError())
		{
			if (document.HasMember("enableVSync")) {
				if (document["enableVSync"].GetBool()) {
					g_vsync_count = 1;
				}
				else {
					g_vsync_count = 0;
				}
			}
			if (document.HasMember("vSyncCount")) {
				g_vsync_count = document["vSyncCount"].GetInt();
			}
			if (document.HasMember("maxFps")) {
				g_max_fps = document["maxFps"].GetInt();
			}
			if (document.HasMember("3DResolutionScale")) {
				g_3d_resolution_scale = document["3DResolutionScale"].GetFloat();
			}

			if (document.HasMember("enableConsole")) {
				g_enable_console = document["enableConsole"].GetBool();
			}
			if (document.HasMember("localifyBasePath")) {
				g_localify_base = document["localifyBasePath"].GetString();
			}
			if (document.HasMember("hotKey")) {
				hotKey = document["hotKey"].GetString()[0];
			}
			if (document.HasMember("autoDumpAllJson")) {
				g_auto_dump_all_json = document["autoDumpAllJson"].GetBool();
			}
			if (document.HasMember("dumpUntransLyrics")) {
				g_dump_untrans_lyrics = document["dumpUntransLyrics"].GetBool();
			}
			if (document.HasMember("dumpUntransLocal2")) {
				g_dump_untrans_unlocal = document["dumpUntransLocal2"].GetBool();
			}
			if (document.HasMember("extraAssetBundlePath")) {
				logs.push_back("[WARNING] Option `extraAssetBundlePath` is obsolete. Use `asset_bundle_path::asset_path` to specify an asset.\n");
			}
			if (document.HasMember("extraAssetBundlePaths")) {
				logs.push_back("[WARNING] Option `extraAssetBundlePaths` is obsolete. Use `asset_bundle_path::asset_path` to specify an asset.\n");
			}
			if (document.HasMember("customFontPath")) {
				g_custom_font_path = document["customFontPath"].GetString();
				if (g_custom_font_path.find("::") == std::string::npos) {
					logs.push_back("[WARNING] Option `customFontPath` is set by old style; the font is assumed to be inside the default bundle. Use `asset_bundle_path::asset_path` to specify an asset.\n");
					g_custom_font_path = "scsp_localify/scsp-bundle::" + g_custom_font_path;
				}
			}
			if (document.HasMember("fontSizeOffset")) {
				g_font_size_offset = document["fontSizeOffset"].GetFloat();
			}

			if (document.HasMember("blockOutOfFocus")) {
				g_block_out_of_focus = document["blockOutOfFocus"].GetBool();
			}

			if (document.HasMember("baseFreeCamera")) {
				const auto& freeCamConfig = document["baseFreeCamera"];
				if (freeCamConfig.IsObject()) {
					if (freeCamConfig.HasMember("enable")) {
						g_enable_free_camera = freeCamConfig["enable"].GetBool();
					}
					if (freeCamConfig.HasMember("moveStep")) {
						BaseCamera::moveStep = freeCamConfig["moveStep"].GetFloat() / 1000;
					}
					if (freeCamConfig.HasMember("mouseSpeed")) {
						g_free_camera_mouse_speed = freeCamConfig["mouseSpeed"].GetFloat();
					}
				}
			}

			if (document.HasMember("allowUseTryOnCostume")) {
				g_allow_use_tryon_costume = document["allowUseTryOnCostume"].GetBool();
			}
			if (document.HasMember("allowSameIdol")) {
				g_allow_same_idol = document["allowSameIdol"].GetBool();
			}
			if (document.HasMember("saveAndReplaceCostumeChanges")) {
				g_save_and_replace_costume_changes = document["saveAndReplaceCostumeChanges"].GetBool();
			}
			if (document.HasMember("unlockPIdolAndSCharaEvents")) {
				g_unlock_PIdol_and_SChara_events = document["unlockPIdolAndSCharaEvents"].GetBool();
			}

			if (document.HasMember("startResolution")) {
				auto& startResolution = document["startResolution"];
				g_start_resolution_w = startResolution["w"].GetInt();
				g_start_resolution_h = startResolution["h"].GetInt();
				g_start_resolution_fullScreen = startResolution["isFull"].GetBool();
			}
			
		}

		config_stream.close();
		return dicts;
	}
}

void reloadTransData() {
	SCLocal::loadLocalTrans();
}

void reload_all_data() {
	std::vector<std::string> logs{};
	read_config(logs);
	for (const auto& log : logs) {
		printf("%s", log.c_str());
	}
	reloadTransData();
}

extern std::function<void()> g_on_hook_ready;
std::function<void()> g_reload_all_data = reload_all_data;

int __stdcall DllMain(HINSTANCE dllModule, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		// the DMM Launcher set start path to system32 wtf????
		std::string module_name;
		module_name.resize(MAX_PATH);
		module_name.resize(GetModuleFileName(nullptr, module_name.data(), MAX_PATH));

		std::filesystem::path module_path(module_name);

		// check name
		if (module_path.filename() != "imasscprism.exe")
			return 1;

		std::filesystem::current_path(
			module_path.parent_path()
		);


		std::vector<std::string> logs{};
		auto dicts = read_config(logs);

		if (g_enable_console) {
			create_debug_console();
			printf("Command: %s\n", GetCommandLineA());
		}

		for (const auto& log : logs) {
			printf("%s", log.c_str());
		}

		std::thread init_thread([dicts = std::move(dicts)] {

			if (g_enable_console)
			{
				start_console();
			}

			init_hook();

			std::mutex mutex;
			std::condition_variable cond;
			std::atomic<bool> hookIsReady(false);
			g_on_hook_ready = [&]
			{
				hookIsReady.store(true, std::memory_order_release);
				cond.notify_one();
			};

			// 依赖检查游戏版本的指针加载，因此在 hook 完成后再加载翻译数据
			std::unique_lock lock(mutex);
			cond.wait(lock, [&] {
				return hookIsReady.load(std::memory_order_acquire);
				});
			if (g_enable_console)
			{
				auto _ = freopen("CONOUT$", "w+t", stdout);
				_ = freopen("CONOUT$", "w", stderr);
				_ = freopen("CONIN$", "r", stdin);
			}

			reloadTransData();
			SetConsoleTitleW(CONSOLE_TITLE);  // 保持控制台标题
			});
		init_thread.detach();
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
		uninit_hook();
	}
	return 1;
}
