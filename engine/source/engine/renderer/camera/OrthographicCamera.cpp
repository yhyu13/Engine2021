#include "engine-precompiled-header.h"
#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>


namespace longmarch {
	
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: 
		m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), 
		m_ViewMatrix(1.0f)
	{
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top);
	}

	 void OrthographicCamera::RecalculateViewMatrix()
	 {
		 Mat4 transform = Geommath::ToTranslateMatrix(m_Position) * glm::rotate(Mat4(1.0f), (m_Rotation * DEG2RAD), Vec3f(0,0,1));
		 m_ViewMatrix = Geommath::SmartInverse(transform);
	 }
}
