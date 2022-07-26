#pragma once

#include "engine/EngineEssential.h"
#include "engine/core/thread/Scheduler.h"
#include "engine/core/thread/StealThreadPool.h"
#include "EventHandler.h"

namespace longmarch
{
#define THROW_ON_DUPLICATED_EVENT_CALLBACk 1
#define THROW_ON_UNREGISTERED_EVENT_CALLBACk 0
	/**
	 * @brief Base class event subscriber handle
	 *
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
	 */
	struct BaseEventSubHandle
	{
		NONCOPYABLE(BaseEventSubHandle);
		BaseEventSubHandle() = default;
		virtual ~BaseEventSubHandle() = default;
		virtual void RemoveEventSub() const = 0;
	};
	/**
	 * @brief Base class event queue
	 *
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
	 */
	template <typename EventType>
	class EventQueue final : protected BaseAtomicClassNC
	{
	public:
		NONCOPYABLE(EventQueue);
		using EventPtr = std::shared_ptr<Event<EventType>>;	// Define event smart pointer type

		static EventQueue* GetInstance()
		{
			static EventQueue instance;
			return &instance;
		}

	private:
		EventQueue()
		{
			m_eventsUpdateHandle = Scheduler::GetInstance(33)->set_interval(std::chrono::milliseconds(33), [this]() { UpdateAsync(33e-3); });
		}
		~EventQueue()
		{
			m_eventsUpdateHandle->signal();
		}

		using BaseEventPtr = std::shared_ptr<BaseEventHandler>;
		using EventHandlersMap = LongMarch_UnorderedMap<size_t, BaseEventPtr>;
		struct EventHandlers final : public BaseAtomicClass
		{
			auto begin() const { return handlersMap.begin(); }
			auto end() const { return handlersMap.end(); }
			EventHandlersMap handlersMap;
		};
		using EventSubsriberLUT = LongMarch_UnorderedMap_Par_node<EventType, EventHandlers>;

		//! Internal type that convert a regular event into a delayed event
		struct DelayedEvent : Event<EventType> {
			DelayedEvent() = default;
			explicit DelayedEvent(EventPtr e, double delayTime)
				:
				m_event(e), m_triggerTime(delayTime)
			{}
			EventPtr m_event;
			double m_triggerTime;
		};

		using DelayedEventPtr = std::shared_ptr<DelayedEvent>;	// Define delayed event smart pointer type
		struct DelayedEventComparator {
			bool operator() (const DelayedEventPtr& event1, const DelayedEventPtr& event2) {
				return event1->m_triggerTime > event2->m_triggerTime;
			}
		};
		using delayed_queue = std::priority_queue<DelayedEventPtr, std::vector<DelayedEventPtr>, DelayedEventComparator>;

	public:
		struct EventSubHandle final : public BaseEventSubHandle
		{
			NONCOPYABLE(EventSubHandle);
			EventSubHandle() = delete;
			explicit EventSubHandle(EventQueue* ptr, EventType type, size_t mask)
				:
				m_ptr(ptr),
				m_type(type),
				m_mask(mask)
			{}

			inline void RemoveEventSub() const
			{
				m_ptr->LockNC();
				if (auto it = m_ptr->m_subscribers.find(m_type); it != m_ptr->m_subscribers.end())
				{
					it->second.Lock();
					it->second.handlersMap.erase(m_mask);
					it->second.UnLock();
				}
				m_ptr->UnlockNC();
			}

		private:
			EventQueue* m_ptr;
			EventType m_type;
			size_t m_mask;
		};

		//! Subscribe member method event handler (either public, protected, or private works)
		template <typename T>
		[[nodiscard]] std::shared_ptr<EventSubHandle> Subscribe(T* instance, EventType eventType, void (T::* Function)(EventPtr))
		{
			LOCK_GUARD_NC();
			size_t mask = 0;
			LongMarch_HashCombine(mask, reinterpret_cast<uintptr_t>(&Function));
			LongMarch_HashCombine(mask, reinterpret_cast<uintptr_t>(instance));
			auto& subs = m_subscribers[eventType];
			subs.Lock();
			if (!LongMarch_contains(subs.handlersMap, mask))
			{
				subs.handlersMap.emplace(mask, MemoryManager::Make_shared<InstanceEventHandler<T, EventType>>(instance, Function));
				subs.UnLock();
				return MemoryManager::Make_shared<EventSubHandle>(this, eventType, mask);
			}
			else
			{
#if  THROW_ON_DUPLICATED_EVENT_CALLBACk == 0
				return nullptr;
#else
				ENGINE_EXCEPT(L"Duplicated event callback!");
				return nullptr;
#endif //  THROW_ON_DUPLICATED_EVENT_CALLBACk == 0
			}
		}

