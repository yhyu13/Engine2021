#pragma once

#include "StealThreadPool.h"

namespace longmarch
{
	/*
		Unity like Job system
	*/
	class BaseJob
	{
	public:
		typedef std::future<void> JHandle;

		BaseJob() = default;
		virtual ~BaseJob() = default;

		/*
			Bring all content into the Execute() method
		*/
		virtual void Execute() = 0;

		/*
			Call Schedule() to launch without dependency
		*/
		[[nodiscard]] JHandle Schedule()
		{
			return StealThreadPool::GetInstance()->enqueue_task([this]() { Execute(); });
		}
		/*
			Call Schedule() to launch with dependency
		*/
		[[nodiscard]] JHandle Schedule(std::initializer_list<JHandle*> dependet_handles)
		{
			return StealThreadPool::GetInstance()->enqueue_task([this, dependet_handles]()
			{
				for (auto& job : dependet_handles)
				{
					(*job).wait();
				}
				Execute();
			});
		}
		/*
			Call Schedule() to launch with dependency
		*/
		[[nodiscard]] JHandle Schedule(std::vector<JHandle*>& dependet_handles)
		{
			return StealThreadPool::GetInstance()->enqueue_task([this, dependet_handles]()
			{
				for (auto& job : dependet_handles)
				{
					(*job).wait();
				}
				Execute();
			});
		}
	};
}