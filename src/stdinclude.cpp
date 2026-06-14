#include <stdinclude.hpp>
#include <rapidjson/error/en.h>

namespace debug {
	void DumpRelationMemoryHex(const void* target, const size_t length)
	{
		if (target == nullptr || length == 0) {
			return;
		}

		constexpr size_t LINE_SIZE = 0x10;

		// Compute addresses as uintptr_t to allow arithmetic
		uintptr_t start = reinterpret_cast<uintptr_t>(target);

		// Compute how many bytes in total we will print.
		// We must print from 'start' through 'start + printed_total - 1' inclusive.
		// Ensure the printed region covers [target, target + length).
		uintptr_t end_needed = start + length;
		uintptr_t printed_total_bytes = 0;
		if (end_needed > start) {
			printed_total_bytes = end_needed - start;
		}
		else {
			printed_total_bytes = 0;
		}

		// Round printed_total_bytes up to a multiple of LINE_SIZE so we print full lines
		size_t lines = static_cast<size_t>((printed_total_bytes + (LINE_SIZE - 1)) / LINE_SIZE);
		if (lines == 0) {
			lines = 1; // at least one line
		}

		// Printing loop: read each line (LINE_SIZE bytes) into local buffer and print
		unsigned char buffer[LINE_SIZE];

		for (size_t line = 0; line < lines; ++line) {
			uintptr_t line_addr = start + line * LINE_SIZE;

			// For safety when reading possibly unaligned memory, copy byte-by-byte with memcpy from address
			// Note: if memory is unreadable, this will still likely crash; caller must ensure region is readable.
			for (size_t b = 0; b < LINE_SIZE; ++b) {
				uintptr_t byte_addr = line_addr + b;
				// Only fill valid bytes that fall within [start, end_needed). Otherwise zero.
				if (byte_addr < end_needed) {
					// Use memcpy to avoid undefined behaviour from dereferencing arbitrary pointer types
					std::memcpy(&buffer[b], reinterpret_cast<const void*>(byte_addr), 1);
				}
				else {
					buffer[b] = 0;
				}
			}

			// Print the address followed by 16 bytes in two uppercase hex characters each
			std::printf("%016" PRIxPTR ": ", line_addr);
			for (size_t b = 0; b < LINE_SIZE; ++b) {
				std::printf("%02X", static_cast<unsigned int>(buffer[b]));
				if (b + 1 < LINE_SIZE) std::putchar(' ');
			}
			std::putchar('\n');
		}
	}

	void DumpRegisters() {
		CONTEXT ctx;
		RtlCaptureContext(&ctx); // or CaptureContext(&ctx) on some toolchains

		std::printf("RIP=%016llX RSP=%016llX RBP=%016llX\n",
			(unsigned long long)ctx.Rip,
			(unsigned long long)ctx.Rsp,
			(unsigned long long)ctx.Rbp);
		std::printf("RAX=%016llX RBX=%016llX RCX=%016llX RDX=%016llX\n",
			(unsigned long long)ctx.Rax,
			(unsigned long long)ctx.Rbx,
			(unsigned long long)ctx.Rcx,
			(unsigned long long)ctx.Rdx);
		std::printf("RSI=%016llX RDI=%016llX  R8=%016llX  R9=%016llX\n",
			(unsigned long long)ctx.Rsi,
			(unsigned long long)ctx.Rdi,
			(unsigned long long)ctx.R8,
			(unsigned long long)ctx.R9);
	}

	static std::vector<const MethodInfo*> managedMethodTable{};
	static void PrepareManagedMethodAddressTable() {
		size_t assemblyCount = 0;
		auto** assemblies = il2cpp_domain_get_assemblies(il2cpp_domain_get(), &assemblyCount);

		for (size_t a = 0; a < assemblyCount; a++) {
			auto* image = il2cpp_assembly_get_image((void*)assemblies[a]);
			int classCount = il2cpp_image_get_class_count(image);

			for (int i = 0; i < classCount; i++) {
				auto* klass = il2cpp_image_get_class(image, i);
				void* iter = nullptr;
				const MethodInfo* method = nullptr;
				while ((method = il2cpp_class_get_methods((void*)klass, &iter))) {
					if (!method->methodPointer) continue;
					managedMethodTable.push_back(method);
				}
			}
		}

		std::sort(managedMethodTable.begin(), managedMethodTable.end(),
			[](const MethodInfo* a, const MethodInfo* b) {
				return a->methodPointer < b->methodPointer;
			});
	}
	static void EnsureManagedMethodTable() {
		if (!managedMethodTable.empty()) return;
		PrepareManagedMethodAddressTable();
	}
	const std::vector<const MethodInfo*>& GetManagedMethodTable() {
		EnsureManagedMethodTable();
		return managedMethodTable;
	}

