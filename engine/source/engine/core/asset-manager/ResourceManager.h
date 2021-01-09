#pragma once
#include "../EngineCore.h"
#include "../utility/TypeHelper.h"
#include "../exception/EngineException.h"
#include "../file-system/FileSystem.h"

namespace longmarch
{
	/**
	 * @brief Custom ResourceManager template that manages the life time of resources
	 *
	 * Use it like : ResourceManager<T>::GetInstance()->AddResource(name, path, resource),
	 *				 ResourceManager<T>::GetInstance()->LoadFromFile(name, path)
	 *				 ResourceManager<T>::GetInstance()->LoadFromFileAsync(name, path)
	 *				 ResourceManager<T>::GetInstance()->TryGet(name)
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	template<typename T>
	class ENGINE_API ResourceManager : public BaseAtomicClassNC
	{
	private:
		ResourceManager() = default;
		NONCOPYABLE(ResourceManager);

	public:
		using ResourcePtr = std::shared_ptr<T>;
		using ResourceFuture = std::shared_future<ResourcePtr>;
		using ResourceCallback = std::function<void()>;
		using ResourcePromise = std::promise<ResourcePtr>;
		using ResourceLoadFromFile = std::function<ResourcePtr(const fs::path&)>;

		struct ResourceTask final : public BaseAtomicClass2
		{
			ResourceTask() = default;
			// Define only copy constructor/assingment to avoid implicit defining move operations 
			// which is not desired because we are using future
			ResourceTask(const ResourceTask&) = default;
			ResourceTask& operator=(const ResourceTask&) = default;

			//! Converting contrsutor
			ResourceTask(const ResourcePtr& ptr_) :ptr(ptr_) {}
			ResourceTask& operator=(const ResourcePtr& ptr_) { Reset(); ptr = ptr_; return *this; }

			//! Converting contrsutor
			ResourceTask(const ResourceFuture& future_) : future(future_) {}
			ResourceTask& operator=(const ResourceFuture& future_) { Reset(); future = future_; return *this; }

			//! Is the future contains a valid shared state
			bool IsFutureValid() const noexcept
			{
				LOCK_GUARD2();
				return future.valid();
			}

			//! Invalidate both resource and future
			void Reset() noexcept
			{
				LOCK_GUARD2();
				ptr.reset();
				future = std::move(ResourceFuture());
			}

			//! Set future and invalidate resource
			void SetFuture(const ResourceFuture& future_) noexcept
			{
				LOCK_GUARD2();
				ptr.reset();
				future = future_;
			}

			//! Set callback on successful getting, callback is only called once and callback is presistent on copying
			void SetCallback(const ResourceCallback& callback_)
			{
				LOCK_GUARD2();
				callback = callback_;
				if (ptr && callback)
				{
					callback();
				}
			}

			//! Call future.wait_for(0), if not ready, return nullptr
			[[nodiscard]] ResourcePtr TryGet()
			{
				LOCK_GUARD2();
				if (ptr != nullptr)
				{
					return ptr;
				}
				else
				{
					if (future.valid())
					{
						switch (auto status = future.wait_for(std::chrono::seconds(0)); status)
						{
						case std::future_status::ready:
							if (ptr = future.get(); ptr)
							{
								if (callback)
								{
									callback();
								}
								return ptr;
							}
							else
							{
								ENGINE_EXCEPT(L"ResourceTask " + wStr(typeid(T).name()) + L" future return nullptr!");
								return nullptr;
							}
						default:
							return nullptr;
						}
					}
					else
					{
						return nullptr;
					}
				}
			}

			//! Call future.get(). Wait until the resource is ready.
			[[nodiscard]] ResourcePtr Get()
			{
				LOCK_GUARD2();
				if (ptr != nullptr)
				{
					return ptr;
				}
				else
				{
					if (future.valid())
					{
						if (ptr = future.get(); ptr)
						{
							if (callback)
							{
								callback();
							}
							return ptr;
						}
						else
						{
							ENGINE_EXCEPT(L"ResourceTask " + wStr(typeid(T).name()) + L" future return nullptr!");
							return nullptr;
						}
					}
					else
					{
						ENGINE_EXCEPT(L"ResourceTask " + wStr(typeid(T).name()) + L" future is not valid!");
						return nullptr;
					}
				}
			}

			//! pointer redirector, casting to pointer (call Get())
			ResourcePtr operator->()
			{
				return Get();
			}

			//! casting to bool
			operator bool() const noexcept
			{
				LOCK_GUARD2();
				return ptr != nullptr;
			}

			//! Either handle that contains nullptr would return false
			friend bool operator==(const ResourceTask& lhs, const ResourceTask& rhs) noexcept
			{
				bool ret = false;
				lhs.Lock2(); rhs.Lock2();
				if (lhs.ptr && rhs.ptr)
				{
					ret = (lhs.ptr == rhs.ptr);
				}
				lhs.Unlock2(); rhs.Unlock2();
				return ret;
			}
		private:
			friend ResourceManager;
			ResourcePtr ptr{ nullptr };
			ResourceFuture future;
			ResourceCallback callback;
		};

		struct ResourceHandle
		{
			ResourceHandle() = default;
			ResourceHandle(const ResourceHandle&) = default;
			ResourceHandle& operator=(const ResourceHandle&) = default;
			ResourceHandle(ResourceHandle&&) = default;
			ResourceHandle& operator=(ResourceHandle&&) = default;

			// Explicit constructor
			explicit ResourceHandle(const std::shared_ptr<ResourceTask>& task_) : task(task_){}
			// Converting constructor
			ResourceHandle(const ResourceTask& task_) :task(MemoryManager::Make_shared<ResourceTask>(task_)) {}
			// Explicit assingment
			ResourceHandle& operator=(const ResourcePtr& ptr_) noexcept{ *task = ptr_; return *this; }

			ResourceTask* operator->() noexcept
			{
				if (task)
				{
					return task.get();
				}
				else
				{
					return &s_task_empty;
				}
			}
			ResourceTask& operator*() noexcept
			{
				if (task)
				{
					return *task;
				}
				else
				{
					return s_task_empty;
				}
			}
			operator ResourceTask() noexcept
			{
				if (task)
				{
					return *task;
				}
				else
				{
					return s_task_empty;
				}
			}
			operator bool() const noexcept
			{
				if (task)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		private:
			std::shared_ptr<ResourceTask> task{ nullptr };
			inline static ResourceTask s_task_empty;
		};

	public:
		static ResourceManager* GetInstance() noexcept
		{
			static ResourceManager inst;
			return &inst;
		}

		//! Add resource to the allocator, replace previously owned resource if name is conflicted.
		void AddResource(const std::string& Name, const fs::path& filePath, const ResourcePtr& resource) noexcept
		{
			LOCK_GUARD_NC();
			m_resources[Name] = std::make_pair(filePath, ResourceTask(resource));
			auto& v = m_promises[Name];
			for (auto& p : v)
			{
				p.set_value(resource);
			}
			v.clear();
		}

		//! Set custom load from file function (must be set before loading any resource<T> that invloves)
		void SetLoadFromFileFunc(const ResourceLoadFromFile& func) noexcept
		{
			LOCK_GUARD_NC();
			m_loadFromFileFunc = func;
		}

		//! Check if LoadFromFile function has been set to an non-empty value
		bool HasLoadFromFileFunc() const noexcept
		{
			LOCK_GUARD_NC();
			return !!m_loadFromFileFunc;
		}

		//! Load a resource with a user give implementation, call SetLoadFromFileFunc before calling this function
		//! LoadFromFile blocks on loading but it can fulfill futures that are generated from Get() by promise.
		ResourceHandle LoadFromFile(const std::string& Name, const fs::path& filePath)
		{
			{
				LOCK_GUARD_NC();
				if (const auto& it = m_resources.find(Name); it != m_resources.end())
				{
					if (const auto& res_ref = it->second.second; res_ref || res_ref.IsFutureValid())
					{
						return res_ref;
					}
				}
				for (const auto& item : m_resources)
				{
					if (const auto& path = item.second.first; path == filePath)
					{
						if (const auto& res_ref = item.second.second; res_ref || res_ref.IsFutureValid())
						{
							return res_ref;
						}
					}
				}
			}
			ENGINE_EXCEPT_IF(!m_loadFromFileFunc, L"Resource type : " + wStr(typeid(T).name()) + L" does not provide a valid LoadFromFile function!");
			auto resource = m_loadFromFileFunc(filePath);
			ENGINE_EXCEPT_IF(!resource, L"Resource at " + str2wstr(filePath.string()) + L" has failed to load!");
			AddResource(Name, filePath, resource);
			return ResourceTask(resource);
		}

		//! Load a resource with a user give implementation, call SetLoadFromFileFunc before calling this function
		//! LoadFromFileAsync does not block but it cannot fulfill futures that are generated from Get() by promise.
		ResourceHandle LoadFromFileAsync(const std::string& Name, const fs::path& filePath)
		{
			LOCK_GUARD_NC();
			if (const auto& it = m_resources.find(Name); it != m_resources.end())
			{
				if (const auto& res_ref = it->second.second; res_ref || res_ref.IsFutureValid())
				{
					return res_ref;
				}
			}
			for (const auto& item : m_resources)
			{
				if (const auto& path = item.second.first; path == filePath)
				{
					if (const auto& res_ref = item.second.second; res_ref || res_ref.IsFutureValid())
					{
						return res_ref;
					}
				}
			}
			ENGINE_EXCEPT_IF(!m_loadFromFileFunc, L"Resource type : " + wStr(typeid(T).name()) + L" does not provide a valid LoadFromFile function!");
			ResourceTask task(std::async(std::launch::async, [this, filePath, Name]() {
				auto resource = m_loadFromFileFunc(filePath);
				ENGINE_EXCEPT_IF(!resource, L"Resource at " + str2wstr(filePath.string()) + L" has failed to load!");
				return resource;
			}).share());
			m_resources[Name] = std::make_pair(filePath, task);
			return task;
		}

		//! Get resource by name, if does not exist, it will call LoadFromFile if path exists. If both fails, then it retuns a future from a promise that can be fulfiled by AddResource
		[[nodiscard]] ResourceHandle TryGet(const std::string& Name)
		{
			LOCK_GUARD_NC();
			if (auto it = m_resources.find(Name); it != m_resources.end())
			{
				if (auto& res_ref = it->second.second; res_ref || res_ref.IsFutureValid())
				{
					return res_ref;
				}
				else
				{
					// If path exist, reload the resource
					if (const auto& filePath = it->second.first; !filePath.empty())
					{
						ENGINE_EXCEPT_IF(!m_loadFromFileFunc, L"Resource type : " + wStr(typeid(T).name()) + L" does not provide a valid LoadFromFile function!");
						res_ref.SetFuture(std::async(std::launch::async, [this, filePath, Name]() {
							auto resource = m_loadFromFileFunc(filePath);
							ENGINE_EXCEPT_IF(!resource, L"Resource at " + str2wstr(filePath.string()) + L" has failed to load!");
							return resource;
						}).share());
						return res_ref;
					}
					// If path does not exist, generate a promise that can either be fulfilled by LoadFromFile or AddResource
					else
					{
						if (auto& v = m_promises[Name]; v.empty()) // Only generate a single promise and reuse its shared future
						{
							v.emplace_back(ResourcePromise());
							res_ref.SetFuture(v.back().get_future().share());
						}
						return res_ref;
					}
				}
			}
			else // If task does not exists, generate one from promise
			{
				if (auto& v = m_promises[Name]; v.empty()) // Only generate a single promise and reuse its shared future
				{
					v.emplace_back(ResourcePromise());
					m_resources[Name] = std::make_pair("", ResourceTask(v.back().get_future().share()));
				}
				return m_resources[Name].second;
			}
		}

		//! Release all resources, does not invalidate pending future
		void RemoveAll() noexcept
		{
			LOCK_GUARD_NC();
			for (auto& it : m_resources)
			{
				if (auto& res_ref = it.second.second; res_ref)
				{
					res_ref.Reset();
				}
			}
		}

		//! Remove resource by name/path, does not invalidate pending future
		void Remove(const std::string& NameOrPath) noexcept
		{
			LOCK_GUARD_NC();
			if (auto it = m_resources.find(NameOrPath); it != m_resources.end())
			{
				if (auto& res_ref = it->second.second; res_ref)
				{
					res_ref.Reset();
				}
			}
			else
			{
				// Case Name is a path
				for (auto& item : m_resources)
				{
					if (auto& path = item.second.first; path == NameOrPath)
					{
						if (auto& res_ref = item.second.second; res_ref)
						{
							res_ref.Reset();
						}
					}
				}
			}
		}

		//! Check if resouce with name/path exists
		bool Has(const std::string& NameOrPath) const noexcept
		{
			LOCK_GUARD_NC();
			if (const auto& it = m_resources.find(NameOrPath); it != m_resources.end())
			{
				if (const auto& res_ref = it->second.second; res_ref || res_ref.IsFutureValid())
				{
					return true;
				}
			}
			// Case Name is a path
			for (const auto& item : m_resources)
			{
				if (const auto& path = item.second.first; path == NameOrPath)
				{
					if (const auto& res_ref = item.second.second; res_ref || res_ref.IsFutureValid())
					{
						return true;
					}
				}
			}
			return false;
		}

		//! Get all resource names in alphabetic order
		[[nodiscard]] inline std::vector<std::string> GetAllNames() const noexcept
		{
			LOCK_GUARD_NC();
			std::vector<std::string> names;
			LongMarch_MapKeyToVec(m_resources, names);
			std::sort(names.begin(), names.end());
			return names;
		}

		//! Get all resource paths in alphabetic order
		[[nodiscard]] inline std::vector<fs::path> GetAllPaths() const noexcept
		{
			LOCK_GUARD_NC();
			std::vector<fs::path> names;
			for (const auto& item : m_resources)
			{
				names.emplace_back(item.second.first);
			}
			std::sort(names.begin(), names.end());
			return names;
		}

		//! Get all resource name-paths pair
		[[nodiscard]] inline std::map<std::string, fs::path> GetAllNamePaths() const noexcept
		{
			LOCK_GUARD_NC();
			std::map<std::string, fs::path> names;
			for (const auto& item : m_resources)
			{
				names.emplace(item.first, item.second.first);
			}
			return names;
		}

		[[nodiscard]] inline const std::string GetName(const ResourceTask& resource) const noexcept
		{
			LOCK_GUARD_NC();
			for (const auto& item : m_resources)
			{
				if (item.second.second == resource)
				{
					return item.first;
				}
			}
			return std::string();
		}

		[[nodiscard]] inline const fs::path GetPath(const ResourceTask& resource) const noexcept
		{
			LOCK_GUARD_NC();
			for (const auto& item : m_resources)
			{
				if (item.second.second == resource)
				{
					return item.second.first;
				}
			}
			return fs::path();
		}

		[[nodiscard]] inline const std::pair<std::string, fs::path> GetNamePathPair(const ResourceTask& resource) const noexcept
		{
			LOCK_GUARD_NC();
			for (const auto& item : m_resources)
			{
				if (item.second.second == resource)
				{
					return std::make_pair(item.first, item.second.first);
				}
			}
			return std::pair<std::string, fs::path>();
		}

		[[nodiscard]] inline const LongMarch_Vector<ResourcePtr> GetAllResources() const noexcept
		{
			LongMarch_Vector<ResourcePtr> ret;
			for (const auto& item : m_resources)
			{
				if (const auto& res_ref = item.second.second; res_ref)
				{
					ret.emplace_back(res_ref.ptr);
				}
			}
			return ret;
		}

	private:
		ResourceLoadFromFile m_loadFromFileFunc;
		LongMarch_UnorderedMap_flat<std::string, std::pair<fs::path, ResourceTask>> m_resources;
		LongMarch_UnorderedMap_flat<std::string, std::vector<ResourcePromise>> m_promises;
	};
}
