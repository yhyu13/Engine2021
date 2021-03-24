#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {
	/**
	 * @brief Display all logging info
	 * It should act like a dockable menu
	 */
	class EnginePerformanceMonitor final : public BaseWidget
	{
	public:
		NONCOPYABLE(EnginePerformanceMonitor);
		EnginePerformanceMonitor();
		virtual void Render() override;

	private:
		ImVec2 m_Size;
	};

	// utility structure for realtime plot
	struct ScrollingBuffer {
		int MaxSize;
		int Offset;
		ImVector<ImVec2> Data;
		ScrollingBuffer() {
			MaxSize = 2000;
			Offset = 0;
			Data.reserve(MaxSize);
		}
		void AddPoint(float x, float y) {
			if (Data.size() < MaxSize)
				Data.push_back(ImVec2(x, y));
			else {
				Data[Offset] = ImVec2(x, y);
				Offset = (Offset + 1) % MaxSize;
			}
		}
		void Erase() {
			if (Data.size() > 0) {
				Data.shrink(0);
				Offset = 0;
			}
		}
	};

	// utility structure for realtime plot
	struct RollingBuffer {
		float Span;
		ImVector<ImVec2> Data;
		RollingBuffer() {
			Span = 10.0f;
			Data.reserve(2000);
		}
		void AddPoint(float x, float y) {
			float xmod = fmodf(x, Span);
			if (!Data.empty() && xmod < Data.back().x)
				Data.shrink(0);
			Data.push_back(ImVec2(xmod, y));
		}
	};
}
