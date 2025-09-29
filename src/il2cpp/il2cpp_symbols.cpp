#include <stdinclude.hpp>

il2cpp_string_new_utf16_t il2cpp_string_new_utf16;
il2cpp_string_new_t il2cpp_string_new;
il2cpp_domain_get_t il2cpp_domain_get;
il2cpp_domain_assembly_open_t il2cpp_domain_assembly_open;
il2cpp_assembly_get_image_t il2cpp_assembly_get_image;
il2cpp_class_from_name_t il2cpp_class_from_name;
il2cpp_class_get_methods_t il2cpp_class_get_methods;
il2cpp_class_get_method_from_name_t il2cpp_class_get_method_from_name;
il2cpp_method_get_param_t il2cpp_method_get_param;
il2cpp_object_new_t il2cpp_object_new;
il2cpp_resolve_icall_t il2cpp_resolve_icall;
il2cpp_array_new_t il2cpp_array_new;
il2cpp_thread_attach_t il2cpp_thread_attach;
il2cpp_thread_detach_t il2cpp_thread_detach;
il2cpp_class_get_field_from_name_t il2cpp_class_get_field_from_name;
il2cpp_class_is_assignable_from_t il2cpp_class_is_assignable_from;
il2cpp_class_for_each_t il2cpp_class_for_each;
il2cpp_class_get_nested_types_t il2cpp_class_get_nested_types;
il2cpp_class_get_type_t il2cpp_class_get_type;
il2cpp_type_get_object_t il2cpp_type_get_object;
il2cpp_gchandle_new_t il2cpp_gchandle_new;
il2cpp_gchandle_free_t il2cpp_gchandle_free;
il2cpp_gchandle_get_target_t il2cpp_gchandle_get_target;
il2cpp_class_from_type_t il2cpp_class_from_type;
il2cpp_runtime_class_init_t il2cpp_runtime_class_init;
il2cpp_runtime_invoke_t il2cpp_runtime_invoke;
il2cpp_class_get_static_field_data_t il2cpp_class_get_static_field_data;
il2cpp_field_get_value_t il2cpp_field_get_value;
il2cpp_field_get_value_object_t il2cpp_field_get_value_object;
il2cpp_class_from_system_type_t il2cpp_class_from_system_type;
il2cpp_get_corlib_t il2cpp_get_corlib;
il2cpp_class_get_name_t il2cpp_class_get_name;
il2cpp_field_set_value_t il2cpp_field_set_value;
il2cpp_field_set_value_object_t il2cpp_field_set_value_object;
il2cpp_field_static_set_value_t il2cpp_field_static_set_value;
il2cpp_value_box_t il2cpp_value_box;
il2cpp_object_unbox_t il2cpp_object_unbox;
il2cpp_array_length_t il2cpp_array_length;
il2cpp_class_get_parent_t il2cpp_class_get_parent;
il2cpp_method_get_name_t il2cpp_method_get_name;
il2cpp_method_get_class_t il2cpp_method_get_class;

char* il2cpp_array_addr_with_size(void* array, int32_t size, uintptr_t idx)
{
	return ((char*)array) + kIl2CppSizeOfArray + size * idx;
}

namespace il2cpp_symbols
{
#define RESOLVE_IMPORT(name) name = reinterpret_cast<name##_t>(GetProcAddress(game_module, #name))

	void* il2cpp_domain = nullptr;

