#pragma once

#include "stdinclude.hpp"

namespace reflection {
	enum BindingFlags {
		Default = 0,
		IgnoreCase = 1,
		DeclaredOnly = 2,
		Instance = 4,
		Static = 8,
		Public = 0x10,
		NonPublic = 0x20,
		FlattenHierarchy = 0x40,
		InvokeMethod = 0x100,
		CreateInstance = 0x200,
		GetField = 0x400,
		SetField = 0x800,
		GetProperty = 0x1000,
		SetProperty = 0x2000,
		PutDispProperty = 0x4000,
		PutRefDispProperty = 0x8000,
		ExactBinding = 0x10000,
		SuppressChangeType = 0x20000,
		OptionalParamBinding = 0x40000,
		IgnoreReturn = 0x1000000,
		DoNotWrapExceptions = 0x2000000
	};

	Il2CppReflectionType* typeof(const char* assemblyName, const char* namespaze, const char* klassName);
	Il2CppReflectionType* typeof_corlib(const char* klassName);
	Il2CppReflectionType* typeof_corlib(const char* namespaze, const char* klassName);

	template<typename T> Il2CppArray* CreateManagedArray(Il2CppClass* elementType, const std::vector<T>& vector);
	Il2CppArray* CreateManagedTypeArray(const std::vector<Il2CppReflectionType*>& vector);

	Il2CppReflectionType* Object_GetType(Il2CppObject* instance);

	Il2CppString* Exception_getMessage(Il2CppObject* exception);

	Il2CppObject* Activator_CreateInstance(Il2CppReflectionType* refl);


	Il2CppObject* InvokeInTry(const MethodInfo* method, Il2CppObject* instance, Il2CppObject** params, Il2CppObject** exc, const char* context);
	template<typename T = Il2CppObject*> T Invoke(const MethodInfo* method, Il2CppObject* instance, Il2CppObject** params, const char* context = nullptr) {
		Il2CppObject* exc = nullptr;
		Il2CppObject* ret = nullptr;
		ret = InvokeInTry(method, instance, params, &exc, context);
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
		return (T)ret;
	}
	void InvokeVoid(const MethodInfo* method, Il2CppObject* instance, Il2CppObject** params, const char* context = nullptr);
}

namespace reflection::helper {
	std::string ToUtf8(Il2CppString* s);
}