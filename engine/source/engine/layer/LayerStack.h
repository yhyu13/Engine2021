#pragma once

#include "engine/EngineEssential.h"
#include "Layer.h"

namespace longmarch {
	
	struct ENGINE_API LayerStack
	{
	public:
		typedef LongMarch_Vector<std::shared_ptr<Layer>> LayerType;
		typedef LongMarch_UnorderedMap<Layer::LAYER_TYPE, LayerType> LayerStackType;
		

		void SwitchCurrentLayer(Layer::LAYER_TYPE layer);
		void PushLayer(const std::shared_ptr<Layer>& layer);
		void PushOverlay(const std::shared_ptr<Layer>& overlay);
		void PopLayer(const std::shared_ptr<Layer>& layer);
		void PopOverlay(const std::shared_ptr<Layer>& overlay);

		inline LayerStackType& GetAllLayers() { return m_Layers; }
		inline LayerType* GetCurrentLayer() { return m_currentLayer; }
	private:
		void SetCurrentLayer(Layer::LAYER_TYPE layer);
		void AttachCurrentLayer();
		void DetachCurrentLayer();
	private:
		LayerStackType m_Layers;
		LayerType* m_currentLayer;
		Layer::LAYER_TYPE m_currentLayerType;
	};
}