	void init(HMODULE game_module)
	{
		RESOLVE_IMPORT(il2cpp_string_new_utf16);
		RESOLVE_IMPORT(il2cpp_string_new);
		RESOLVE_IMPORT(il2cpp_domain_get);
		RESOLVE_IMPORT(il2cpp_domain_assembly_open);
		RESOLVE_IMPORT(il2cpp_assembly_get_image);
		RESOLVE_IMPORT(il2cpp_class_from_name);
		RESOLVE_IMPORT(il2cpp_class_get_methods);
		RESOLVE_IMPORT(il2cpp_class_get_method_from_name);
		RESOLVE_IMPORT(il2cpp_method_get_param);
		RESOLVE_IMPORT(il2cpp_object_new);
		RESOLVE_IMPORT(il2cpp_resolve_icall);
		RESOLVE_IMPORT(il2cpp_array_new);
		RESOLVE_IMPORT(il2cpp_thread_attach);
		RESOLVE_IMPORT(il2cpp_thread_detach);
		RESOLVE_IMPORT(il2cpp_class_get_field_from_name);
		RESOLVE_IMPORT(il2cpp_class_is_assignable_from);
		RESOLVE_IMPORT(il2cpp_class_for_each);
		RESOLVE_IMPORT(il2cpp_class_get_nested_types);
		RESOLVE_IMPORT(il2cpp_class_get_type);
		RESOLVE_IMPORT(il2cpp_type_get_object);
		RESOLVE_IMPORT(il2cpp_gchandle_new);
		RESOLVE_IMPORT(il2cpp_gchandle_free);
		RESOLVE_IMPORT(il2cpp_gchandle_get_target);
		RESOLVE_IMPORT(il2cpp_class_from_type);
		RESOLVE_IMPORT(il2cpp_runtime_class_init);
		RESOLVE_IMPORT(il2cpp_runtime_invoke);
		RESOLVE_IMPORT(il2cpp_class_get_static_field_data);
		RESOLVE_IMPORT(il2cpp_field_get_value);
		RESOLVE_IMPORT(il2cpp_field_get_value_object);
		RESOLVE_IMPORT(il2cpp_class_from_system_type);
		RESOLVE_IMPORT(il2cpp_get_corlib);
		RESOLVE_IMPORT(il2cpp_class_get_name);
		RESOLVE_IMPORT(il2cpp_field_set_value);
		RESOLVE_IMPORT(il2cpp_field_set_value_object);
		RESOLVE_IMPORT(il2cpp_field_static_set_value);
		RESOLVE_IMPORT(il2cpp_value_box);
		RESOLVE_IMPORT(il2cpp_object_unbox);
		RESOLVE_IMPORT(il2cpp_array_length);
		RESOLVE_IMPORT(il2cpp_class_get_parent);
		RESOLVE_IMPORT(il2cpp_method_get_name);
		RESOLVE_IMPORT(il2cpp_method_get_class);

		il2cpp_domain = il2cpp_domain_get();
	}

	void* get_class(const char* assemblyName, const char* namespaze, const char* klassName)
	{
		auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
		auto image = il2cpp_assembly_get_image(assembly);
		return il2cpp_class_from_name(image, namespaze, klassName);
	}

	uintptr_t get_method_pointer(const char* assemblyName, const char* namespaze,
		const char* klassName, const char* name, int argsCount)
	{
		auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
		if (!assembly) {
			printf("\nError: invalid assembly: %s\n", assemblyName);
			return NULL;
		}
		auto image = il2cpp_assembly_get_image(assembly);
		auto klass = il2cpp_class_from_name(image, namespaze, klassName);
		if (!klass) {
			printf("\nError: invalid klass: %s::%s\n", namespaze, klassName);
			return NULL;
		}
		auto ret = il2cpp_class_get_method_from_name(klass, name, argsCount);
		if (ret) {
			return ret->methodPointer;
		}
		else {
			printf("\nError: method not found: %s - %s::%s.%s (%d)\n\n", assemblyName, namespaze, klassName, name, argsCount);
			return NULL;
		}

	}

	void* find_nested_class_from_name(void* klass, const char* name)
	{
		return find_nested_class(klass, [name = std::string_view(name)](void* nestedClass) {
			return static_cast<Il2CppClassHead*>(nestedClass)->name == name;
			});
	}

	MethodInfo* get_method(const char* assemblyName, const char* namespaze,
		const char* klassName, const char* name, int argsCount)
	{
		auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
		auto image = il2cpp_assembly_get_image(assembly);
		auto klass = il2cpp_class_from_name(image, namespaze, klassName);

		return il2cpp_class_get_method_from_name(klass, name, argsCount);
	}

	uintptr_t find_method(const char* assemblyName, const char* namespaze,
		const char* klassName, std::function<bool(const MethodInfo*)> predict)
	{
		auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
		auto image = il2cpp_assembly_get_image(assembly);
		auto klass = il2cpp_class_from_name(image, namespaze, klassName);

		void* iter = nullptr;
		while (const MethodInfo* method = il2cpp_class_get_methods(klass, &iter))
		{
			if (predict(method))
				return method->methodPointer;
		}

		return 0;
	}

	FieldInfo* get_field(const char* assemblyName, const char* namespaze,
		const char* klassName, const char* name)
	{
		const auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
		const auto image = il2cpp_assembly_get_image(assembly);
		const auto klass = il2cpp_class_from_name(image, namespaze, klassName);

		return il2cpp_class_get_field_from_name(klass, name);
	}

