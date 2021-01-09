#include "engine-precompiled-header.h"
#include "Particle3D.h"

namespace longmarch
{
	Particle3D::Particle3D(const Vec3f& t_position, const Vec3f& t_velocity, float t_gravity, float t_lifeLength, float t_rotation, float t_scale, int t_textureRows)
		: m_position(t_position),
		m_velocity(t_velocity),
		m_gravityEffect(t_gravity),
		m_lifeLength(t_lifeLength),
		m_rotation(t_rotation),
		m_scale(t_scale),
		m_elapseTime(0.0f),
		m_distance(0.0f),
		m_textureRows(t_textureRows),
		m_currentTextureOffset(Vec2f(0.0f)),
		m_nextTextureOffset(Vec2f(0.0f)),
		m_blendFactor(0.0f)
	{}

	bool Particle3D::Update(const float& frametime, const Vec3f& cameraPosition)
	{
		m_velocity.z += GRAVITY * m_gravityEffect * frametime; // frame-time in seconds
		Vec3f change(m_velocity);
		change *= frametime;
		m_position += change;
		m_distance = Geommath::Distance(cameraPosition, m_position);
		UpdateTextureCoordinateInfo();
		m_elapseTime += frametime;
		return (m_elapseTime < m_lifeLength);
	}

	void Particle3D::UpdateTextureCoordinateInfo()
	{
		float lifeFactor = m_elapseTime / m_lifeLength;
		int stageCount = m_textureRows * m_textureRows;
		float atlasProgression = lifeFactor * stageCount;
		int currentIndex = (int)atlasProgression;
		int nextIndex = currentIndex < stageCount - 1 ? currentIndex + 1 : currentIndex;
		m_blendFactor = std::fmodf(atlasProgression, 1.0f);
		UpdateTextureOffsets(m_currentTextureOffset, currentIndex);
		UpdateTextureOffsets(m_nextTextureOffset, nextIndex);
	}

	void Particle3D::UpdateTextureOffsets(Vec2f& offset, const int index)
	{
		int column = std::fmodf(index, m_textureRows);
		int row = index / m_textureRows;
		offset.x = (float)column / m_textureRows;
		offset.y = (float)row / m_textureRows;
	}
}