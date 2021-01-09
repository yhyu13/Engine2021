#pragma once
#include <json/json.h>

namespace AAAAgames
{
	template <typename T>
	Json::Value A4GAMES_ArrayToJsonValue(const T& arr)
	{
		Json::Value vec(Json::arrayValue);
		for (auto&& item : arr)
		{
			vec.append(Json::Value(item));
		}
		return vec;
	}

	template <typename T>
	Json::Value A4GAMES_ArrayToJsonValue(const T& arr, int length)
	{
		Json::Value vec(Json::arrayValue);
		for (int i(0); i < length; ++i)
		{
			vec.append(Json::Value(arr[i]));
		}
		return vec;
	}

	template <typename T>
	Json::Value A4GAMES_ToJsonValue(const T& item)
	{
		return Json::Value(item);
	}
}