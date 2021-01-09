#pragma once

namespace longmarch
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
std::ostream& operator<<(std::ostream& os, const longmarch::Event<EventType>& dt)
{
	os << '(' << dt.m_type << ") ";
	return os;
}