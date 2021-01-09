#include "engine-precompiled-header.h"
#include "OrthographicCameraController.h"

namespace longmarch {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		:m_AspectRatio(aspectRatio), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio* m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel), m_Rotation(rotation)
	{

	}

	void OrthographicCameraController::OnUpdate(double ts)
	{
		float left = -m_AspectRatio * m_ZoomLevel + m_CameraPosition.x;
		float right = m_AspectRatio * m_ZoomLevel + m_CameraPosition.x;
		float button = -m_ZoomLevel + m_CameraPosition.y;
		float top = m_ZoomLevel + m_CameraPosition.y;
		m_Camera.SetProjection(left, right, button, top);
	}
}
