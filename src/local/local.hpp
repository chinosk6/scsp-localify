#pragma once
#include <string>
#include <unordered_map>
#include <optional>
#include <vector>
#include <filesystem>

namespace SCLocal {
	// 定义 V2 数据结构
	struct SubtitleConfig {
		int zhSize = 38;
		int jpSize = 24;
		int lineSpacing = -10;
		bool dualMode = true;
	};

	struct SubtitleData {
		std::string original;    // 日文原文
		std::string translation; // 中文译文
		SubtitleConfig config;   // 样式配置
	};

	void loadLocalTrans();
	void loadLrcTrans();
	void loadGenericTrans();
	void loadUnlocalTrans();

	bool getGameUnlocalTrans(const std::wstring& hash, std::string* outTrans);
	bool getLrc(const std::string& hash, std::string* outTrans);

	// Legacy / Utility functions required by hook.cpp
	std::string getLyricsTrans(const std::wstring& orig);
	bool getLocalifyText(const std::string& category, int id, std::string* getStr);
	bool getLocalifyText(const std::wstring& category, int id, std::wstring* getStr);
	std::filesystem::path getFilePathByName(const std::wstring& gamePath, bool createFatherPath, const std::filesystem::path& fatherBase);
	
	// Added default value for checkExists to match previous usage
	bool getLocalFileName(const std::wstring& gamePath, std::filesystem::path* localPath, bool checkExists = true);

	// 新的接口：返回完整的 SubtitleData 对象
	bool getSubtitle(const std::string& key, SubtitleData& outData);

	// Dump Module Helpers
	bool isScenarioTranslated(const std::string& scenarioId);
	void addToMissingList(const std::string& scenarioId);
}
