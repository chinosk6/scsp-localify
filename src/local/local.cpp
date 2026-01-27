#include <stdinclude.hpp>
#include "local.hpp"
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <filesystem>
#include <nlohmann/json.hpp>

using namespace std;

namespace SCLocal {
	namespace {
		std::unordered_map<std::string, std::unordered_map<int, std::string>> localTrans{};
		std::unordered_map<std::string, std::string> lrcTrans{};
		std::unordered_map<std::string, std::string> unLocalTrans_Legacy{}; // 用于存储 local2.json 等旧数据
		std::unordered_map<std::string, SubtitleData> unLocalTrans{}; // 用于存储 Timeline 数据 (UUID -> SubtitleData)
	}

	void loadGenericTrans(const char* fileName, std::unordered_map<std::string, std::string>& transDict) {
		try {
			transDict.clear();
			std::ifstream file(g_localify_base / fileName);
			if (!file.is_open()) {
				printf("Load %s failed: file not found.\n", fileName);
				return;
			}
			std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			file.close();
			auto fileData = nlohmann::json::parse(fileContent);
			for (auto& i : fileData.items()) {
				const auto& key = i.key();
				const std::string value = i.value();
				transDict[key] = value;
			}
		}
		catch (std::exception& e) {
			printf("Load %s failed: %s\n", fileName, e.what());
		}
	}

	void loadLrcTrans() {
		loadGenericTrans("lyrics.json", lrcTrans);
	}
	void loadUnlocalTrans() {
		loadGenericTrans("local2.json", unLocalTrans_Legacy);
	}

	void loadTimelineTrans() {
		std::filesystem::path timelinePath = g_localify_base / "translate_data";

		if (!std::filesystem::exists(timelinePath) || !std::filesystem::is_directory(timelinePath)) {
			printf("Timeline translation directory not found: %ls\n", timelinePath.c_str());
			return;
		}

		printf("Loading timeline translations (V2) from %ls...\n", timelinePath.c_str());
		int fileCount = 0;
		int itemCount = 0;

		try {
			for (const auto& entry : std::filesystem::recursive_directory_iterator(timelinePath)) {
				if (entry.is_regular_file()) {
					auto ext = entry.path().extension().string();
					std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
					if (ext == ".json") {
						try {
							std::ifstream file(entry.path());
						std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
						file.close();

						auto jsonArray = nlohmann::json::parse(content);
						if (jsonArray.is_array()) {
							for (const auto& item : jsonArray) {
								if (item.contains("uuid")) {
									std::string uuid = item["uuid"];
									SubtitleData data;

									// 1. 读取译文
									if (item.contains("translation")) {
										data.translation = item["translation"];
									}
									else if (item.contains("cn_text")) { // 兼容旧格式
										data.translation = item["cn_text"];
									}
									
									// 2. 读取原文
									if (item.contains("original")) {
										data.original = item["original"];
									}
									else if (item.contains("jp_text")) { // 兼容旧格式
										data.original = item["jp_text"];
									}

									// 3. 读取配置
									if (item.contains("config")) {
										auto& cfg = item["config"];
										if (cfg.contains("zhSize")) data.config.zhSize = cfg["zhSize"];
										if (cfg.contains("jpSize")) data.config.jpSize = cfg["jpSize"];
										if (cfg.contains("lineSpacing")) data.config.lineSpacing = cfg["lineSpacing"];
										if (cfg.contains("dualMode")) data.config.dualMode = cfg["dualMode"];
									}

									if (!uuid.empty() && !data.translation.empty()) {
										unLocalTrans[uuid] = data;
										itemCount++;
									}
								}
							}
							fileCount++;
						}
					}
					catch (std::exception& e) {
						printf("Error loading timeline file %ls: %s\n", entry.path().c_str(), e.what());
					}
				}
			}
		}
		}
		catch (std::exception& e) {
			printf("Error iterating timeline directory: %s\n", e.what());
		}

		printf("Loaded %d timeline files with %d entries.\n", fileCount, itemCount);
	}

