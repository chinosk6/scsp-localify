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
}

void UnitIdol::Clear() {
	CharaId = -1;
	ClothId = 0;
	HairId = 0;
	delete[] AccessoryIds;
	AccessoryIds = nullptr;
	AccessoryIdsLength = 0;
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
	os << "] }" << std::endl;
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


bool tools::output_networking_calls = false;
