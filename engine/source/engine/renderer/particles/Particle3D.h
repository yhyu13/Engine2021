#pragma once

#include "engine/math/Geommath.h"

namespace longmarch
{
	struct Particle3D
	{
		Particle3D() = default;
		explicit Particle3D(const Vec3f& _position, const Vec3f& _velocity, float _gravity, float _lifeLength, float _rotation, float _scale, int _textureRows);

		Vec3f m_position;
		Vec3f m_velocity;
		float m_gravityEffect;
		float m_lifeLength;
		float m_rotation;
		float m_scale;
		float m_elapseTime;
		float m_distance;
		int m_textureRows{ 0 };
		Vec2f m_currentTextureOffset;
		Vec2f m_nextTextureOffset;
		float m_blendFactor;

	public:
		bool Update(const float frametime, const Vec3f& cameraPosition);

	private:
		void UpdateTextureCoordinateInfo();
		void UpdateTextureOffsets(Vec2f& offset, const int index);
	};
}