	void loadLocalTrans() {
		loadLrcTrans();
		loadUnlocalTrans();
		loadTimelineTrans();
		localTrans.clear();
		printf("Loading localify.json...\n");
		int totalItemCount = 0;
		try {
			std::ifstream file(g_localify_base / "localify.json");
			if (!file.is_open()) {
				printf("Load localify.json failed: file not found.\n");
				return;
			}
			std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			file.close();
			auto fileData = nlohmann::json::parse(fileContent);
			for (auto& i : fileData.items()) {
				const auto& key = i.key();
				localTrans[key] = {};
				for (auto& v : i.value().items()) {
					const auto& subIdStr = v.key();
					const auto subId = std::stoi(subIdStr);
					std::string localText = v.value();
					if (auto it = unLocalTrans_Legacy.find(localText); it != unLocalTrans_Legacy.end()) {
						localText = it->second;
					}
					localTrans[key][subId] = localText;
					totalItemCount++;
				}
			}
		}
		catch (std::exception& e) {
			printf("Load localify.json failed: %s\n", e.what());
		}
		printf("%d items in localify.json loaded.\n", totalItemCount);
	}

	bool getLocalifyText(const std::string& category, int id, std::string* getStr) {
		if (auto it = localTrans.find(category); it != localTrans.end()) {
			const auto& value = it->second;
			if (auto vIt = value.find(id); vIt != value.end()) {
				*getStr = vIt->second;
				return true;
			}
		}
		return false;
	}

	/*
	Category: mlStory_MainStoryEpisode, mlMusic_CueSheet, mlMusic_MVScene
	Wait for game structure to modify these values (Placeholder)
	*/
	bool getLocalifyText(const std::wstring& category, int id, std::wstring* getStr) {
		const auto categoryS = utility::conversions::to_utf8string(category);
		std::string resultS = "";
		if (getLocalifyText(categoryS, id, &resultS)) {
			const auto resultWs = utility::conversions::to_utf16string(resultS);
			*getStr = resultWs;
			return true;
		}
		return false;
	}

	std::vector<std::wstring> split(const std::wstring& text, wchar_t delimiter) {
		std::vector<std::wstring> parts;
		std::wstring::size_type start = 0;
		std::wstring::size_type end = text.find(delimiter);
		while (end != std::wstring::npos) {
			parts.push_back(text.substr(start, end - start));
			start = end + 1;
			end = text.find(delimiter, start);
		}
		parts.push_back(text.substr(start));
		return parts;
	}

	std::filesystem::path splitFatherDirectoryByUnderline(const std::wstring& name) {
		auto parts = split(name, L'_');
		if (parts.size() == 1) {
			return ".";
		}

		std::filesystem::path filePath;
		for (int i = 0; i < 2; i++) {
			filePath /= parts[i];
		}
		return filePath;
	}

	std::filesystem::path getFilePathByName(const std::wstring& gamePath, bool createFatherPath, const std::filesystem::path& fatherBase) {
		std::filesystem::path localFileName;
		if (gamePath.starts_with(L"s")) {
			localFileName /= L"scenario";
		}
		const auto fatherPath = localFileName / splitFatherDirectoryByUnderline(gamePath);
		if (createFatherPath) {
			if (!std::filesystem::exists(fatherBase / fatherPath)) {
				std::filesystem::create_directories(fatherBase / fatherPath);
			}
		}
		return fatherPath / gamePath;
	}

	bool getLocalFileName(const std::wstring& gamePath, std::filesystem::path* localPath, bool checkExists) {
		auto localFileName = g_localify_base / getFilePathByName(gamePath, !checkExists, g_localify_base);
		if (std::filesystem::exists(localFileName) || !checkExists) {
			*localPath = localFileName;
			return true;
		}
		return false;
	}

