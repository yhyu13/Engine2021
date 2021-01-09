#pragma once
#include <any>
#include <vector>
#include <string>
#include "engine/core/EngineCore.h"
#include "engine/core/utility/TypeHelper.h"

namespace AAAAgames
{
	class Blackboard
	{
	public:
		template <typename T>
		void set_value(const std::string& key, const T& value);

		//! Get value with key. If key does not exist, throw an exception.
		template <typename T>
		T get_value(const std::string& key) const;

		bool has_value(const std::string& key) const
		{
			return data.find(key) != data.end();
		}

	private:
		A4GAMES_UnorderedMap_Par_flat<std::string, std::any> data;
	};

	template<typename T>
	inline void Blackboard::set_value(const std::string& key, const T& value)
	{
		// std::any can only work with copy-constructible objects, so static assert here instead of getting a nasty template error at compile time :)
		ASSERT(std::is_copy_constructible<T>::value, "Attempting to add non-copy-constructible type into blackboard");
		data[key] = value;
	}

	template<typename T>
	inline T Blackboard::get_value(const std::string& key) const
	{
		// make sure there is a value for the key
		if (auto result = data.find(key); result != data.end())
		{
			// try to cast the value to the expected type
			try
			{
				return std::any_cast<T>(result->second);
			}
			catch (const std::bad_any_cast&)
			{
				// attempted to cast value to the wrong type, that means the value for this key was set to a different type
				__debugbreak();
			}
		}
		else
		{
			// attempted to get value that hasn't been set yet
			ENGINE_EXCEPT(str2wstr(key) + L" does not exist in bb!");
		}

		// if we manage to get here, just default construct the value
		// this really isn't a good thing to do, but it will allow you to continue
		// the debugging process if you really don't care about the value
		return T();
	}
}