#pragma once

#include "milo/common/Common.h"
#include "milo/logging/Log.h"

namespace milo {

	using Handle = void*;

	class GraphicsContext {
		friend class Graphics;
	public:
		virtual ~GraphicsContext() = default;
		[[nodiscard]] virtual Handle handle() const = 0;
	protected:
		virtual void init() = 0;
	};
}