	void dumpGenericText(const std::string& dumpStr, const char* fileName, bool withOrigText = false) {
		try {
			const std::filesystem::path dumpBasePath("dumps");
			const auto dumpFilePath = dumpBasePath / fileName;

			if (!std::filesystem::is_directory(dumpBasePath)) {
				std::filesystem::create_directories(dumpBasePath);
			}
			if (!std::filesystem::exists(dumpFilePath)) {
				std::ofstream dumpWriteLrcFile(dumpFilePath, std::ofstream::out);
				dumpWriteLrcFile << "{}";
				dumpWriteLrcFile.close();
			}

			std::ifstream dumpLrcFile(dumpFilePath);
			std::string fileContent((std::istreambuf_iterator<char>(dumpLrcFile)), std::istreambuf_iterator<char>());
			dumpLrcFile.close();
			auto fileData = nlohmann::ordered_json::parse(fileContent);
			fileData[dumpStr] = withOrigText ? dumpStr : "";
			const auto newStr = fileData.dump(4, 32, false);
			std::ofstream dumpWriteLrcFile(dumpFilePath, std::ofstream::out);
			dumpWriteLrcFile << newStr.c_str();
			dumpWriteLrcFile.close();
		}
		catch (std::exception& e) {
			printf("Dump text to %s error: %s\n", fileName, e.what());
		}

	}

	std::string replaceAll(const std::string& str, const std::string& oldStr, const std::string& newStr) {
		std::string result = str;
		size_t pos = 0;
		while ((pos = result.find(oldStr, pos)) != std::string::npos) {
			result.replace(pos, oldStr.length(), newStr);
			pos += newStr.length();
		}
		return result;
	}

	std::string getLyricsTrans(const std::wstring& orig) {
		// const auto lrcStr = replaceAll(replaceAll(utility::conversions::to_utf8string(orig), "\n", "\\n"), "\r", "\\r");
		const auto lrcStr = utility::conversions::to_utf8string(orig);
		if (auto iter = lrcTrans.find(lrcStr); iter != lrcTrans.end()) {
			return iter->second;
		}
		else {
			if (g_dump_untrans_lyrics) {
				dumpGenericText(lrcStr, "lyrics.json", true);
			}
		}
		return lrcStr;
	}

	bool getGameUnlocalTrans(const std::wstring& orig, std::string* newStr) {
		// const auto origStr = replaceAll(replaceAll(utility::conversions::to_utf8string(orig), "\n", "\\n"), "\r", "\\r");
		const auto origStr = utility::conversions::to_utf8string(orig);
		if (auto iter = unLocalTrans_Legacy.find(origStr); iter != unLocalTrans_Legacy.end()) {
			*newStr = iter->second;
			return true;
		}
		else {
			if (g_dump_untrans_unlocal) {
				dumpGenericText(origStr, "local2.json");
			}
		}
		return false;
	}

	bool getSubtitle(const std::string& key, SubtitleData& outData) {
		if (auto iter = unLocalTrans.find(key); iter != unLocalTrans.end()) {
			outData = iter->second;
			return true;
		}
		return false;
	}

	bool isScenarioTranslated(const std::string& scenarioId) {
		// Expected format: s44_01010100
		// Path: translate_data/s44/0101/s44_01010100.json
		
		size_t firstUnderscore = scenarioId.find('_');
		if (firstUnderscore == std::string::npos) return false;

		std::string prefix = scenarioId.substr(0, firstUnderscore);
		std::string idBody = scenarioId.substr(firstUnderscore + 1);

		if (idBody.length() < 4) return false;
		std::string subFolder = idBody.substr(0, 4);

		std::filesystem::path filePath = g_localify_base / "translate_data" / prefix / subFolder / (scenarioId + ".json");
		return std::filesystem::exists(filePath);
	}

