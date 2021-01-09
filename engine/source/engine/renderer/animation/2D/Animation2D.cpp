#include "engine-precompiled-header.h"
#include "Animation2D.h"

std::shared_ptr<Animation2D> AAAAgames::Animation2D::LoadFromFile(const fs::path& path)
{
	// TODO : implement Animation 2D load from file function
	return MemoryManager::Make_shared<Animation2D>();
}

AAAAgames::Animation2D::Animation2D()
	:
	keyFrames(0),
	currentFrameIndex(0),
	currentFrameTime(0.0),
	m_IsPaused(false),
	m_IsLooping(true)
{
}

void AAAAgames::Animation2D::AddFrame(std::string textureName, int x, int y, int width, int height, double frameTime)
{
	KeyFrame2D data;
	data.textureName = textureName;
	data.x = x;
	data.y = y;
	data.width = width;
	data.height = height;
	data.timeStamp = frameTime;

	keyFrames.push_back(data);
}

[[nodiscard]]
bool AAAAgames::Animation2D::UpdateFrame(double deltaTime)
{
	if (m_IsPaused || Empty())
	{
		return false;
	}
	if ((currentFrameTime += deltaTime) >= keyFrames[currentFrameIndex].timeStamp)
	{
		return IncrementFrame();
	}
	return false;
}

[[nodiscard]]
bool AAAAgames::Animation2D::IncrementFrame()
{
	// check if we reached the last frame
	if (auto lastIndex = (keyFrames.size() - 1); currentFrameIndex++ >= lastIndex)
	{
		if (m_IsLooping)
		{
			Reset();
		}
		else
		{
			currentFrameIndex = lastIndex;
			return false;
		}
	}
	return true;
}

void AAAAgames::Animation2D::Reset()
{
	currentFrameTime = 0.0;
	currentFrameIndex = 0;
}