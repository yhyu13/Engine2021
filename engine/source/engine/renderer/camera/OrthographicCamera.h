#pragma once

#include "engine/math/Geommath.h"

namespace longmarch
{
	class OrthographicCamera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);

		const Vec3f& GetPosition() const { return m_Position; }
		void SetPosition(Vec3f& position) { m_Position = position; }

		float GetRotation() const { return m_Rotation; }
		void SetRotation(float rotation) { m_Rotation = rotation; }

		inline const Mat4 GetViewProjectionMatrix() const { return m_ProjectionMatrix * m_ViewMatrix; }

	private:
		void RecalculateViewMatrix();

	private:
		Mat4 m_ProjectionMatrix;
		Mat4 m_ViewMatrix;

		Vec3f m_Position = { 0.0f, 0.0f, 0.0f };
		float m_Rotation = 0.0f;
	};
}
