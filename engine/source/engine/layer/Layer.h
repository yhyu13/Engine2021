#pragma once
#include "engine/core/EngineCore.h"
#include "engine/events/EventQueue.h"
#include <imgui/imgui.h>

namespace longmarch {
	/**
	 * @brief Base layer class to drive engine/game update
	 *
	 * @author Kyle Wang (kyle.wang@digipen.edu | 60000719), Hang Yu (yohan680919@gmail.com)
	 */
	class ENGINE_API Layer : public BaseEventSubHandleClass
	{
	public:
		enum class LAYER_TYPE : uint32_t
		{
			EMPTY = 0u,
			ENG_LAYER,
			APP_LAYER,
			NUM
		};

	public:
		NONCOPYABLE(Layer);
		explicit Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;
		virtual void Init() {}
		virtual void Finish() {}

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(double ts) {}
		virtual void OnImGuiRender() {}

		inline const std::string GetDebugName() { return m_DebugName; }

	protected:
		std::string m_DebugName;
	};
}
