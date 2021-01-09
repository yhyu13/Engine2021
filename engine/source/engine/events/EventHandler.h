#pragma once
#include <memory>
#include <functional>
#include "Event.h"

namespace AAAAgames
{
	/**
	 * @brief Base class event callback
	 *
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class BaseEventHandler
	{
	public:
		NONCOPYABLE(BaseEventHandler);
		BaseEventHandler() = default;
		virtual ~BaseEventHandler() = default;
		void Execute(const std::shared_ptr<BaseEvent>& event)
		{
			return HandleEvent(event);
		}
	protected:
		virtual void HandleEvent(const std::shared_ptr<BaseEvent>& event) = 0;
	};

	/**
	 * @brief Event handler that take instance method
	 *
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	template<typename T, typename EventType>
	class InstanceEventHandler final : public BaseEventHandler
	{
	public:
		typedef void (T::* InstanceHandlerFunction) (std::shared_ptr<Event<EventType>>);
		NONCOPYABLE(InstanceEventHandler);
		InstanceEventHandler() = delete;
		explicit InstanceEventHandler(T* instance, InstanceHandlerFunction function)
			:
			m_handlerOwner(instance),
			m_handler(function)
		{}

		virtual void HandleEvent(const std::shared_ptr<BaseEvent>& event)
		{
#ifdef _DEBUG
			if (auto ptr = std::dynamic_pointer_cast<Event<EventType>>(event); ptr)
			{
				(m_handlerOwner->*m_handler)(ptr);
			}
			else
			{
				ENGINE_EXCEPT(L"Can not cast event to : " + wStr(typeid(Event<EventType>).name()));
			}
#else
			(m_handlerOwner->*m_handler)(std::static_pointer_cast<Event<EventType>>(event));
#endif // DEBUG
		}

	private:
		T* m_handlerOwner;
		InstanceHandlerFunction m_handler;
	};

	/**
	 * @brief Event handler that take global/class method
	 *
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	template<typename EventType>
	class GlobalEventHandler final : public BaseEventHandler
	{
	public:
		typedef void (*GlobalHandlerFunction) (std::shared_ptr<Event<EventType>>);
		NONCOPYABLE(GlobalEventHandler);
		GlobalEventHandler() = delete;
		explicit GlobalEventHandler(GlobalHandlerFunction function)
			:
			m_handler(function)
		{}

		virtual void HandleEvent(const std::shared_ptr<BaseEvent>& event)
		{
#ifdef _DEBUG
			if (auto ptr = std::dynamic_pointer_cast<Event<EventType>>(event); ptr)
			{
				(*m_handler)(ptr);
			}
			else
			{
				ENGINE_EXCEPT(L"Can not cast event to : " + wStr(typeid(Event<EventType>).name()));
			}
#else
			(*m_handler)(std::static_pointer_cast<Event<EventType>>(event));
#endif // DEBUG
		}

	private:
		GlobalHandlerFunction m_handler;
	};
}