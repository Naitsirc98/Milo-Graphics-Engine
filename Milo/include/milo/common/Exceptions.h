#pragma once

#include <exception>
#include <stdexcept>

#define MILO_DETAILED_MESSAGE(message) std::to_string(message).append("\n\tat ").append(__FILE__).append("(").append(std::to_string(__LINE__)).append(")"))
#define MILO_EXCEPTION(message) Exception(MILO_DETAILED_MESSAGE((message))
#define MILO_RUNTIME_EXCEPTION(message) RuntimeException(MILO_DETAILED_MESSAGE((message))
#define MILO_INVALID_ARGUMENT(message) InvalidArgumentException(MILO_DETAILED_MESSAGE((message))

namespace milo {

	using Exception = std::exception;

	using RuntimeException = std::runtime_error;

	using InvalidArgumentException = std::invalid_argument;
}