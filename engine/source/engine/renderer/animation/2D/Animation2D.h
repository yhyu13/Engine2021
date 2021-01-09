#pragma once
#include "engine/EngineEssential.h"

namespace longmarch
{
	struct ENGINE_API KeyFrame2D
	{
		std::string textureName;
		int x;
		int y;
		int width;
		int height;
		double timeStamp; // TODO does time stamp stands for the begnning or the ending of the current key frame?
	};

	class ENGINE_API Animation2D
	{
	public:
		static std::shared_ptr<Animation2D> LoadFromFile(const std::filesystem::path& path);

		Animation2D();

		void AddFrame(std::string textureName, int x, int y, int width, int height, double frameTime);

		[[nodiscard]] bool Empty() const { return keyFrames.empty(); }
		[[nodiscard]] const KeyFrame2D& GetCurrentKeyFrame() const { return keyFrames[currentFrameIndex]; }

		[[nodiscard]] bool UpdateFrame(double deltaTime);
		void Reset();

		void SetAnimLooping(bool b) { m_IsLooping = b; };
		void SetAnimPaused(bool b) { m_IsPaused = b; };

	private:
		[[nodiscard]] bool IncrementFrame();

		LongMarch_Vector<KeyFrame2D> keyFrames;

		int currentFrameIndex{ 0 };
		double currentFrameTime{ 0 };

		bool m_IsPaused{ false };
		bool m_IsLooping{ false };
	};
}
