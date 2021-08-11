#pragma once

#include "milo/common/Common.h"

namespace milo {

	using Handle = void*;

	class GraphicsContext {
	public:
		virtual ~GraphicsContext() = default;
		[[nodiscard]] virtual Handle handle() const = 0;
	protected:
		virtual void init() = 0;
	};
}