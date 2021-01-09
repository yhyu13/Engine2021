#include "engine-precompiled-header.h"
#include "LayerStack.h"

namespace longmarch {

	void LayerStack::SwitchCurrentLayer(Layer::LAYER_TYPE layer)
	{
		if (m_currentLayerType != layer)
		{
			SetCurrentLayer(m_currentLayerType);
			DetachCurrentLayer();
			SetCurrentLayer(layer);
			AttachCurrentLayer();
		}
	}

	void LayerStack::PushLayer(const std::shared_ptr<Layer>& layer)
	{
		m_currentLayer->push_back(layer);
		layer->OnAttach();
	}

	void LayerStack::PushOverlay(const std::shared_ptr<Layer>& overlay)
	{
		m_currentLayer->push_back(overlay);
		overlay->OnAttach();
	}

	void LayerStack::PopLayer(const std::shared_ptr<Layer>& layer)
	{
		if (auto it = std::find(m_currentLayer->begin(), m_currentLayer->end(), layer); 
			it != m_currentLayer->end())
		{
			for (auto _it = m_currentLayer->end()-1; _it != it; --_it)
			{
				(*_it)->OnDetach();
			}
			(*it)->OnDetach();
			m_currentLayer->erase(it, m_currentLayer->end());
		}
	}

	void LayerStack::PopOverlay(const std::shared_ptr<Layer>& overlay)
	{
		if (auto it = std::find(m_currentLayer->begin(), m_currentLayer->end(), overlay); 
			it != m_currentLayer->end())
		{
			(*it)->OnDetach();
			m_currentLayer->erase(it);
		}
	}
	void LayerStack::SetCurrentLayer(Layer::LAYER_TYPE layer)
	{
		m_currentLayerType = layer;
		m_currentLayer = &(m_Layers[layer]);
	}
	void LayerStack::AttachCurrentLayer()
	{
		for (auto _it = m_currentLayer->begin(); _it != m_currentLayer->end(); ++_it)
		{
			(*_it)->OnAttach();
		}
	}
	void LayerStack::DetachCurrentLayer()
	{
		for (auto _it = m_currentLayer->rbegin(); _it != m_currentLayer->rend(); ++_it)
		{
			(*_it)->OnDetach();
		}
	}
}

