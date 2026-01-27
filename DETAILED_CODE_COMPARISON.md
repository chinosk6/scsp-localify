# 详细代码改动对比报告 (Detailed Code Comparison)

本报告基于 `feature/timeline-dump` 分支与 `chinosk6/scsp-localify` 主仓库 (`upstream/main`) 的完整比对。

## 1. 核心 Hook 逻辑 (`src/hook.cpp`)

### 1.1 `DepthOfFieldClip_CreatePlayable_hook` (重构与修复)
这是本次修改中最关键的部分，承载了剧情字幕的注入逻辑。

*   **修复崩溃 (Critical Fix)**: 
    *   修正了 Hook 函数签名。由于 `DramaSubtitlePlayableAsset` 的 CreatePlayable 返回值是结构体（大于8字节），IL2CPP 约定第一个参数必须是隐式的返回值指针 (`void* retstr`)。
    *   旧代码: `void* func(void* __this, ...)` -> 导致 `0xc0000005` 访问违规。
    *   新代码: `void* func(void* retstr, void* __this, ...)`。
*   **方法折叠处理 (Method Folding)**: 
    *   增加了对 `DramaSubtitlePlayableAsset` 的类名检查，防止因 Unity 方法折叠（不同类共享同一函数地址）导致 Hook 误触发其他 PlayableAsset。
*   **双语字幕注入**:
    *   实现了 `original` (原文) 与 `translation` (译文) 的组合逻辑。
    *   应用了 `<size>` 和 `lineSpacing` 等富文本标签控制排版。
*   **动态导出 (Dynamic Dump)**:
    *   在 Hook 内部集成了导出逻辑：当 `g_isDumping` 为真且当前 UUID 未翻译时，调用 `SCLocal::appendDumpEntry`。

### 1.2 `ScenarioManager_Init_hook` (新增)
*   **功能**: 拦截剧情初始化，获取当前剧情 ID (`Scenario_Home`, `s44_01...`)。
*   **逻辑**: 
    *   检查当前剧情是否已完全汉化 (`isScenarioTranslated`)。
    *   如果未汉化，设置全局标志 `g_isDumping = true`，触发后续的动态导出流程。

### 1.3 `MainThreadDispatcher_LateUpdate_hook` (新增)
*   **功能**: 替代原有的 `WndProc` 消息钩子，实现更稳定的热重载。
*   **逻辑**: 每帧轮询 `VK_F5` 键状态（带防抖），触发 `SCLocal::loadLocalTrans()`。

### 1.4 `DumpTimeline` & `il2cpp_timeline` (新增/实验性)
*   **功能**: 包含了一整套用于遍历 Unity Timeline 结构的反射辅助函数 (`Init`, `iterate_IEnumerable`)。
*   **状态**: 代码已合入，但 `PlayableDirector_Play_hook` 中的调用目前被注释（`// Dump logic disabled...`），目前主要使用 `CreatePlayable` 进行逐句导出，此模块保留作为批量导出的备选方案。

### 1.5 `fmtAndDumpJsonBytesData` (修改)
*   **功能**: 修改了通用的 JSON 导出函数。
*   **变动**: 增加了对 `g_dumpStaticEntries` 开关的检查。如果开启，调用 `SCLocal::processStaticDump` 将原始 JSON 转换为 V2 格式。

---

## 2. 本地化核心模块 (`src/local/local.cpp` & `.hpp`)

### 2.1 数据结构升级 (V2 Format)
*   **`SubtitleData` 结构体**: 新增 `original` (原文), `jpSize`, `zhSize`, `lineSpacing` 字段。
*   **`loadTimelineTrans`**: 全新的加载器，支持递归读取 `translate_data` 目录下的 V2 格式文件，并建立 UUID 索引。

### 2.2 智能导出 (Smart Dump)
*   **`appendDumpEntry`**: 
    *   **V2 格式化**: 使用 `nlohmann::ordered_json` 保证字段顺序 (`uuid` -> `name` -> `original` -> `translation`)。
    *   **路径路由**: 实现了 `parsePathFromUUID`，优先从 UUID (`s44_01010105_...`) 解析出 `s44/0101` 路径，解决了非标准 ScenarioID 导致的文件混乱。
    *   **防重**: 写入前检查 `loadedScenarios` 缓存。
*   **`processStaticDump` (静态转换)**:
    *   **逻辑**: 解析原始 Scenario JSON -> 递归提取所有含日文的字符串 -> 生成合成 UUID (`_static_`) -> 导出为 V2 格式。
    *   **辅助函数**: `hasJapanese` (正则检测), `extractStrings` (递归遍历 JSON)。

### 2.3 状态管理
*   **`loadedScenarios`**: 新增 `std::set` 用于记录已加载或已导出的文件，防止重复 IO。
*   **`missing_scenarios.json`**: 自动记录缺失翻译的剧情 ID。

---

## 3. IL2CPP 基础库 (`src/il2cpp/il2cpp_symbols.cpp`)

### 3.1 `iterate_IEnumerable` (增强)
*   **背景**: Unity 的 `IEnumerable` 遍历在对象被销毁或未初始化时极易崩溃。
*   **改动**: 
    *   增加了对 `obj`, `callback`, `GetEnumerator`, `MoveNext`, `Current` 的全流程空指针检查。
    *   这是解决之前 "进入剧情随机闪退" 问题的关键。

---

## 4. 其他改动

*   **`src/mhotkey.cpp`**: 实现了通用的热键注册表 `register_hotkey`，支持 `GetAsyncKeyState` 轮询模式。
*   **`resources/scsp-config.json`**:
    *   新增 `"dumpStaticEntries": false`。
    *   新增 `"dumpUntransTimeline": true` (隐含控制)。
*   **`src/main.cpp`**: 加载上述新配置项。
*   **`MODIFICATION_REPORT.md`**: 新增的修改记录文档。

## 5. 总结
相比原仓库，我们不仅修复了严重的稳定性 Bug，还重构了整个汉化数据的**生产（Dump）- 消费（Hook）**链路，使其支持更精细的控制（Timeline V2）和更高效的开发流程（热重载、静态转换）。

