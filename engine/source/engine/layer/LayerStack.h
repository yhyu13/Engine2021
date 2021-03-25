#pragma once

#include "engine/EngineEssential.h"
#include "Layer.h"

namespace longmarch {
	
	struct ENGINE_API LayerStack
	{
	public:
		using LayerStackVec = LongMarch_Vector<std::shared_ptr<Layer>>;
		using LayerStackMap = LongMarch_UnorderedMap<Layer::LAYER_TYPE, LayerStackVec>;
		

		void SwitchCurrentLayer(Layer::LAYER_TYPE layer);
		void PushLayer(const std::shared_ptr<Layer>& layer);
		void PushOverlay(const std::shared_ptr<Layer>& overlay);
		void PopLayer(const std::shared_ptr<Layer>& layer);
		void PopOverlay(const std::shared_ptr<Layer>& overlay);

		inline LayerStackMap& GetAllLayers() { return m_Layers; }
		inline LayerStackVec* GetCurrentLayer() { return m_currentLayer; }
	private:
		void SetCurrentLayer(Layer::LAYER_TYPE layer);
		void AttachCurrentLayer();
		void DetachCurrentLayer();
	private:
		LayerStackMap m_Layers;
		LayerStackVec* m_currentLayer;
		Layer::LAYER_TYPE m_currentLayerType;
	};
}