	void* get_class_from_instance(const void* instance)
	{
		return *static_cast<void* const*>(std::assume_aligned<alignof(void*)>(instance));
	}

	Il2CppString* NewWStr(std::wstring_view str)
	{
		return il2cpp_string_new_utf16(str.data(), str.size());
	}

	void* get_system_class_from_reflection_type_str(const char* typeStr, const char* assemblyName) {
		static auto assemblyLoad = reinterpret_cast<void* (*)(Il2CppString*)>(
			il2cpp_symbols::get_method_pointer("mscorlib.dll", "System.Reflection",
				"Assembly", "Load", 1)
			);
		static auto assemblyGetType = reinterpret_cast<Il2CppReflectionType * (*)(void*, Il2CppString*)>(
			il2cpp_symbols::get_method_pointer("mscorlib.dll", "System.Reflection",
				"Assembly", "GetType", 1)
			);

		static auto reflectionAssembly = assemblyLoad(il2cpp_string_new(assemblyName));
		auto reflectionType = assemblyGetType(reflectionAssembly, il2cpp_string_new(typeStr));
		return il2cpp_class_from_system_type(reflectionType);
	}

	void* array_get_value(void* array, int index) {
		static auto klass_System_Array = il2cpp_class_from_name(il2cpp_get_corlib(), "System", "Array");
		static auto mtd_Array_GetValue = il2cpp_class_get_method_from_name(klass_System_Array, "GetValue", 1);
		static auto func_Array_GetValue = reinterpret_cast<void* (*)(void* _this, int index, void* mtd)>(mtd_Array_GetValue->methodPointer);
		return func_Array_GetValue(array, index, mtd_Array_GetValue);
	}

	void array_set_value(void* array, void* value, int index) {
		static auto klass_System_Array = il2cpp_class_from_name(il2cpp_get_corlib(), "System", "Array");
		static auto mtd_Array_SetValue = il2cpp_class_get_method_from_name(klass_System_Array, "SetValue", 2);
		static auto func_Array_SetValue = reinterpret_cast<void* (*)(void* _this, void* value, int index, void* mtd)>(mtd_Array_SetValue->methodPointer);
		func_Array_SetValue(array, value, index, mtd_Array_SetValue);
	}
}


namespace il2cpp_symbols_logged {

	void* il2cpp_resolve_icall(const char* name) {
		auto p = ::il2cpp_resolve_icall(name);
		if (p == nullptr) {
			std::cout << "[ERROR] `il2cpp_resolve_icall(\"" << name << "\")` returned NULL." << std::endl;
		}
		return p;
	}

	void* get_class(const char* assemblyName, const char* namespaze, const char* klassName) {
		auto p = il2cpp_symbols::get_class(assemblyName, namespaze, klassName);
		if (p == nullptr) {
			std::cout << "[ERROR] `il2cpp_symbols::get_class(`" << assemblyName << ", " << namespaze << ", " << klassName << ")` returned NULL." << std::endl;
		}
		return p;
	}

	FieldInfo* il2cpp_class_get_field_from_name(void* klass, const char* name) {
		auto p = ::il2cpp_class_get_field_from_name(klass, name);
		if (p == nullptr) {
			std::cout << "[ERROR] `il2cpp_class_get_field_from_name(" << klass << ", " << name << ")` returned NULL." << std::endl;
		}
		return p;
	}

	MethodInfo* get_method(const char* assemblyName, const char* namespaze, const char* klassName, const char* name, int argsCount) {
		auto p = il2cpp_symbols::get_method(assemblyName, namespaze, klassName, name, argsCount);
		if (p == 0) {
			std::cout << "[ERROR] `il2cpp_symbols::get_method(" << assemblyName << ", " << namespaze << ", " << klassName << ", " << name << ", " << argsCount << ")` returned NULL." << std::endl;
		}
		return p;
	}

	uintptr_t get_method_pointer(const char* assemblyName, const char* namespaze, const char* klassName, const char* name, int argsCount) {
		auto p = il2cpp_symbols::get_method_pointer(assemblyName, namespaze, klassName, name, argsCount);
		if (p == 0) {
			std::cout << "[ERROR] `il2cpp_symbols::get_method_pointer(" << assemblyName << ", " << namespaze << ", " << klassName << ", " << name << ", " << argsCount << ")` returned NULL." << std::endl;
		}
		return p;
	}
}