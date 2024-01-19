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

#include <nlohmann/json.hpp>
#include <cpprest/uri.h>
#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>
#include <string>

#include "local/local.hpp"
#include "camera/camera.hpp"


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
extern std::list<std::string> g_extra_assetbundle_paths;
extern char hotKey;
extern bool g_enable_free_camera;
extern bool g_block_out_of_focus;
extern float g_free_camera_mouse_speed;
extern bool g_allow_use_tryon_costume;
extern bool g_allow_same_idol;
