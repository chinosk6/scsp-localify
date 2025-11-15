#pragma once

#include <concepts>

typedef struct Il2CppClass Il2CppClass;
typedef uint32_t il2cpp_array_size_t;

// UnityEngine.Color
struct Color_t
{
public:
	// System.Single UnityEngine.Color::r
	float r;
	// System.Single UnityEngine.Color::g
	float g;
	// System.Single UnityEngine.Color::b
	float b;
	// System.Single UnityEngine.Color::a
	float a;
};

// UnityEngine.Vector2
struct Vector2_t
{
public:
	// System.Single UnityEngine.Vector2::x
	float x;
	// System.Single UnityEngine.Vector2::y
	float y;
};

// UnityEngine.Vector2
struct Vector2Int_t
{
public:
	int x;
	int y;
};

// UnityEngine.Vector3
struct Vector3_t
{
public:
	// System.Single UnityEngine.Vector3::x
	float x;
	// System.Single UnityEngine.Vector3::y
	float y;
	// System.Single UnityEngine.Vector3::z
	float z;
};

struct Rect_t
{
public:
	float x;
	float y;
	float width;
	float height;
};

// UnityEngine.Quaternion
struct Quaternion_t
{
public:
	float w;
	float x;
	float y;
	float z;
};

struct Resolution_t
{
public:
	int width;
	int height;
	int herz;
};

struct Matrix4x4_t
{
public:
	float m00;
	float m10;
	float m20;
	float m30;
	float m01;
	float m11;
	float m21;
	float m31;
	float m02;
	float m12;
	float m22;
	float m32;
	float m03;
	float m13;
	float m23;
	float m33;
};

// UnityEngine.TextGenerationSettings
struct TextGenerationSettings_t
{
public:
	// UnityEngine.Font UnityEngine.TextGenerationSettings::font
	void* font;
	// UnityEngine.Color UnityEngine.TextGenerationSettings::color
	Color_t color;
	// System.Int32 UnityEngine.TextGenerationSettings::fontSize
	int32_t fontSize;
	// System.Single UnityEngine.TextGenerationSettings::lineSpacing
	float lineSpacing;
	// System.Boolean UnityEngine.TextGenerationSettings::richText
	bool richText;
	// System.Single UnityEngine.TextGenerationSettings::scaleFactor
	float scaleFactor;
	// UnityEngine.FontStyle UnityEngine.TextGenerationSettings::fontStyle
	int32_t fontStyle;
	// UnityEngine.TextAnchor UnityEngine.TextGenerationSettings::textAnchor
	int32_t textAnchor;
	// System.Boolean UnityEngine.TextGenerationSettings::alignByGeometry
	bool alignByGeometry;
	// System.Boolean UnityEngine.TextGenerationSettings::resizeTextForBestFit
	bool resizeTextForBestFit;
	// System.Int32 UnityEngine.TextGenerationSettings::resizeTextMinSize
	int32_t resizeTextMinSize;
	// System.Int32 UnityEngine.TextGenerationSettings::resizeTextMaxSize
	int32_t resizeTextMaxSize;
	// System.Boolean UnityEngine.TextGenerationSettings::updateBounds
	bool updateBounds;
	// UnityEngine.VerticalWrapMode UnityEngine.TextGenerationSettings::verticalOverflow
	int32_t verticalOverflow;
	// UnityEngine.HorizontalWrapMode UnityEngine.TextGenerationSettings::horizontalOverflow
	int32_t horizontalOverflow;
	// UnityEngine.Vector2 UnityEngine.TextGenerationSettings::generationExtents
	Vector2_t  generationExtents;
	// UnityEngine.Vector2 UnityEngine.TextGenerationSettings::pivot
	Vector2_t  pivot;
	// System.Boolean UnityEngine.TextGenerationSettings::generateOutOfBounds
	bool generateOutOfBounds;
};

// not real Il2CppString class
struct Il2CppString
{
	void* Empty;
	void* WhiteChars;
	int32_t length;
	wchar_t start_char[1];
};

