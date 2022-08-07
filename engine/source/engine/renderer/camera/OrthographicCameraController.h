#pragma once
#include "OrthographicCamera.h"
#include "engine/core/EngineCore.h"

namespace longmarch
{
	class ENGINE_API OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(double ts);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }
		float GetZoomLevel() const { return m_ZoomLevel; }
		float GetCameraMoveSpeed() const { return m_CameraMoveSpeed; }
		float GetAspectRatio() const { return m_AspectRatio; }
		void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; }
		void SetPosition(const Vec3f& pos) { m_CameraPosition = pos; }
		const Vec3f& GetPosition() const { return m_CameraPosition; }
	private:
		OrthographicCamera m_Camera;
		Vec3f m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;
		float m_CameraMoveSpeed = 5.0f;
		float m_CameraRotationSpeed = PI;
		float m_ZoomLevel = 1.0f;
		float m_AspectRatio = 1.0f;
		bool m_Rotation = false;
	};
}
