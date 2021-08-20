#pragma once

#include "milo/common/Common.h"
#include "milo/graphics/rendering/GraphicsPresenter.h"
#include "milo/logging/Log.h"

namespace milo {

	class Window;

	using Handle = void*;

	class GraphicsContext {
		friend class Graphics;
	public:
		virtual ~GraphicsContext() = default;
		[[nodiscard]] virtual Handle handle() const = 0;
		virtual GraphicsPresenter& presenter() const = 0;
	protected:
		virtual void init(Window& mainWindow) = 0;
	};
}