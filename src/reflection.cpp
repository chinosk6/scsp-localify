#include "reflection.hpp"
#include "il2cpp/il2cpp_symbols.hpp"

namespace reflection {
	Il2CppReflectionType* Object_GetType(Il2CppObject* instance) {
		if (instance == nullptr) return nullptr;
		auto klass = il2cpp_object_get_class(instance);
		return (Il2CppReflectionType*)il2cpp_class_get_type(klass);
	}

	Il2CppString* Exception_getMessage(Il2CppObject* exception) {
		static auto method = il2cpp_class_get_method_from_name(
			il2cpp_class_from_name(il2cpp_get_corlib(), "System", "Exception"),
			"get_Message", 0
		);
		return (reinterpret_cast<Il2CppString * (*)(Il2CppObject*)>(method->methodPointer))(exception);
	}

	Il2CppObject* Invoke(MethodInfo* method, Il2CppObject* instance, Il2CppObject** params, const char* context) {
		Il2CppObject* exc = nullptr;
		auto ret = (Il2CppObject*)il2cpp_runtime_invoke(method, (void*)instance, (void**)params, &exc);
		if (exc) {
			auto klass = il2cpp_object_get_class(exc);
			auto klassName = il2cpp_class_get_name(klass);
			std::cerr << "[EXCEPTION] " << klassName << ": ";
			auto excMessage = Exception_getMessage(exc);
			std::wstring wstr(il2cpp_string_chars(excMessage), il2cpp_string_length(excMessage));
			std::wcerr << wstr;
			if (context != nullptr) {
				std::cerr << " (" << context << ")";
			}
			std::cerr << std::endl;
			return nullptr;
		}
		return ret;
	}
}