enum Il2CppTypeEnum
{
	IL2CPP_TYPE_END = 0x00,       /* End of List */
	IL2CPP_TYPE_VOID = 0x01,
	IL2CPP_TYPE_BOOLEAN = 0x02,
	IL2CPP_TYPE_CHAR = 0x03,
	IL2CPP_TYPE_I1 = 0x04,
	IL2CPP_TYPE_U1 = 0x05,
	IL2CPP_TYPE_I2 = 0x06,
	IL2CPP_TYPE_U2 = 0x07,
	IL2CPP_TYPE_I4 = 0x08,
	IL2CPP_TYPE_U4 = 0x09,
	IL2CPP_TYPE_I8 = 0x0a,
	IL2CPP_TYPE_U8 = 0x0b,
	IL2CPP_TYPE_R4 = 0x0c,
	IL2CPP_TYPE_R8 = 0x0d,
	IL2CPP_TYPE_STRING = 0x0e,
	IL2CPP_TYPE_PTR = 0x0f,
	IL2CPP_TYPE_BYREF = 0x10,
	IL2CPP_TYPE_VALUETYPE = 0x11,
	IL2CPP_TYPE_CLASS = 0x12,
	IL2CPP_TYPE_VAR = 0x13,
	IL2CPP_TYPE_ARRAY = 0x14,
	IL2CPP_TYPE_GENERICINST = 0x15,
	IL2CPP_TYPE_TYPEDBYREF = 0x16,
	IL2CPP_TYPE_I = 0x18,
	IL2CPP_TYPE_U = 0x19,
	IL2CPP_TYPE_FNPTR = 0x1b,
	IL2CPP_TYPE_OBJECT = 0x1c,
	IL2CPP_TYPE_SZARRAY = 0x1d,
	IL2CPP_TYPE_MVAR = 0x1e,
	IL2CPP_TYPE_CMOD_REQD = 0x1f,
	IL2CPP_TYPE_CMOD_OPT = 0x20,
	IL2CPP_TYPE_INTERNAL = 0x21,

	IL2CPP_TYPE_MODIFIER = 0x40,
	IL2CPP_TYPE_SENTINEL = 0x41,
	IL2CPP_TYPE_PINNED = 0x45,

	IL2CPP_TYPE_ENUM = 0x55
};

typedef struct Il2CppType
{
	void* dummy;
	unsigned int attrs : 16;
	Il2CppTypeEnum type : 8;
	unsigned int num_mods : 6;
	unsigned int byref : 1;
	unsigned int pinned : 1;
} Il2CppType;

struct ParameterInfo
{
	const char* name;
	int32_t position;
	uint32_t token;
	const Il2CppType* parameter_type;
};

struct MethodInfo
{
	uintptr_t methodPointer;
	uintptr_t virtualMethodPointer;
	uintptr_t invoker_method;
	const char* name;
	uintptr_t klass;
	const Il2CppType* return_type;
	const ParameterInfo* parameters;
	uintptr_t methodDefinition;
	uintptr_t genericContainer;
	uint32_t token;
	uint16_t flags;
	uint16_t iflags;
	uint16_t slot;
	uint8_t parameters_count;
	uint8_t is_generic : 1;
	uint8_t is_inflated : 1;
	uint8_t wrapper_type : 1;
	uint8_t is_marshaled_from_native : 1;
};

struct FieldInfo
{
	const char* name;
	const Il2CppType* type;
	uintptr_t parent;
	int32_t offset;
	uint32_t token;
};

typedef struct PropertyInfo
{
	Il2CppClass* parent;
	const char* name;
	const MethodInfo* get;
	const MethodInfo* set;
	uint32_t attrs;
	uint32_t token;
} PropertyInfo;

typedef struct EventInfo
{
	const char* name;
	const Il2CppType* eventType;
	Il2CppClass* parent;
	const MethodInfo* add;
	const MethodInfo* remove;
	const MethodInfo* raise;
	uint32_t token;
} EventInfo;

template <typename T>
struct TypedField
{
	FieldInfo* Field;

	constexpr FieldInfo* operator->() const noexcept
	{
		return Field;
	}
};

struct Il2CppObject
{
	union
	{
		Il2CppClass* klass;
		void* vtable;
	};
	void* monitor;
};