	const MethodInfo* ResolveAddress(uintptr_t pc) {
		auto it = std::upper_bound(managedMethodTable.begin(), managedMethodTable.end(), pc,
			[](uintptr_t val, const MethodInfo* m) {
				return val < m->methodPointer;
			});

		if (it == managedMethodTable.begin()) return nullptr;
		return *--it;
	}

	static const char* TryGetKeywordName(const std::string& token, const std::string& currentNamespace) {
		if (currentNamespace != "System") return nullptr;
		if (token == "Void")    return "void";
		if (token == "Boolean") return "bool";
		if (token == "Byte")    return "byte";
		if (token == "SByte")   return "sbyte";
		if (token == "Int16")   return "short";
		if (token == "UInt16")  return "ushort";
		if (token == "Int32")   return "int";
		if (token == "UInt32")  return "uint";
		if (token == "Int64")   return "long";
		if (token == "UInt64")  return "ulong";
		if (token == "Single")  return "float";
		if (token == "Double")  return "double";
		if (token == "Decimal") return "decimal";
		if (token == "Char")    return "char";
		if (token == "String")  return "string";
		if (token == "Object")  return "object";
		return nullptr;
	}
	static std::string GetTypeName(const Il2CppType* type, bool includeNamespace) {
		if (!type) return "?";
		const char* fullName = il2cpp_type_get_name(type);
		if (!fullName) return "?";
		std::string result;
		std::string token;
		std::string currentNamespace; // tracks the namespace accumulated before the final class name
		for (char c : std::string_view(fullName)) {
			if (c == '.') {
				if (includeNamespace) {
					currentNamespace += token + "."; // accumulate namespace for keyword check
					token += "::";
				}
				else {
					currentNamespace += token + "."; // accumulate even when stripping, for keyword check
					token.clear();
				}
			}
			else if (c == '<' || c == '>' || c == ',') {
				// flush token ¡ª check for keyword substitution first
				const char* keyword = TryGetKeywordName(token, currentNamespace);
				// strip trailing '.' from accumulated namespace before comparing
				std::string ns = currentNamespace.empty() ? "" : currentNamespace.substr(0, currentNamespace.size() - 1);
				keyword = TryGetKeywordName(token, ns);
				result += keyword ? keyword : token;
				token.clear();
				currentNamespace.clear(); // reset namespace for next type argument
				result += c;
				if (c == ',') result += ' ';
			}
			else {
				token += c;
			}
		}
		// flush remainder
		std::string ns = currentNamespace.empty() ? "" : currentNamespace.substr(0, currentNamespace.size() - 1);
		const char* keyword = TryGetKeywordName(token, ns);
		result += keyword ? keyword : token;
		return result;
	}
	std::string FormatMethodInfo(const MethodInfo* method) {
		const char* ns = il2cpp_class_get_namespace((void*)method->klass);
		const char* className = il2cpp_class_get_name((void*)method->klass);
		std::string result;

		if (!il2cpp_method_is_instance(method))
			result += "static ";

		auto returnTypeName = GetTypeName(il2cpp_method_get_return_type(method), false);
		result = result + returnTypeName + " ";

		auto declTypeName = GetTypeName((Il2CppType*)il2cpp_class_get_type(il2cpp_method_get_class(method)), true);
		result = result + declTypeName + "." + il2cpp_method_get_name(method);

		result += "(";
		for (uint8_t i = 0; i < method->parameters_count; i++) {
			if (i > 0) result += ", ";
			auto paramTypeName = GetTypeName(il2cpp_method_get_param(method, i), false);
			result += paramTypeName;
		}
		result += ")";

		return result;
	}

