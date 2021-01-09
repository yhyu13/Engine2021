#pragma once

namespace AAAAgames
{
	struct BaseEvent
	{
		BaseEvent() = default;
		virtual ~BaseEvent() = default;
	};

	template <typename EventType>
	struct Event : BaseEvent
	{
		Event() = default;
		explicit Event(EventType type)
			:
			m_type(type)
		{}
		virtual ~Event() = default;
		EventType m_type;
	};
}

template <typename EventType>
std::ostream& operator<<(std::ostream& os, const AAAAgames::Event<EventType>& dt)
{
	os << '(' << dt.m_type << ") ";
	return os;
}