typedef struct VirtualInvokeData
{
	void* methodPtr;
	const MethodInfo* method;
} VirtualInvokeData;

typedef struct Il2CppClass // unity 6000.0.45f1
{
	// The following fields are always valid for a Il2CppClass structure
	const void* image;
	void* gc_desc;
	const char* name;
	const char* namespaze;
	Il2CppType byval_arg;
	Il2CppType this_arg;
	Il2CppClass* element_class;
	Il2CppClass* castClass;
	Il2CppClass* declaringType;
	Il2CppClass* parent;
	/*Il2CppGenericClass**/ void* generic_class;
	/*Il2CppMetadataTypeHandle*/ void* typeMetadataHandle; // non-NULL for Il2CppClass's constructed from type defintions
	const /*Il2CppInteropData**/ void* interopData;
	Il2CppClass* klass; // hack to pretend we are a MonoVTable. Points to ourself
	// End always valid fields

	// The following fields need initialized before access. This can be done per field or as an aggregate via a call to Class::Init
	FieldInfo* fields; // Initialized in SetupFields
	const EventInfo* events; // Initialized in SetupEvents
	const PropertyInfo* properties; // Initialized in SetupProperties
	const MethodInfo** methods; // Initialized in SetupMethods
	Il2CppClass** nestedTypes; // Initialized in SetupNestedTypes
	Il2CppClass** implementedInterfaces; // Initialized in SetupInterfaces
	/*Il2CppRuntimeInterfaceOffsetPair**/void* interfaceOffsets; // Initialized in Init
	void* static_fields; // Initialized in Init
	const /*Il2CppRGCTXData**/void* rgctx_data; // Initialized in Init
	// used for fast parent checks
	Il2CppClass** typeHierarchy; // Initialized in SetupTypeHierachy
	// End initialization required fields

	void* unity_user_data;

	/*Il2CppGCHandle*/uintptr_t initializationExceptionGCHandle;

	uint32_t cctor_started;
	uint32_t cctor_finished_or_no_cctor;
	__declspec(align(8)) size_t cctor_thread;

	// Remaining fields are always valid except where noted
	/*Il2CppMetadataGenericContainerHandle*/uint32_t genericContainerHandle;
	uint32_t instance_size; // valid when size_inited is true
	uint32_t stack_slot_size; // valid when size_inited is true
	uint32_t actualSize;
	uint32_t element_size;
	int32_t native_size;
	uint32_t static_fields_size;
	uint32_t thread_static_fields_size;
	int32_t thread_static_fields_offset;
	uint32_t flags;
	uint32_t token;

	uint16_t method_count; // lazily calculated for arrays, i.e. when rank > 0
	uint16_t property_count;
	uint16_t field_count;
	uint16_t event_count;
	uint16_t nested_type_count;
	uint16_t vtable_count; // lazily calculated for arrays, i.e. when rank > 0
	uint16_t interfaces_count;
	uint16_t interface_offsets_count; // lazily calculated for arrays, i.e. when rank > 0

	uint8_t typeHierarchyDepth; // Initialized in SetupTypeHierachy
	uint8_t genericRecursionDepth;
	uint8_t rank;
	uint8_t minimumAlignment; // Alignment of this type
	uint8_t packingSize;

	// this is critical for performance of Class::InitFromCodegen. Equals to initialized && !initializationExceptionGCHandle at all times.
	// Use Class::PublishInitialized to update
	uint8_t initialized_and_no_error : 1;

	uint8_t initialized : 1;
	uint8_t enumtype : 1;
	uint8_t nullabletype : 1;
	uint8_t is_generic : 1;
	uint8_t has_references : 1; // valid when size_inited is true
	uint8_t init_pending : 1;
	uint8_t size_init_pending : 1;
	uint8_t size_inited : 1;
	uint8_t has_finalize : 1;
	uint8_t has_cctor : 1;
	uint8_t is_blittable : 1;
	uint8_t is_import_or_windows_runtime : 1;
	uint8_t is_vtable_initialized : 1;
	uint8_t is_byref_like : 1;
	VirtualInvokeData vtable[0];
} Il2CppClass;