	void PrintManagedStackTrace(ULONG framesToSkip, ULONG framesToCapture) {
		EnsureManagedMethodTable();

		std::vector<PVOID> backTrace(framesToCapture);
		ULONG captured = RtlCaptureStackBackTrace(framesToSkip, framesToCapture, backTrace.data(), NULL);

		printf("====== ManagedStackTrace ======\n");
		for (ULONG i = 0; i < captured; i++) {
			uintptr_t pc = reinterpret_cast<uintptr_t>(backTrace[i]);
			const MethodInfo* method = ResolveAddress(pc);

			if (method)
				printf("  %p | %s\n", backTrace[i], FormatMethodInfo(method).c_str());
			else
				printf("  %p\n", backTrace[i]);
		}
	}
}


LONG WINAPI seh_filter(EXCEPTION_POINTERS* ep) {
	DWORD code = ep->ExceptionRecord->ExceptionCode;
	PVOID addr = ep->ExceptionRecord->ExceptionAddress;

	std::cerr << "!! SEH Exception caught !!" << std::endl;
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

	debug::DumpRelationMemoryHex((const void*)((uintptr_t)addr - 0x20));
	debug::DumpRegisters();
	debug::PrintManagedStackTrace();

	return EXCEPTION_EXECUTE_HANDLER;
}


bool WriteClipboard(std::string& text) {
	if (!OpenClipboard(nullptr)) return false;
	EmptyClipboard();

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
	if (!hMem) {
		CloseClipboard();
		return false;
	}

	auto lock = GlobalLock(hMem);
	if (!lock) {
		CloseClipboard();
		return false;
	}
	memcpy(lock, text.c_str(), text.size() + 1);
	GlobalUnlock(hMem);

	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
	return true;
}

bool ReadClipboard(std::string* text) {
	if (!text || !OpenClipboard(nullptr)) return false;

	HANDLE hData = GetClipboardData(CF_TEXT);
	if (hData == nullptr) {
		CloseClipboard();
		return false;
	}

	char* pszText = static_cast<char*>(GlobalLock(hData));
	if (pszText == nullptr) {
		CloseClipboard();
		return false;
	}

	*text = pszText;
	GlobalUnlock(hData);
	CloseClipboard();
	return true;
}


LocalTransform::LocalTransform(const Il2CppObject* transform, bool readtransform) {
	this->transform = transform;
	if (transform) {
		ReadLocalPosition(transform);
		ReadLocalRotation(transform);
		ReadLocalScale(transform);
	}
}

LocalTransform::LocalTransform(const Il2CppObject* transform, Vector3_t localPosition, Quaternion_t localRotation, Vector3_t localScale) {
	this->transform = transform;
	this->localPosition = localPosition;
	this->localRotation = localRotation;
	this->localScale = localScale;
}

void LocalTransform::ReadLocalPosition(const Il2CppObject* transform) {
	static auto method_Transform_get_localPosition = il2cpp_symbols_logged::get_method(
		"UnityEngine.CoreModule.dll", "UnityEngine",
		"Transform", "get_localPosition", 0
	);
	localPosition = method_Transform_get_localPosition->Invoke((Il2CppObject*)transform, {})->unbox_value<Vector3_t>();
}

void LocalTransform::ReadLocalRotation(const Il2CppObject* transform) {
	static auto method_Transform_get_localRotation = il2cpp_symbols_logged::get_method(
		"UnityEngine.CoreModule.dll", "UnityEngine",
		"Transform", "get_localRotation", 0
	);
	localRotation = method_Transform_get_localRotation->Invoke((Il2CppObject*)transform, {})->unbox_value<Quaternion_t>();
}

void LocalTransform::ReadLocalScale(const Il2CppObject* transform) {
	static auto method_Transform_get_localScale = il2cpp_symbols_logged::get_method(
		"UnityEngine.CoreModule.dll", "UnityEngine",
		"Transform", "get_localScale", 0
	);
	localScale = method_Transform_get_localScale->Invoke((Il2CppObject*)transform, {})->unbox_value<Vector3_t>();
}

void LocalTransform::WriteLocalPosition(Il2CppObject* transform) {
	static auto method_Transform_set_localPosition = il2cpp_symbols_logged::get_method(
		"UnityEngine.CoreModule.dll", "UnityEngine",
		"Transform", "set_localPosition", 1
	);
	method_Transform_set_localPosition->InvokeAsVoid(transform, { (Il2CppObject*)&localPosition });
}

