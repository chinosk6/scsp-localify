#include "reflection.hpp"
#include "il2cpp/il2cpp_symbols.hpp"

namespace reflection {
	Il2CppReflectionType* typeof(const Il2CppClass* klass, const char* namespaze, const char* klassName) {
		if (!klass) {
			printf("[ERROR] Failed to find klass \"%s::%s\".\n", namespaze, klassName);
			return nullptr;
		}
		auto cppType = il2cpp_class_get_type((void*)klass);
		if (!cppType) {
			printf("[ERROR] Failed to get Il2CppType* for \"%s::%s\".\n", namespaze, klassName);
			return nullptr;
		}
		auto managedType = il2cpp_type_get_object(cppType);
		if (!managedType) {
			printf("[ERROR] Failed to get Il2CppReflectionType* for \"%s::%s\".\n", namespaze, klassName);
			return nullptr;
		}
		return managedType;
	}

	Il2CppReflectionType* typeof(const char* assemblyName, const char* namespaze, const char* klassName) {
		auto klass = (Il2CppClass*)il2cpp_symbols_logged::get_class(assemblyName, namespaze, klassName);
		return typeof(klass, namespaze, klassName);
	}

	Il2CppReflectionType* typeof_corlib(const char* klassName) {
		return typeof_corlib("System", klassName);
	}

	Il2CppReflectionType* typeof_corlib(const char* namespaze, const char* klassName) {
		auto klass = (Il2CppClass*)il2cpp_class_from_name(il2cpp_get_corlib(), namespaze, klassName);
		return typeof(klass, namespaze, klassName);
	}

	template<typename T> Il2CppArray* CreateManagedArray(Il2CppClass* elementType, const std::vector<T>& vector) {
		auto managed = (Il2CppArray*)il2cpp_array_new(elementType, vector.size());
		for (uint32_t i = 0; i < vector.size(); ++i)
			il2cpp_array_setref(managed, i, (Il2CppObject*)vector[i]);
		return managed;
	}

	Il2CppArray* CreateManagedTypeArray(const std::vector<Il2CppReflectionType*>& vector) {
		static auto klass_Type = il2cpp_symbols_logged::get_class_corlib("System", "Type");
		return CreateManagedArray(klass_Type, vector);
	}

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

	Il2CppObject* Activator_CreateInstance(Il2CppReflectionType* refl) {
		static auto method = il2cpp_symbols_logged::get_method_corlib("System", "Activator", "CreateInstance", 1);
		return Invoke(method, nullptr, (Il2CppObject**)&refl, "Activator_CreateInstance");
	}

	Il2CppObject* InvokeInTry(const MethodInfo* method, const Il2CppObject* instance, Il2CppObject** params, Il2CppObject** exc, const char* context) {
		__try {
			return (Il2CppObject*)il2cpp_runtime_invoke((MethodInfo*)method, (void*)instance, (void**)params, exc);
		}
		__EXCEPT("reflection::Invoke|" << context);
	}

	void InvokeVoid(const MethodInfo* method, const Il2CppObject* instance, Il2CppObject** params, const char* context) {
		Invoke(method, instance, params, context);
	}
}

namespace reflection::helper {
	std::string ToUtf8(Il2CppString* s) {
		static auto klass_Encoding = il2cpp_symbols_logged::get_class_corlib("System.Text", "Encoding");
		static auto method_Encoding_getUTF8 = il2cpp_symbols_logged::get_method_corlib("System.Text", "Encoding", "get_UTF8", 0);
		static auto method_Encoding_GetBytes = il2cpp_symbols::find_method(
			klass_Encoding,
			[](const MethodInfo* mi) {
				if (0 != strcmp(mi->name, "GetBytes")) return false;
				if (1 != il2cpp_method_get_param_count(mi)) return false;
				return 0 == strcmp("String", il2cpp_symbols::il2cpp_method_get_param_type_name(mi, 0));
			}
		);

		auto utf8 = reflection::Invoke(method_Encoding_getUTF8, nullptr, nullptr, "reflection::helper::ToUtf8|Encoding::get_UTF8");
		auto managedBytes = reflection::Invoke(method_Encoding_GetBytes, utf8, (Il2CppObject**)&s, "reflection::helper::ToUtf8|Encoding::GetBytes");

		uint8_t* raw = reinterpret_cast<uint8_t*>(il2cpp_array_addr_with_size(managedBytes, 1, 0));
		il2cpp_array_size_t length = il2cpp_array_length(managedBytes);
		std::string native(reinterpret_cast<char*>(raw), length);
		return native;
	}
}