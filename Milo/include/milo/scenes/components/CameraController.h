#pragma once

#include "Camera.h"
#include "NativeScript.h"

namespace milo {

	struct Transform;
	class Camera;

	class CameraController : public NativeScript {
	private:
		float m_LogPosLastTime = 0;
	protected:
		void onLateUpdate(EntityId entityId) override;
	private:
		void handleMovement(Transform& transform, Camera& camera);
		void handleDirection(Transform& transform, Camera& camera);
	};
}