void LocalTransform::WriteLocalRotation(Il2CppObject* transform) {
	static auto method_Transform_set_localRotation = il2cpp_symbols_logged::get_method(
		"UnityEngine.CoreModule.dll", "UnityEngine",
		"Transform", "set_localRotation", 1
	);
	method_Transform_set_localRotation->InvokeAsVoid(transform, { (Il2CppObject*)&localRotation });
}

void LocalTransform::WriteLocalScale(Il2CppObject* transform) {
	static auto method_Transform_set_localScale = il2cpp_symbols_logged::get_method(
		"UnityEngine.CoreModule.dll", "UnityEngine",
		"Transform", "set_localScale", 1
	);
	method_Transform_set_localScale->InvokeAsVoid(transform, { (Il2CppObject*)&localScale });
}


void* UnitIdol::field_UnitIdol_charaId = nullptr;
void* UnitIdol::klass_UnitIdol = nullptr;
void* UnitIdol::field_UnitIdol_clothId = nullptr;
void* UnitIdol::field_UnitIdol_hairId = nullptr;
void* UnitIdol::field_UnitIdol_accessoryIds = nullptr;

void UnitIdol::ReadFrom(managed::UnitIdol* managed) {
	if (AccessoryIds != nullptr) {
		delete[] AccessoryIds;
	}
	InitUnitIdol(managed);
	void* accessoryIds;
	il2cpp_field_get_value(managed, field_UnitIdol_charaId, &CharaId);
	il2cpp_field_get_value(managed, field_UnitIdol_clothId, &ClothId);
	il2cpp_field_get_value(managed, field_UnitIdol_hairId, &HairId);
	il2cpp_field_get_value(managed, field_UnitIdol_accessoryIds, &accessoryIds);
	AccessoryIdsLength = il2cpp_array_length(accessoryIds);
	AccessoryIds = new int[AccessoryIdsLength];
	for (int i = 0; i < AccessoryIdsLength; ++i) {
		auto item = il2cpp_symbols::array_get_value(accessoryIds, i);
		int32_t* rawPtr = static_cast<int32_t*>(il2cpp_object_unbox((Il2CppObject*)item));
		int32_t value = *rawPtr;
		AccessoryIds[i] = value;
	}

	auto mstField = GetMstCostumeField(managed);
	if (mstField != nullptr) {
		uint64_t mstCostume = 0;
		il2cpp_field_get_value(managed, mstField, &mstCostume);
		// get the lower int (Item1) from managed `(int, int)`
		MstCostumeId = (int)mstCostume;
	}
}

void UnitIdol::ApplyTo(managed::UnitIdol* managed) {
	InitUnitIdol(managed);
	il2cpp_field_set_value(managed, field_UnitIdol_charaId, &CharaId);
	il2cpp_field_set_value(managed, field_UnitIdol_clothId, &ClothId);
	il2cpp_field_set_value(managed, field_UnitIdol_hairId, &HairId);
	static auto klass_System_Int32 = il2cpp_class_from_name(il2cpp_get_corlib(), "System", "Int32");
	auto accessoryIds = il2cpp_array_new(klass_System_Int32, AccessoryIdsLength);
	auto length = il2cpp_array_length(accessoryIds);
	for (int i = 0; i < AccessoryIdsLength; ++i) {
		auto boxed = il2cpp_value_box((Il2CppClass*)klass_System_Int32, &AccessoryIds[i]);
		il2cpp_symbols::array_set_value(accessoryIds, boxed, i);
	}
	il2cpp_field_set_value_object(managed, field_UnitIdol_accessoryIds, accessoryIds);

	auto mstField = GetMstCostumeField(managed);
	if (mstField != nullptr && MstCostumeId >= 0) {
		// combine the managed `(int, int)` data
		auto mstCostume = (((uint64_t)CharaId) << 32) + MstCostumeId;
		il2cpp_field_set_value(managed, mstField, &mstCostume);
	}
}

void UnitIdol::Clear() {
	CharaId = -1;
	ClothId = 0;
	HairId = 0;
	if (AccessoryIds != nullptr)
		delete[] AccessoryIds;
	AccessoryIds = nullptr;
	AccessoryIdsLength = 0;
	MstCostumeId = -1;
}

bool UnitIdol::IsEmpty() const {
	return this->CharaId < 0;
}