typedef int32_t il2cpp_array_lower_bound_t;
#define IL2CPP_ARRAY_MAX_INDEX ((int32_t) 0x7fffffff)
#define IL2CPP_ARRAY_MAX_SIZE  ((uint32_t) 0xffffffff)

typedef struct Il2CppArrayBounds
{
	il2cpp_array_size_t length;
	il2cpp_array_lower_bound_t lower_bound;
} Il2CppArrayBounds;

typedef struct Il2CppArray : public Il2CppObject
{
	/* bounds is NULL for szarrays */
	Il2CppArrayBounds* bounds;
	/* total number of elements of the array */
	il2cpp_array_size_t max_length;
} Il2CppArray;

typedef struct Il2CppArraySize : public Il2CppArray
{
	__declspec(align(8)) void* vector[0];
} Il2CppArraySize;


struct Il2CppReflectionType : public Il2CppObject
{
	const Il2CppType* type;
};

typedef struct Il2CppReflectionMethod : public Il2CppObject
{
	const MethodInfo* method;
	Il2CppString* name;
	Il2CppReflectionType* reftype;
} Il2CppReflectionMethod;


typedef struct __declspec(align(8)) Il2CppDelegate : public Il2CppObject {
	intptr_t method_ptr;
	intptr_t invoke_impl;
	Il2CppObject* m_target;
	intptr_t method;
	intptr_t delegate_trampoline;
	intptr_t extra_arg;
	intptr_t method_code;
	intptr_t interp_method;
	intptr_t interp_invoke_impl;
	Il2CppReflectionMethod* method_info;
	Il2CppReflectionMethod* original_method_info;
	void* data;
	bool method_is_virtual;
};


static const size_t kIl2CppSizeOfArray = (offsetof(Il2CppArraySize, vector));

#if _MSC_VER
typedef wchar_t Il2CppChar;
#elif __has_feature(cxx_unicode_literals)
typedef char16_t Il2CppChar;
#else
typedef uint16_t Il2CppChar;
#endif

