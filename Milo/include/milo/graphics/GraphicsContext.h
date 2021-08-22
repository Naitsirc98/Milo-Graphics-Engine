#pragma once

#include "milo/common/Common.h"
#include "milo/logging/Log.h"
#include "milo/graphics/rendering/GraphicsPresenter.h"

namespace milo {

	class Window;

	class GraphicsContext {
		friend class Graphics;
	public:
		virtual ~GraphicsContext() = default;
		 virtual Handle handle() const = 0;
		 virtual GraphicsPresenter* presenter() const = 0;
	protected:
		virtual void init(Window* mainWindow) = 0;
	};
}