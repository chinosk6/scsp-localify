# 代码改动对比报告 (Code Changes Summary)

本报告对比了 `feature/timeline-dump` 分支与远端主仓库 (`origin/main`) 的关键代码差异。

## 1. 核心稳定性修复 (Stability Fixes)

### 1.1 `CreatePlayable` Hook 签名修正 (`src/hook.cpp`)
**改动说明**: 修复了导致 `0xc0000005` 崩溃的函数签名错误。IL2CPP 返回结构体大于 8 字节时，第一个参数通常是隐藏的返回值指针 (`retstr`)。

```cpp:src/hook.cpp
// [Old] 错误的签名，导致堆栈不平衡和非法访问
// void* DepthOfFieldClip_CreatePlayable_hook(void* __this, void* graph, void* owner, void* method)

// [New] 修正后的签名，显式包含 retstr
void* DepthOfFieldClip_CreatePlayable_hook(void* retstr, void* __this, void* graph, void* owner, void* method) {
    // ...
    // 调用原函数时也需要传递 retstr
    return DepthOfFieldClip_CreatePlayable_orig(retstr, __this, graph, owner, method);
}
```

### 1.2 `IEnumerable` 安全遍历 (`src/il2cpp/il2cpp_symbols.cpp`)
**改动说明**: 在遍历 Unity 集合（如 Track 列表）时，增加了多重空指针检查，防止因对象销毁或反射失败导致的闪退。

```cpp:src/il2cpp/il2cpp_symbols.cpp
void il2cpp_symbols::iterate_IEnumerable(Il2CppObject* obj, std::function<void(Il2CppObject*)> callback) {
    // [New] 新增空指针防御
    if (!obj || !callback) return;

    // 获取 GetEnumerator 方法
    MethodInfo* getEnumeratorMethod = il2cpp_class_get_method_from_name(obj->klass, "GetEnumerator", 0);
    if (!getEnumeratorMethod) return;

    // ... 获取 Current 和 MoveNext ...

    // [New] 循环中的安全检查
    while (true) {
        Il2CppObject* moveNextResult = il2cpp_runtime_invoke(moveNextMethod, enumerator, nullptr, nullptr);
        bool hasMore = *reinterpret_cast<bool*>(il2cpp_object_unbox(moveNextResult));
        if (!hasMore) break;

        Il2CppObject* current = il2cpp_runtime_invoke(getCurrentMethod, enumerator, nullptr, nullptr);
        if (current) {
            callback(current);
        }
    }
}
```

## 2. 智能导出模块 (Smart Dump Module)

### 2.1 静态数据转换 (`src/local/local.cpp`)
**改动说明**: 新增了将原始 Scenario JSON 转换为 Timeline V2 格式的逻辑。

```cpp:src/local/local.cpp
// [New] 静态导出处理函数
void SCLocal::processStaticDump(const std::string& scenarioId, const std::string& jsonContent) {
    if (!g_dumpStaticEntries) return;

    // 解析原始 JSON
    auto j = nlohmann::json::parse(jsonContent);
    std::vector<std::string> strings;
    extractStrings(j, strings); // 递归提取所有日文字符串

    // 生成合成 UUID 并导出
    int index = 0;
    for (const auto& text : strings) {
        // 格式: sXX_XXXX_static_0001
        std::string syntheticUUID = std::format("{}_static_{:04d}", scenarioId, ++index);
        appendDumpEntry(scenarioId, syntheticUUID, text);
    }
}
```

### 2.2 V2 格式写入 (`src/local/local.cpp`)
**改动说明**: `appendDumpEntry` 函数重构，支持新的 JSON 结构和路径路由。

```cpp:src/local/local.cpp
bool SCLocal::appendDumpEntry(const std::string& scenarioId, const std::string& uuid, const std::string& text) {
    // [New] 智能路径路由：优先从 UUID 解析章节号 (s44/0101)
    std::string finalDir = "translate_data/" + parsePathFromUUID(uuid, scenarioId);
    
    // [New] V2 数据结构
    nlohmann::ordered_json entry;
    entry["uuid"] = uuid;
    entry["original"] = text;
    entry["translation"] = ""; // 留空待翻译
    // ...
}
```

## 3. 热重载机制改进 (Hot Reload)

### 3.1 轮询替代 Hook (`src/hook.cpp`)
**改动说明**: 放弃了不稳定的 `WndProc` 窗口消息 Hook，改用在 `LateUpdate` 中轮询 F5 键状态。

```cpp:src/hook.cpp
void MainThreadDispatcher_LateUpdate_hook(void* __this, void* method) {
    MainThreadDispatcher_LateUpdate_orig(__this, method);

    // [New] F5 热重载轮询 (带防抖)
    static bool f5_key_state = false;
    if (GetAsyncKeyState(VK_F5) & 0x8000) {
        if (!f5_key_state) {
            f5_key_state = true;
            printf("[HotKey] F5 pressed. Reloading translations...\n");
            SCLocal::loadLocalTrans(); // 重载函数
        }
    } else {
        f5_key_state = false;
    }
}
```

## 4. 配置文件 (`resources/scsp-config.json`)

```json
{
  "enable": true,
  "debug": true,
  // [New] 新增静态导出开关
  "dumpStaticEntries": false, 
  // ...
}
```

