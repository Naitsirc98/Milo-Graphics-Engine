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
	using AtomicByte = std::atomic_int8_t;
	using AtomicUByte = std::atomic_uint8_t;
	using AtomicShort = std::atomic_int16_t;
	using AtomicUShort = std::atomic_uint16_t;
	using AtomicInt = std::atomic_int32_t;
	using AtomicUInt = std::atomic_uint32_t;
	using AtomicLong = std::atomic_llong;
	using AtomicULong = std::atomic_ullong;
	using AtomicFloat = std::atomic<float>;
}