	void addToMissingList(const std::string& scenarioId) {
		std::filesystem::path listPath = g_localify_base / "missing_scenarios.json";
		nlohmann::json jsonList;

		if (std::filesystem::exists(listPath)) {
			try {
				std::ifstream file(listPath);
				jsonList = nlohmann::json::parse(file);
			} catch (...) {
				jsonList = nlohmann::json::array();
			}
		} else {
			jsonList = nlohmann::json::array();
		}

		bool exists = false;
		for (const auto& item : jsonList) {
			if (item.is_string() && item.get<std::string>() == scenarioId) {
				exists = true;
				break;
			}
		}

		if (!exists) {
			jsonList.push_back(scenarioId);
			std::ofstream file(listPath);
			file << jsonList.dump(4);
			file.close();
			printf("[Dump] Added %s to missing list.\n", scenarioId.c_str());
		}
	}

	bool appendDumpEntry(const std::string& scenarioId, const std::string& uuid, const std::string& original, const std::string& name) {
		if (uuid.empty()) return false;

		std::string sXX, XXXX;
		std::string targetFileName;
		bool standardFormat = false;

		// UUID format check: sXX_XXXXXXXX_...
		if (uuid.length() >= 12 && uuid[0] == 's' && isdigit(uuid[1]) && isdigit(uuid[2]) && uuid[3] == '_') {
			sXX = uuid.substr(0, 3);
			XXXX = uuid.substr(4, 4);

			// Only split sections for Event Stories (s42)
			bool isMultiSection = (sXX == "s42");

			// Check for _YY_ pattern (section index) at index 12 (e.g. s42_01040300_00)
			// Length check: 3(sXX) + 1(_) + 8(XXXXXXXX) + 1(_) + 2(YY) = 15
			if (isMultiSection && uuid.length() >= 15 && uuid[12] == '_' && isdigit(uuid[13]) && isdigit(uuid[14])) {
				targetFileName = uuid.substr(0, 15); 
			} else {
				targetFileName = uuid.substr(0, 12); // e.g. s44_01010105
			}
			standardFormat = true;
		}

		if (!standardFormat) {
			targetFileName = "dump_unknown";
			if (!scenarioId.empty()) targetFileName = scenarioId;
		}

		// Path construction
		// translate_data/sXX/XXXX/targetFileName.json
		std::filesystem::path dumpPath = g_localify_base / "translate_data";
		if (standardFormat) {
			dumpPath /= sXX;
			dumpPath /= XXXX;
		} else {
			dumpPath /= "misc";
		}
		
		std::filesystem::create_directories(dumpPath);
		std::filesystem::path filePath = dumpPath / (targetFileName + ".json");

		// Load existing
		nlohmann::json jsonArray;
		if (std::filesystem::exists(filePath)) {
			try {
				std::ifstream file(filePath);
				jsonArray = nlohmann::json::parse(file);
				file.close();
			} catch (...) {
				jsonArray = nlohmann::json::array();
			}
		} else {
			jsonArray = nlohmann::json::array();
		}

		// Check if UUID exists
		bool found = false;
		for (auto& item : jsonArray) {
			if (item.contains("uuid") && item["uuid"] == uuid) {
				found = true;
				break;
			}
		}

		if (!found) {
			nlohmann::json newItem;
			newItem["uuid"] = uuid;
			newItem["original"] = original;
			newItem["translation"] = "";
			newItem["name"] = name;
			
			// Add config template
			nlohmann::json config;
			config["zhSize"] = 38;
			config["jpSize"] = 24;
			config["lineSpacing"] = -10;
			config["dualMode"] = true;
			newItem["config"] = config;

			jsonArray.push_back(newItem);

			// Write back
			try {
				std::ofstream outFile(filePath);
				outFile << jsonArray.dump(4);
				outFile.close();
				return true;
			} catch (std::exception& e) {
				printf("[Dump] Failed to write %s: %s\n", filePath.string().c_str(), e.what());
			}
		}
		return false;
	}
}