// function types
typedef Il2CppString* (*il2cpp_string_new_utf16_t)(const wchar_t* str, unsigned int len);
typedef Il2CppString* (*il2cpp_string_new_t)(const char* str);
typedef void* (*il2cpp_domain_get_t)();
typedef void* (*il2cpp_domain_assembly_open_t)(void* domain, const char* name);
typedef void* (*il2cpp_assembly_get_image_t)(void* assembly);
typedef Il2CppClass* (*il2cpp_class_from_il2cpp_type_t)(const Il2CppType* type);
typedef void* (*il2cpp_class_from_name_t)(void* image, const char* namespaze, const char* name);
typedef MethodInfo* (*il2cpp_class_get_methods_t)(void* klass, void** iter);
typedef MethodInfo* (*il2cpp_class_get_method_from_name_t)(void* klass, const char* name, int argsCount);
typedef uint32_t(*il2cpp_method_get_param_count_t)(const MethodInfo* method);
typedef Il2CppType* (*il2cpp_method_get_param_t)(const MethodInfo* method, uint32_t index);
typedef Il2CppReflectionMethod* (*il2cpp_method_get_object_t)(const MethodInfo* method, Il2CppClass* refclass);
typedef const Il2CppType* (*il2cpp_method_get_return_type_t)(const MethodInfo* method);
typedef void* (*il2cpp_object_new_t)(void* klass);
typedef void (*il2cpp_runtime_object_init_t)(Il2CppObject* obj);
typedef void* (*il2cpp_resolve_icall_t)(const char* name);
typedef void* (*il2cpp_array_new_t)(void* klass, uintptr_t count);
typedef void* (*il2cpp_thread_attach_t)(void* domain);
typedef void (*il2cpp_thread_detach_t)(void* thread);
typedef FieldInfo* (*il2cpp_class_get_field_from_name_t)(void* klass, const char* name);
typedef bool (*il2cpp_class_is_assignable_from_t)(void* klass, void* oklass);
typedef void (*il2cpp_class_for_each_t)(void(*klassReportFunc)(void* klass, void* userData), void* userData);
typedef void* (*il2cpp_class_get_nested_types_t)(void* klass, void** iter);
typedef void* (*il2cpp_class_get_type_t)(void* klass);
typedef Il2CppReflectionType* (*il2cpp_type_get_object_t)(const void* type);
typedef void* (*il2cpp_gchandle_new_t)(void* obj, bool pinned);
typedef void (*il2cpp_gchandle_free_t)(void* gchandle);
typedef void* (*il2cpp_gchandle_get_target_t)(void* gchandle);
typedef Il2CppType* (*il2cpp_reflection_type_get_type_t)(Il2CppReflectionType* reflType);
typedef void* (*il2cpp_class_from_type_t)(const Il2CppType* type);
typedef void (*il2cpp_runtime_class_init_t)(void* klass);
typedef void* (*il2cpp_runtime_invoke_t)(MethodInfo* method, void* obj, void** params, Il2CppObject** exc);
typedef void* (*il2cpp_class_get_static_field_data_t)(void* klass);
typedef void (*il2cpp_field_get_value_t)(void* obj, void* field, void* value);
typedef void* (*il2cpp_field_get_value_object_t)(void* field, void* obj);
typedef void* (*il2cpp_class_from_system_type_t)(Il2CppReflectionType* type);
typedef void* (*il2cpp_get_corlib_t)();
typedef const char* (*il2cpp_class_get_namespace_t)(void* klass);
typedef const char* (*il2cpp_class_get_name_t)(void* klass);
typedef void (*il2cpp_field_set_value_t)(void* obj, void* field, void* value);
typedef void (*il2cpp_field_set_value_object_t)(void* obj, void* field, void* valueObj);
typedef void (*il2cpp_field_static_set_value_t)(void* field, void* value);
typedef Il2CppObject* (*il2cpp_value_box_t)(Il2CppClass* klass, void* data);
typedef void* (*il2cpp_object_unbox_t)(Il2CppObject* obj);
typedef uint32_t(*il2cpp_array_length_t)(void* arr);
typedef void* (*il2cpp_class_get_parent_t)(void* klass);
typedef const char* (*il2cpp_method_get_name_t)(const MethodInfo* method);
typedef void* (*il2cpp_method_get_class_t)(const MethodInfo* method);
typedef Il2CppClass* (*il2cpp_object_get_class_t)(Il2CppObject* instance);
typedef Il2CppChar* (*il2cpp_string_chars_t)(Il2CppString* str);
typedef int32_t(*il2cpp_string_length_t) (Il2CppString* str);
typedef Il2CppClass* (*il2cpp_type_get_class_or_element_class_t)(const Il2CppType* type);
typedef char* (*il2cpp_type_get_name_t)(const Il2CppType* type);

