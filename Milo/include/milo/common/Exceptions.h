#pragma once

#include <exception>
#include <stdexcept>
#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>

#define MILO_DETAILED_MESSAGE(message) milo::str(message).append("\n\tat ").append(milo::str(milo::getStackTrace(0))))
#define MILO_RUNTIME_EXCEPTION(message) RuntimeException(MILO_DETAILED_MESSAGE((message))

#define ANY_EXCEPTION ...

#define CATCH_ANY_EXCEPTION(e) catch(ANY_EXCEPTION) { try {std::rethrow_exception(std::current_exception());} catch(e)

#define INVOKE_SAFELY(func) try {func;} catch(ANY_EXCEPTION) {}

namespace milo {

	using Exception = std::exception;

	using RuntimeException = std::runtime_error;

	using InvalidArgumentException = std::invalid_argument;

	using BadAllocationException = std::bad_alloc;

	using StackTrace = boost::stacktrace::basic_stacktrace<>;

#define MILO_STACKTRACE_OFFSET 3

	inline StackTrace getStackTrace(size_t skip = 0, size_t maxDepth = 10) {
		return boost::stacktrace::stacktrace(MILO_STACKTRACE_OFFSET + skip, maxDepth);
	}

	typedef boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace> traced;

	template<typename E>
	inline void doThrow(const E& e) {
		throw boost::enable_error_info(e) << traced(getStackTrace());
	}

	template<>
	inline String str(const StackTrace& value) {
		StringStream ss;
		for(size_t i = 0;i < value.size();++i) {
			const auto& frame = value[i];
			const String& file = frame.source_file();
			ss << "  #" << i << " " << frame.name() << "\n    at " << file << ":" << frame.source_line() << "\n";
		}
		return ss.str();
	}
}