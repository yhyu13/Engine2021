#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include "engine/core/EngineCore.h"

namespace longmarch
{
	//https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
	//http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3451.pdf
	// Creating a deamon thread job
	template<typename T>
	void LongMarch_UseDeamonThreadWaitAsyncJob(T&& in)
	{
		static std::mutex vmut;
		static std::vector<T> vec;
		static std::thread getter;
		static std::mutex single_getter;
		if (single_getter.try_lock())
		{
			getter = std::thread([&]()->void
			{
				size_t size;
				for (;;)
				{
					do
					{
						vmut.lock();
						size = vec.size();
						if (size > 0)
						{
							T target = std::move(vec[size - 1]);
							vec.pop_back();
							vmut.unlock();
							DEBUG_PRINT("LongMarch_UseDeamonThreadWaitAsyncJob getting!");
							if (target.valid())
							{
								target.wait();
							}
						}
						else
						{
							vmut.unlock();
						}
					} while (size > 0);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			});
			getter.detach();
		}
		vmut.lock();
		vec.push_back(std::move(in));
		vmut.unlock();
	}
}
