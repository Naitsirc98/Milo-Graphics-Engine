#pragma once

#include <thread>
#include <mutex>

namespace milo {

	using Thread = std::thread;

	using namespace std::this_thread;

	using Mutex = std::mutex;

	template<typename T>
	using Atomic = std::atomic<T>;

	using AtomicBool = std::atomic_bool;
	using AtomicInt = std::atomic_int32_t;
	using AtomicLong = std::atomic_llong;
	using AtomicFloat = std::atomic<float>;
}
