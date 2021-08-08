#pragma once

#define MILO_SUCCESS 0
#define MILO_FAILURE 1
#define MILO_ENGINE_ALREADY_LAUNCHED 2

#define DELETE_PTR(ptr) {delete ptr; ptr = nullptr;}
#define DELETE_ARRAY(arr) {delete arr; arr = nullptr;}

namespace milo {}