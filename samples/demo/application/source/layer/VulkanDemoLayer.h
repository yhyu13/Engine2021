#pragma once
#include "Import.h"

namespace longmarch
{
	/**
	 * @brief Main editor layer
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class VulkanDemoLayer final : public Layer
	{
	public:
		NONCOPYABLE(VulkanDemoLayer);
		VulkanDemoLayer();
		virtual void Init() override;
		virtual void OnUpdate(double ts) override;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void PreUpdate(double ts);
		void Update(double ts);
		void JoinAll();
		void PreRenderUpdate(double ts);
		void Render(double ts);
		void PostRenderUpdate(double ts);

		void BuildRenderPipeline();
		void BuildTestScene();

	private:
		struct
		{
			std::function<void(double)> mainRenderPipeline;

			ImFont* m_font_1k;
			ImFont* m_font_2k;
			ImFont* m_font_4k;
		}m_Data;
	};
}