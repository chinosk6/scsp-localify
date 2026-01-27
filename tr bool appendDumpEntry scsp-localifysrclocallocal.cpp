[1mdiff --git a/src/hook.cpp b/src/hook.cpp[m
[1mindex 6715af4..a0deb32 100644[m
[1m--- a/src/hook.cpp[m
[1m+++ b/src/hook.cpp[m
[36m@@ -26,6 +26,11 @@[m [mstd::map<std::string, CharaParam_t> charaParam{};[m
 CharaParam_t baseParam(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);[m
 std::vector<std::function<bool()>> mainThreadTasks{};  // 返回 true，执行后移除列表；返回 false，执行后不移除[m
 [m
[32m+[m	[32m// Dump Module Globals[m[41m[m
[32m+[m	[32mbool g_isDumping = true; // Auto-dump by default for development[m[41m[m
[32m+[m	[32mstd::string g_dumpingScenarioId = "";[m[41m[m
[32m+[m	[32mstd::set<std::string> g_dumpedUUIDs;[m[41m[m
[32m+[m[41m[m
 std::map<int, CharaSwayStringParam_t> charaSwayStringOffset{};[m
 std::map<int, std::string> swayTypes{[m
 	{0x0, "Test"},[m
[36m@@ -2225,38 +2230,143 @@[m [mbool g_shouldDumpCurrentScenario = false;[m
 [m
 [m
 	HOOK_ORIG_TYPE DramaSubtitlePlayableAsset_CreatePlayable_orig;[m
[32m+[m	[32mHOOK_ORIG_TYPE DepthOfFieldClip_CreatePlayable_orig;[m[41m[m
[32m+[m[41m[m
 	// Merged into DepthOfFieldClip_CreatePlayable_hook due to method folding/shared address[m
[32m+[m	[32m// DramaSubtitlePlayableAsset::CreatePlayable shares the same function address as DepthOfFieldClip::CreatePlayable[m[41m[m
[32m+[m	[32m// likely due to IL2CPP Identical Code Folding (ICF).[m[41m[m
[32m+[m	[32mvoid DepthOfFieldClip_CreatePlayable_hook(void* retstr, void* _this, void* graph, void* go) {[m[41m[m
[32m+[m		[32mstatic auto DramaSubtitlePlayableAsset_klass = il2cpp_symbols::get_class("PRISM.Interactions.Drama.dll", "PRISM.Interactions.Drama", "DramaSubtitlePlayableAsset");[m[41m[m
[32m+[m		[32mif (!DramaSubtitlePlayableAsset_klass)[m[41m[m
[32m+[m			[32mDramaSubtitlePlayableAsset_klass = il2cpp_symbols::get_class("PRISM.Legacy.dll", "PRISM.Interactions.Drama", "DramaSubtitlePlayableAsset");[m[41m[m
[32m+[m[41m		[m
[32m+[m		[32mauto this_klass = il2cpp_symbols::get_class_from_instance(_this);[m[41m[m
[32m+[m[41m		[m
[32m+[m		[32mif (this_klass == DramaSubtitlePlayableAsset_klass) {[m[41m[m
[32m+[m			[32m// Extract Data[m[41m[m
[32m+[m			[32mstatic auto behaviour_field = il2cpp_class_get_field_from_name(DramaSubtitlePlayableAsset_klass, "behaviour");[m[41m[m
[32m+[m			[32mif (!behaviour_field) behaviour_field = il2cpp_class_get_field_from_name(DramaSubtitlePlayableAsset_klass, "m_Template");[m[41m[m
[32m+[m[41m[m
[32m+[m			[32mif (behaviour_field) {[m[41m[m
[32m+[m				[32mauto behaviour = il2cpp_field_get_value_object(behaviour_field, _this);[m[41m[m
[32m+[m				[32mif (behaviour) {[m[41m[m
[32m+[m					[32mstatic auto behaviour_klass = il2cpp_symbols::get_class_from_instance(behaviour);[m[41m[m
[32m+[m					[32mstatic auto uniqueId_field = il2cpp_class_get_field_from_name(behaviour_klass, "uniqueId");[m[41m[m
[32m+[m					[32mif (!uniqueId_field) uniqueId_field = il2cpp_class_get_field_from_name(behaviour_klass, "uuid");[m[41m[m
[32m+[m					[32mstatic auto text_field = il2cpp_class_get_field_from_name(behaviour_klass, "text");[m[41m[m
[32m+[m					[32mif (!text_field) text_field = il2cpp_class_get_field_from_name(behaviour_klass, "_text");[m[41m[m
[32m+[m[41m[m
[32m+[m					[32m// Character Name Fields[m[41m[m
[32m+[m					[32mstatic auto displayTalkerName_field = il2cpp_class_get_field_from_name(behaviour_klass, "displayTalkerName");[m[41m[m
[32m+[m					[32mif (!displayTalkerName_field) displayTalkerName_field = il2cpp_class_get_field_from_name(behaviour_klass, "_displayTalkerName");[m[41m[m
[32m+[m					[32mstatic auto talkerName_field = il2cpp_class_get_field_from_name(behaviour_klass, "talkerName");[m[41m[m
[32m+[m					[32mif (!talkerName_field) talkerName_field = il2cpp_class_get_field_from_name(behaviour_klass, "_talkerName");[m[41m[m
[32m+[m[41m[m
[32m+[m					[32mif (uniqueId_field && text_field) {[m[41m[m
[32m+[m						[32mIl2CppString* uniqueIdStr = nullptr;[m[41m[m
[32m+[m						[32mil2cpp_field_get_value(behaviour, uniqueId_field, &uniqueIdStr);[m[41m[m
[32m+[m[41m[m
[32m+[m						[32mif (uniqueIdStr) {[m[41m[m
[32m+[m							[32mstd::string uidStr = uniqueIdStr->ToUtf8String();[m[41m[m
[32m+[m							[32m// if (g_debugMode) printf("[InjectTranslation] Found uniqueId: %s\n", uidStr.c_str());[m[41m[m
[32m+[m[41m[m
[32m+[m							[32mSCLocal::SubtitleData subData;[m[41m[m
[32m+[m							[32mbool isTranslated = SCLocal::getSubtitle(uidStr, subData);[m[41m[m
[32m+[m[41m							[m
[32m+[m							[32m// Extract Original Text[m[41m[m
[32m+[m							[32mIl2CppString* textStr = nullptr;[m[41m[m
[32m+[m							[32mil2cpp_field_get_value(behaviour, text_field, &textStr);[m[41m[m
[32m+[m							[32mstd::string originalText = textStr ? textStr->ToUtf8String() : "";[m[41m[m
[32m+[m[41m[m
[32m+[m							[32m// Extract Character Name[m[41m[m
[32m+[m							[32mstd::string charName = "Unknown";[m[41m[m
[32m+[m							[32mbool foundName = false;[m[41m[m
[32m+[m							[32mif (displayTalkerName_field) {[m[41m[m
[32m+[m								[32mIl2CppString* nameIl = nullptr;[m[41m[m
[32m+[m								[32mil2cpp_field_get_value(behaviour, displayTalkerName_field, &nameIl);[m[41m[m
[32m+[m								[32mif (nameIl) {[m[41m[m
[32m+[m									[32mstd::string n = nameIl->ToUtf8String();[m[41m[m
[32m+[m									[32mif (!n.empty()) {[m[41m[m
[32m+[m										[32mcharName = n;[m[41m[m
[32m+[m										[32mfoundName = true;[m[41m[m
[32m+[m									[32m}[m[41m[m
[32m+[m								[32m}[m[41m[m
[32m+[m							[32m}[m[41m[m
[32m+[m							[32mif (!foundName && talkerName_field) {[m[41m[m
[32m+[m								[32mIl2CppString* nameIl = nullptr;[m[41m[m
[32m+[m								[32mil2cpp_field_get_value(behaviour, talkerName_field, &nameIl);[m[41m[m
[32m+[m								[32mif (nameIl) {[m[41m[m
[32m+[m									[32mstd::string n = nameIl->ToUtf8String();[m[41m[m
[32m+[m									[32mif (!n.empty()) {[m[41m[m
[32m+[m										[32mcharName = n;[m[41m[m
[32m+[m									[32m}[m[41m[m
[32m+[m								[32m}[m[41m[m
[32m+[m							[32m}[m[41m[m
 [m
[32m+[m							[32m// Dump Logic[m[41m[m
[32m+[m							[32mif (g_isDumping && !isTranslated) {[m[41m[m
[32m+[m								[32m// Only dump if this scenario ID matches the one we want to dump[m[41m[m
[32m+[m								[32m// or if we are dumping everything (which we probably shouldn't do blindly)[m[41m[m
[32m+[m								[32m// Here we check against g_dumpingScenarioId[m[41m[m
[32m+[m								[32mif (!g_dumpingScenarioId.empty() && g_currentScenarioId == g_dumpingScenarioId) {[m[41m[m
[32m+[m									[32mif (!g_dumpedUUIDs.contains(uidStr)) {[m[41m[m
[32m+[m										[32mg_dumpedUUIDs.insert(uidStr);[m[41m[m
[32m+[m										[32mSCLocal::appendDumpEntry(g_dumpingScenarioId, uidStr, originalText, charName);[m[41m[m
[32m+[m										[32mif (g_debugMode) printf("[Dump] Dumped %s (%s)\n", uidStr.c_str(), charName.c_str());[m[41m[m
[32m+[m									[32m}[m[41m[m
[32m+[m								[32m}[m[41m[m
[32m+[m							[32m}[m[41m[m
 [m
[31m-	// obsolete hook removed[m
[31m-	// HDR Live[m
[31m-	//HOOK_ORIG_TYPE PostProcess_DepthOfFieldClip_CreatePlayable_orig;[m
[31m-	//void PostProcess_DepthOfFieldClip_CreatePlayable_hook(void* retstr, void* _this, void* graph, void* go, void* mtd) {[m
[32m+[m							[32mif (isTranslated) {[m[41m[m
[32m+[m								[32mif (g_debugMode) printf("Translating Drama Subtitle: %s -> %s\n", uidStr.c_str(), subData.translation.c_str());[m[41m[m
 [m
[31m-	//	if (g_enable_free_camera) {[m
[31m-	//		static auto DepthOfFieldClip_klass = il2cpp_symbols::get_class("PRISM.Legacy.dll", "UnityEngine.Rendering.Universal.PostProcess", "DepthOfFieldClip");[m
[31m-	//		static auto DepthOfFieldClip_behaviour_field = il2cpp_class_get_field_from_name(DepthOfFieldClip_klass, "behaviour");[m
[32m+[m								[32mstd::string finalText;[m[41m[m
[32m+[m								[32mif (subData.config.dualMode && !subData.original.empty()) {[m[41m[m
[32m+[m									[32m// Interleaved Layout[m[41m[m
[32m+[m									[32mauto zhLines = splitString(subData.translation);[m[41m[m
[32m+[m									[32mauto jpLines = splitString(subData.original);[m[41m[m
 [m
[31m-	//		static auto DepthOfFieldBehaviour_klass = il2cpp_symbols::get_class("PRISM.Legacy.dll", "UnityEngine.Rendering.Universal.PostProcess", "DepthOfFieldBehaviour");[m
[31m-	//		static auto DepthOfFieldBehaviour_focusDistance_field = il2cpp_class_get_field_from_name(DepthOfFieldBehaviour_klass, "focusDistance");[m
[31m-	//		static auto DepthOfFieldBehaviour_aperture_field = il2cpp_class_get_field_from_name(DepthOfFieldBehaviour_klass, "aperture");[m
[31m-	//		static auto DepthOfFieldBehaviour_focalLength_field = il2cpp_class_get_field_from_name(DepthOfFieldBehaviour_klass, "focalLength");[m
[31m-	//		auto depthOfFieldBehaviour = il2cpp_symbols::read_field(_this, DepthOfFieldClip_behaviour_field);[m
[31m-	//		/*[m
[31m-	//		auto focusDistance = il2cpp_symbols::read_field<float>(depthOfFieldBehaviour, DepthOfFieldBehaviour_focusDistance_field);[m
[31m-	//		auto aperture = il2cpp_symbols::read_field<float>(depthOfFieldBehaviour, DepthOfFieldBehaviour_aperture_field);[m
[31m-	//		auto focalLength = il2cpp_symbols::read_field<float>(depthOfFieldBehaviour, DepthOfFieldBehaviour_focalLength_field);[m
[31m-	//		*/[m
[32m+[m									[32mstd::string combinedText = "";[m[41m[m
[32m+[m									[32msize_t maxLines = std::max(zhLines.size(), jpLines.size());[m[41m[m
 [m
[31m-	//		il2cpp_symbols::write_field(depthOfFieldBehaviour, DepthOfFieldBehaviour_focusDistance_field, 1000.0f);[m
[31m-	//		il2cpp_symbols::write_field(depthOfFieldBehaviour, DepthOfFieldBehaviour_aperture_field, 32.0f);[m
[31m-	//		il2cpp_symbols::write_field(depthOfFieldBehaviour, DepthOfFieldBehaviour_focalLength_field, 1.0f);[m
[32m+[m									[32mfor (size_t i = 0; i < maxLines; i++) {[m[41m[m
[32m+[m										[32mif (i > 0) combinedText += "\n";[m[41m[m
 [m
[31m-	//		// printf("DepthOfFieldClip_CreatePlayable, focusDistance: %f, aperture: %f, focalLength: %f\n", focusDistance, aperture, focalLength);[m
[31m-	//	}[m
[32m+[m										[32m// Japanese Line (Top)[m[41m[m
[32m+[m										[32mif (i < jpLines.size() && !jpLines[i].empty()) {[m[41m[m
[32m+[m											[32mstd::string jpLine = jpLines[i];[m[41m[m
[32m+[m											[32mjpLine.erase(std::remove(jpLine.begin(), jpLine.end(), '\r'), jpLine.end());[m[41m[m
[32m+[m											[32mint jpLineHeight = 100 + subData.config.lineSpacing;[m[41m[m
[32m+[m											[32mcombinedText += std::format("<line-height={}%><nobr><size={}><color=#CCCCCC>{}</color></size></nobr></line-height>", jpLineHeight, subData.config.jpSize, jpLine);[m[41m[m
[32m+[m											[32mcombinedText += "\n";[m[41m[m
[32m+[m										[32m}[m[41m[m
 [m
[31m-	//	HOOK_CAST_CALL(void, PostProcess_DepthOfFieldClip_CreatePlayable)(retstr, _this, graph, go, mtd);[m
[31m-	//}[m
[32m+[m										[32m// Chinese Line (Bottom)[m[41m[m
[32m+[m										[32mif (i < zhLines.size() && !zhLines[i].empty()) {[m[41m[m
[32m+[m											[32mstd::string zhLine = zhLines[i];[m[41m[m
[32m+[m											[32mzhLine.erase(std::remove(zhLine.begin(), zhLine.end(), '\r'), zhLine.end());[m[41m[m
[32m+[m											[32mstd::string zhFormatted = std::format("<line-height=100%><nobr><size={}>{}</size></nobr></line-height>", subData.config.zhSize, zhLine);[m[41m[m
[32m+[m											[32mcombinedText += zhFormatted;[m[41m[m
[32m+[m										[32m}[m[41m[m
[32m+[m									[32m}[m[41m[m
[32m+[m									[32mfinalText = combinedText;[m[41m[m
[32m+[m								[32m}[m[41m[m
[32m+[m								[32melse {[m[41m[m
[32m+[m									[32mfinalText = subData.translation;[m[41m[m
[32m+[m								[32m}[m[41m[m
[32m+[m[41m[m
[32m+[m								[32mauto wTranslation = utility::conversions::to_utf16string(finalText);[m[41m[m
[32m+[m								[32mil2cpp_field_set_value(behaviour, text_field, il2cpp_string_new_utf16((const wchar_t*)wTranslation.c_str(), wTranslation.length()));[m[41m[m
[32m+[m							[32m}[m[41m[m
[32m+[m						[32m}[m[41m[m
[32m+[m					[32m}[m[41m[m
[32m+[m				[32m}[m[41m[m
[32m+[m			[32m}[m[41m[m
[32m+[m		[32m}[m[41m[m
[32m+[m[41m		[m
[32m+[m		[32m// Call Original (Shared)[m[41m[m
[32m+[m		[32mHOOK_CAST_CALL(void, DepthOfFieldClip_CreatePlayable)(retstr, _this, graph, go);[m[41m[m
[32m+[m	[32m}[m[41m[m
 [m
 	// 已过时[m
 	HOOK_ORIG_TYPE Live_SetEnableDepthOfField_orig;[m
[36m@@ -3911,10 +4021,12 @@[m [mbool g_shouldDumpCurrentScenario = false;[m
 		ADD_HOOK(TextLog_AddLog, "TextLog_AddLog at %p");[m
 		ADD_HOOK(InvokeMoveNext, "InvokeMoveNext at %p");[m
 		// ADD_HOOK(Live_SetEnableDepthOfField, "Live_SetEnableDepthOfField at %p");[m
[31m-		// ADD_HOOK(DepthOfFieldClip_CreatePlayable, "DepthOfFieldClip_CreatePlayable at %p");[m
[31m-		ADD_HOOK(PlayableDirector_Play, "PlayableDirector_Play at %p");[m
[31m-		ADD_HOOK(PlayableDirector_Play_NoArg, "PlayableDirector_Play_NoArg at %p");[m
[32m+[m		[32mADD_HOOK(DepthOfFieldClip_CreatePlayable, "DepthOfFieldClip_CreatePlayable at %p");[m[41m[m
[32m+[m		[32m// DramaSubtitlePlayableAsset::CreatePlayable shares the same function address as DepthOfFieldClip::CreatePlayable[m[41m[m
[32m+[m		[32m// likely due to IL2CPP Identical Code Folding (ICF). Hooking one effectively hooks both.[m[41m[m
 		// ADD_HOOK(DramaSubtitlePlayableAsset_CreatePlayable, "DramaSubtitlePlayableAsset_CreatePlayable at %p");[m
[32m+[m[41m		[m
[32m+[m		[32mADD_HOOK(PlayableDirector_Play, "PlayableDirector_Play at %p");[m[41m[m
 		//ADD_HOOK(PostProcess_DepthOfFieldClip_CreatePlayable, "PostProcess_DepthOfFieldClip_CreatePlayable at %p");[m
 		// ADD_HOOK(Live_Update, "Live_Update at %p");[m
 		//ADD_HOOK(LiveCostumeChangeView_setTryOnMode, "LiveCostumeChangeView_setTryOnMode at %p");[m