// function defines
extern il2cpp_string_new_utf16_t il2cpp_string_new_utf16;
extern il2cpp_string_new_t il2cpp_string_new;
extern il2cpp_domain_get_t il2cpp_domain_get;
extern il2cpp_domain_assembly_open_t il2cpp_domain_assembly_open;
extern il2cpp_assembly_get_image_t il2cpp_assembly_get_image;
extern il2cpp_class_from_il2cpp_type_t il2cpp_class_from_il2cpp_type;
extern il2cpp_class_from_name_t il2cpp_class_from_name;
extern il2cpp_class_get_methods_t il2cpp_class_get_methods;
extern il2cpp_class_get_method_from_name_t il2cpp_class_get_method_from_name;
extern il2cpp_method_get_param_count_t il2cpp_method_get_param_count;
extern il2cpp_method_get_param_t il2cpp_method_get_param;
extern il2cpp_method_get_object_t il2cpp_method_get_object;
extern il2cpp_method_get_return_type_t il2cpp_method_get_return_type;
extern il2cpp_object_new_t il2cpp_object_new;
extern il2cpp_runtime_object_init_t il2cpp_runtime_object_init;
extern il2cpp_resolve_icall_t il2cpp_resolve_icall;
extern il2cpp_array_new_t il2cpp_array_new;
extern il2cpp_thread_attach_t il2cpp_thread_attach;
extern il2cpp_thread_detach_t il2cpp_thread_detach;
extern il2cpp_class_get_field_from_name_t il2cpp_class_get_field_from_name;
extern il2cpp_class_is_assignable_from_t il2cpp_class_is_assignable_from;
extern il2cpp_class_for_each_t il2cpp_class_for_each;
extern il2cpp_class_get_nested_types_t il2cpp_class_get_nested_types;
extern il2cpp_class_get_type_t il2cpp_class_get_type;
extern il2cpp_type_get_object_t il2cpp_type_get_object;
extern il2cpp_gchandle_new_t il2cpp_gchandle_new;
extern il2cpp_gchandle_free_t il2cpp_gchandle_free;
extern il2cpp_gchandle_get_target_t il2cpp_gchandle_get_target;
extern il2cpp_reflection_type_get_type_t il2cpp_reflection_type_get_type;
extern il2cpp_class_from_type_t il2cpp_class_from_type;
extern il2cpp_runtime_class_init_t il2cpp_runtime_class_init;
extern il2cpp_runtime_invoke_t il2cpp_runtime_invoke;
extern il2cpp_class_get_static_field_data_t il2cpp_class_get_static_field_data;
extern il2cpp_field_get_value_t il2cpp_field_get_value;
extern il2cpp_field_get_value_object_t il2cpp_field_get_value_object;
extern il2cpp_class_from_system_type_t il2cpp_class_from_system_type;
extern il2cpp_get_corlib_t il2cpp_get_corlib;
extern il2cpp_class_get_namespace_t il2cpp_class_get_namespace;
extern il2cpp_class_get_name_t il2cpp_class_get_name;
extern il2cpp_field_set_value_t il2cpp_field_set_value;
extern il2cpp_field_set_value_object_t il2cpp_field_set_value_object;
extern il2cpp_field_static_set_value_t il2cpp_field_static_set_value;
extern il2cpp_value_box_t il2cpp_value_box;
extern il2cpp_object_unbox_t il2cpp_object_unbox;
extern il2cpp_array_length_t il2cpp_array_length;
extern il2cpp_class_get_parent_t il2cpp_class_get_parent;
extern il2cpp_method_get_name_t il2cpp_method_get_name;
extern il2cpp_method_get_class_t il2cpp_method_get_class;
extern il2cpp_object_get_class_t il2cpp_object_get_class;
extern il2cpp_string_chars_t il2cpp_string_chars;
extern il2cpp_string_length_t il2cpp_string_length;
extern il2cpp_type_get_class_or_element_class_t il2cpp_type_get_class_or_element_class;
extern il2cpp_type_get_name_t il2cpp_type_get_name;

char* il2cpp_array_addr_with_size(void* arr, int32_t size, uintptr_t idx);

// array macro
#define il2cpp_array_addr(array, type, index) ((type*)(void*) il2cpp_array_addr_with_size (array, sizeof (type), index))

#define il2cpp_array_setref(array, index, value)  \
	do {    \
		void* *__p = (void* *) il2cpp_array_addr ((array), void*, (index)); \
		 *__p = (value);    \
	} while (0)

namespace il2cpp_symbols
{
	void init(HMODULE game_module);
	uintptr_t get_method_pointer(const char* assemblyName, const char* namespaze,
		const char* klassName, const char* name, int argsCount);

	void* get_class(const char* assemblyName, const char* namespaze, const char* klassName);

	void* find_nested_class(void* klass, std::predicate<void*> auto&& predicate)
	{
		void* iter{};
		while (const auto curNestedClass = il2cpp_class_get_nested_types(klass, &iter))
		{
			if (static_cast<decltype(predicate)>(predicate)(curNestedClass))
			{
				return curNestedClass;
			}
		}

		return nullptr;
	}

	void* find_nested_class_from_name(void* klass, const char* name);

	MethodInfo* get_method(const char* assemblyName, const char* namespaze,
		const char* klassName, const char* name, int argsCount);

	const MethodInfo* find_method(const char* assemblyName, const char* namespaze,
		const char* klassName, std::function<bool(const MethodInfo*)> predict);

	const MethodInfo* find_method(Il2CppClass* klass, std::function<bool(const MethodInfo*)> predict);

	FieldInfo* get_field(const char* assemblyName, const char* namespaze,
		const char* klassName, const char* name);