void UnitIdol::Print(std::ostream& os) const {
	os << "{ \"CharaId\": " << CharaId
		<< ", \"HairId\": " << HairId
		<< ", \"ClothId\": " << ClothId
		<< ", \"AccessoryIds\": [";
	bool first = true;
	if (AccessoryIds != nullptr) {
		for (int i = 0; i < AccessoryIdsLength; ++i) {
			if (!first)
				os << ", ";
			os << AccessoryIds[i];
			first = false;
		}
	}
	os << "], "
		<< "\"MstCostumeId\": " << MstCostumeId
		<< " }" << std::endl;
}

std::string UnitIdol::ToString() const {
	std::ostringstream oss;
	oss.imbue(std::locale::classic());
	Print(oss);
	return oss.str();
}

#define JSON_READ_INT(name) if (doc.HasMember(#name) && doc[#name].IsInt()) { name = doc[#name].GetInt(); }

void UnitIdol::LoadJson(const char* json) {
	rapidjson::Document doc;
	rapidjson::ParseResult result = doc.Parse(json);
	if (!result) {
		fprintf(stderr, "[ERROR] JSON parse error: %s (%zu)\n", rapidjson::GetParseError_En(result.Code()), result.Offset());
		return;
	}
	JSON_READ_INT(CharaId);
	JSON_READ_INT(HairId);
	JSON_READ_INT(ClothId);
	JSON_READ_INT(MstCostumeId);
	if (doc.HasMember("AccessoryIds") && doc["AccessoryIds"].IsArray()) {
		const rapidjson::Value& arr = doc["AccessoryIds"];
		delete[] AccessoryIds;
		AccessoryIds = new int[AccessoryIdsLength = arr.Size()];
		for (rapidjson::SizeType i = 0; i < AccessoryIdsLength; ++i) {
			if (arr[i].IsInt()) {
				AccessoryIds[i] = arr[i].GetInt();
			}
			else {
				fprintf(stderr, "[WARNING] UnitIdol::LoadJson: AccessoryIds[%d] isn't an int.", i);
			}
		}
	}
	return;
}


std::vector<std::pair<const Il2CppObject*, const Il2CppObject*>> GetActiveIdolObjects() {
	static auto method_SceneManager_get_sceneCount = il2cpp_symbols_logged::get_method(
		"UnityEngine.CoreModule.dll", "UnityEngine.SceneManagement",
		"SceneManager", "get_sceneCount", 0
	);
	static auto method_SceneManager_GetSceneAt = il2cpp_symbols_logged::get_method(
		"UnityEngine.CoreModule.dll", "UnityEngine.SceneManagement",
		"SceneManager", "GetSceneAt", 1
	);
	static auto method_Scene_GetRootGameObjects = il2cpp_symbols_logged::get_method(
		"UnityEngine.CoreModule.dll", "UnityEngine.SceneManagement",
		"Scene", "GetRootGameObjects", 0
	);

	std::vector<std::pair<const Il2CppObject*, const Il2CppObject*>> vec{};
	auto sceneCount = method_SceneManager_get_sceneCount->Invoke(nullptr, {})->unbox_value<int>();
	for (int sceneIndex = 0; sceneIndex < sceneCount; ++sceneIndex) {
		auto scene = method_SceneManager_GetSceneAt->Invoke(nullptr, { (Il2CppObject*)&sceneIndex });
		auto rootObjects = method_Scene_GetRootGameObjects->Invoke((Il2CppObject*)il2cpp_object_unbox(scene), {});
		int rootObjectsLength = il2cpp_array_length(rootObjects);
		for (int i = 0; i < rootObjectsLength; ++i) {
			auto obj = (Il2CppObject*)il2cpp_symbols::array_get_value(rootObjects, i);
			il2cpp_symbols::EnumerateAllChildrenGameObjects(obj,
				[&](Il2CppObject* go, Il2CppObject* tf) -> int {
					auto name = reflection::UnityObject_get_name(go)->ToUtf8String();
					if (name.starts_with("m_ALL_")) {
						std::cout << "[GetActiveIdolObjects] " << name << std::endl;
						vec.emplace_back(go, tf);
						return -1;
					}
					else return 1;
				}
			);
		}
	}
	return vec;
}


bool tools::output_networking_calls = false;