		//! Subscribe global/static method event handler (either public, protected, or private works)
		std::shared_ptr<EventSubHandle> Subscribe(EventType eventType, void (*Function)(EventPtr))
		{
			LOCK_GUARD_NC();
			size_t mask = reinterpret_cast<size_t>(&Function);
			auto& subs = m_subscribers[eventType];
			subs.Lock();
			if (!LongMarch_contains(subs.handlersMap, mask))
			{
				subs.handlersMap.emplace(mask, MemoryManager::Make_shared<GlobalEventHandler<EventType>>(Function));
				subs.UnLock();
				return MemoryManager::Make_shared<EventSubHandle>(this, eventType, mask);
			}
			else
			{
#if  THROW_ON_DUPLICATED_EVENT_CALLBACk == 0
				return nullptr;
#else
				ENGINE_EXCEPT(L"Duplicated event callback!");
				return nullptr;
#endif //  THROW_ON_DUPLICATED_EVENT_CALLBACk == 0
			}
		}

		//! Intantieous execution of a synchronous event
		inline void Publish(EventPtr e)
		{
			auto _e = std::static_pointer_cast<BaseEvent>(e);
			LockNC();
			auto it = m_subscribers.find(e->m_type);
			if (it == m_subscribers.end())
			{
				UnlockNC();
#if THROW_ON_UNREGISTERED_EVENT_CALLBACk == 0			
				ENGINE_WARN("Event with type " + (Str(e->m_type)) + " is not registered in the event queue of type: " + (typeid(EventType).name()));
				return;
#else
				ENGINE_EXCEPT(L"Event with type " + wStr(Str(e->m_type)) + L" is not registered in the event queue of type: " + wStr(typeid(EventType).name()));
				return;
#endif
			}
			auto& subs = it->second;
			UnlockNC();
			subs.Lock();
			for (auto& [_, handler] : subs)
			{
				if (handler != nullptr)
				{
					handler->Execute(_e);
				}
			}
			subs.UnLock();
		}

		//! Instanct execution of an async event in a background thread
		inline void PublishAsync(EventPtr e)
		{
			LongMarch_DeamonThread(std::async(std::launch::async, [this, e]()
			{
				auto& _e = std::static_pointer_cast<BaseEvent>(e);
				LockNC();
				auto& it = m_subscribers.find(e->m_type);
				ENGINE_EXCEPT_IF(it == m_subscribers.end(), L"Event with type " + wStr(Str(e->m_type)) + L" is not registered in the event queue of type: " + wStr(typeid(EventType).name()));
				auto& subs = it->second;
				UnlockNC();
				subs.Lock();
				for (auto& [_, handler] : subs)
				{
					if (handler)
					{
						handler->Execute(_e);
					}
				}
				subs.UnLock();
			}));
		}

		//! Delayed execution of a synchronous event
		inline void Publish(EventPtr event, const double delay) {
			LOCK_GUARD_NC();
			DelayedEventPtr delayedEvent = MemoryManager::Make_shared<DelayedEvent>(event, delay);
			m_events.emplace(std::move(delayedEvent));
		}

		//! Delayed execution of an delayed async event in a background thread
		inline void PublishAsync(EventPtr event, const double delay) {
			LOCK_GUARD_NC();
			DelayedEventPtr delayedEvent = MemoryManager::Make_shared<DelayedEvent>(event, delay);
			m_eventsAsync.emplace(std::move(delayedEvent));
		}

		//! Clear all delayed events and subscribers
		inline void Clear()
		{
			LOCK_GUARD_NC();
			// Reset with empty queue
			m_events = std::move(delayed_queue());
			m_eventsAsync = std::move(delayed_queue());
			m_subscribers.clear();
			m_bDelayedEventShouldBeCleared = true;
		}

		//! Remove delayed events by type
		inline void RemoveDelayedEvent(EventType type)
		{
			LOCK_GUARD_NC();
			{
				std::vector<DelayedEventPtr> temp_events;
				temp_events.reserve(m_events.size());
				while (!m_events.empty())
				{
					if (DelayedEventPtr delayedEvent = m_events.top();
						!(delayedEvent->m_event->m_type == type))
					{
						temp_events.emplace_back(delayedEvent);
					}
					m_events.pop();
				}
				for (auto event : temp_events)
				{
					m_events.emplace(event);
				}
			}
			{
				std::vector<DelayedEventPtr> temp_events;
				temp_events.reserve(m_eventsAsync.size());
				while (!m_eventsAsync.empty())
				{
					if (DelayedEventPtr delayedEvent = m_eventsAsync.top();
						!(delayedEvent->m_event->m_type == type))
					{
						temp_events.emplace_back(delayedEvent);
					}
					m_eventsAsync.pop();
				}
				for (auto event : temp_events)
				{
					m_eventsAsync.emplace(event);
				}
			}
		}

