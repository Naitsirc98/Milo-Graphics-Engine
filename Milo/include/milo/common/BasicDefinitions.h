#pragma once

#ifdef _BUILD_DYNAMIC_LIB
#define MILO_ENUM enum __declspec(dllexport)
#define MILO_ENUM_CLASS enum class __declspec(dllexport)
#define MILO_CLASS class __declspec(dllexport)
#define MILO_STRUCT struct __declspec(dllexport)
#else
#define MILO_ENUM enum
#define MILO_ENUM_CLASS enum class
#define MILO_CLASS class
#define MILO_STRUCT struct
#endif

#define INHERITS : public

#define MILO_SUCCESS 0
#define MILO_FAILURE 1
#define MILO_ENGINE_ALREADY_LAUNCHED 2

#define DELETE_PTR(ptr) {delete ptr; ptr = nullptr;}
#define DELETE_ARRAY(arr) {delete arr; arr = nullptr;}

namespace milo {}