	template <typename T>
	TypedField<T> get_field(const char* assemblyName, const char* namespaze,
		const char* klassName, const char* name)
	{
		return { get_field(assemblyName, namespaze, klassName, name) };
	}

	void* get_class_from_instance(const void* instance);

	template <typename T = void*> requires std::is_trivial_v<T>
	T read_field(const void* ptr, const FieldInfo* field)
	{
		T result;
		const auto fieldPtr = static_cast<const std::byte*>(ptr) + field->offset;
		std::memcpy(std::addressof(result), fieldPtr, sizeof(T));
		return result;
	}

	template <typename T>
	T read_field(const void* ptr, TypedField<T> field)
	{
		return read_field<T>(ptr, field.Field);
	}

	template <typename T> requires std::is_trivial_v<T>
	void write_field(void* ptr, const FieldInfo* field, const T& value)
	{
		const auto fieldPtr = static_cast<std::byte*>(ptr) + field->offset;
		std::memcpy(fieldPtr, std::addressof(value), sizeof(T));
	}

	template <typename T, typename U>
	void write_field(void* ptr, TypedField<T> field, U&& value)
	{
		write_field<T>(ptr, field.Field, static_cast<T>(std::forward<U>(value)));
	}

	template <typename T = void*>
	void iterate_list(const void* list, std::invocable<int32_t, T> auto&& receiver)
	{
		const auto listClass = get_class_from_instance(list);
		const auto getItemMethod = reinterpret_cast<T(*)(const void*, int32_t)>(il2cpp_class_get_method_from_name(listClass, "get_Item", 1)->methodPointer);
		const auto getCountMethod = reinterpret_cast<int32_t(*)(const void*)>(il2cpp_class_get_method_from_name(listClass, "get_Count", 0)->methodPointer);

		const auto count = getCountMethod(list);
		for (int32_t i = 0; i < count; ++i)
		{
			static_cast<decltype(receiver)>(receiver)(i, getItemMethod(list, i));
		}
	}

	template <typename T = void*>
	void iterate_IEnumerable(const void* obj, std::invocable<T> auto&& receiver)
	{
		const auto klass = get_class_from_instance(obj);
		const auto getEnumeratorMethod = reinterpret_cast<void* (*)(const void*)>(il2cpp_class_get_method_from_name(klass, "GetEnumerator", 0)->methodPointer);
		const auto enumerator = getEnumeratorMethod(obj);
		const auto enumeratorClass = get_class_from_instance(enumerator);
		const auto getCurrentMethod = reinterpret_cast<T(*)(void*)>(il2cpp_class_get_method_from_name(enumeratorClass, "get_Current", 0)->methodPointer);
		const auto moveNextMethod = reinterpret_cast<bool(*)(void*)>(il2cpp_class_get_method_from_name(enumeratorClass, "MoveNext", 0)->methodPointer);

		while (moveNextMethod(enumerator))
		{
			static_cast<decltype(receiver)>(receiver)(getCurrentMethod(enumerator));
		}
	}

	Il2CppString* NewWStr(std::wstring_view str);
	void* get_system_class_from_reflection_type_str(const char* typeStr, const char* assemblyName = "mscorlib");

	void* array_get_value(void* array, int index);

	void array_set_value(void* array, void* value, int index);

	const char* il2cpp_method_get_param_type_name(const MethodInfo* mi, uint32_t index);

	template <typename T> T unbox(Il2CppObject* boxed) { return *(T*)il2cpp_object_unbox(boxed); }
}

namespace il2cpp_symbols_logged {
	void* il2cpp_resolve_icall(const char* name);
	void* get_class(const char* assemblyName, const char* namespaze, const char* klassName);
	Il2CppClass* get_class_corlib(const char* namespaze, const char* klassName);
	FieldInfo* il2cpp_class_get_field_from_name(void* klass, const char* name);
	MethodInfo* get_method(const char* assemblyName, const char* namespaze, const char* klassName, const char* name, int argsCount);
	MethodInfo* get_method_corlib(const char* namespaze, const char* klassName, const char* name, int argsCount);
	uintptr_t get_method_pointer(const char* assemblyName, const char* namespaze, const char* klassName, const char* name, int argsCount);
}