		//! Update synchronous delayed event
		inline void Update(double frameTime)
		{
			delayed_queue events_temp;
			{
				LOCK_GUARD_NC();
				if (m_events.empty())
				{
					return;
				}
				// Make a deep copy
				events_temp = m_events;
				// Clear the original queue
				m_events = std::move(delayed_queue());
			}
			/*
				A few delayed event might call Clear() function such that all remaining delayed event
				should not be called. We will need to check if m_bDelayedEventShouldBeCleared has
				been set.
			*/
			m_bDelayedEventShouldBeCleared = false;
			while (!events_temp.empty() && !m_bDelayedEventShouldBeCleared)
			{
				DelayedEventPtr delayedEvent = events_temp.top();
				events_temp.pop();
				/*
					Since the priority queue mimic min-heap, event at the top has the lowest trigger-time.
					If the current system-time is not greater than the trigger-time of element at the top,
					program counter will come out of the loop. Otherwise, the system will keep publishing
					events.
				*/
				if ((delayedEvent->m_triggerTime -= frameTime) <= 0)
				{
					Publish(delayedEvent->m_event);
				}
				else
				{
					LOCK_GUARD_NC();
					m_events.emplace(delayedEvent);
				}
			}
		}

	private:
		//! Update async delayed event
		inline void UpdateAsync(double frameTime)
		{
			delayed_queue events_temp;
			{
				LOCK_GUARD_NC();
				if (m_eventsAsync.empty())
				{
					return;
				}
				// Make a deep copy
				events_temp = m_eventsAsync;
				// Clear the original queue
				m_eventsAsync = std::move(delayed_queue());
			}
			/*
				A few delayed event might call Clear() function such that all remaining delayed event
				should not be called. We will need to check if m_bDelayedEventShouldBeCleared has
				been set.
			*/
			m_bDelayedEventShouldBeCleared = false;
			while (!events_temp.empty() && !m_bDelayedEventShouldBeCleared)
			{
				DelayedEventPtr delayedEvent = events_temp.top();
				events_temp.pop();
				/*
					Since the priority queue mimic min-heap, event at the top has the lowest trigger-time.
					If the current system-time is not greater than the trigger-time of element at the top,
					program counter will come out of the loop. Otherwise, the system will keep publishing
					events.
				*/
				if ((delayedEvent->m_triggerTime -= frameTime) <= 0)
				{
					LongMarch_DeamonThread(std::async(std::launch::async, [this, delayedEvent]() { Publish(delayedEvent->m_event); }));
				}
				else
				{
					LOCK_GUARD_NC();
					m_eventsAsync.emplace(delayedEvent);
				}
			}
		}

	private:
		EventSubsriberLUT m_subscribers;
		delayed_queue m_events;
		delayed_queue m_eventsAsync;
		Scheduler::SchedulerHandle m_eventsUpdateHandle = { nullptr };
		bool m_bDelayedEventShouldBeCleared = { false };
	};
	/**
	 * @brief Base class event subscriber helper class
	 *
	 * Making a dervied class of every class that you want event subscriptions to be canceled on destruction.
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class BaseEventSubHandleClass
	{
	public:
		virtual ~BaseEventSubHandleClass()
		{
			RemoveAllHandles();
		}

		void ManageEventSubHandle(const std::shared_ptr<BaseEventSubHandle>& handle)
		{
			if (handle)
			{
				m_subHandles.insert(handle);
			}
			else
			{
#if  THROW_ON_DUPLICATED_EVENT_CALLBACk == 0
				DEBUG_PRINT("Trying to manage a nullptr event sub handle!");
#else
				ENGINE_EXCEPT(L"Trying to manage a nullptr event sub handle!");
#endif //  THROW_ON_DUPLICATED_EVENT_CALLBACk == 0
			}
		}

		void RemoveManagedHandle(const std::shared_ptr<BaseEventSubHandle>& handle)
		{
			if (const auto& it = m_subHandles.find(handle); it != m_subHandles.cend())
			{
				if (auto& handle = *it; handle)
				{
					handle->RemoveEventSub();
				}
				m_subHandles.erase(it);
			}
		}

		void RemoveAllHandles()
		{
			const auto& _m_subHandles = m_subHandles;
			for (auto& handle : _m_subHandles)
			{
				if (handle)
				{
					handle->RemoveEventSub();
				}
			}
			m_subHandles.clear();
		}

	private:
		LongMarch_Set<std::shared_ptr<BaseEventSubHandle>> m_subHandles;
	};
#undef THROW_ON_DUPLICATED_EVENT_CALLBACk
#undef THROW_ON_UNREGISTERED_EVENT_CALLBACk
}
