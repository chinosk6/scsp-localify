#pragma once

#include "stdinclude.hpp"

namespace reflection {
	Il2CppReflectionType* Object_GetType(Il2CppObject* instance);
	Il2CppString* Exception_getMessage(Il2CppObject* exception);
	Il2CppObject* Invoke(MethodInfo* method, Il2CppObject* instance, Il2CppObject** args, const char* context = nullptr);
}